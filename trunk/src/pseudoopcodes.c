// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// pseudo opcode stuff
#include <stdlib.h>
#include <stdio.h>
#include "acme.h"
#include "config.h"
#include "cpu.h"
#include "alu.h"
#include "dynabuf.h"
#include "flow.h"
#include "input.h"
#include "macro.h"
#include "global.h"
#include "output.h"
#include "section.h"
#include "symbol.h"
#include "tree.h"
#include "typesystem.h"
#include "pseudoopcodes.h"


// constants
static const char	s_08[]	= "08";
#define s_8	(s_08 + 1)	// Yes, I know I'm sick
#define s_16	(s_65816 + 3)	// Yes, I know I'm sick
#define s_sl	(s_asl + 1)	// Yes, I know I'm sick


// variables
struct ronode	*pseudo_opcode_tree	= NULL;	// tree to hold pseudo opcodes


// not really a pseudo opcode, but close enough to be put here:
// called when "* = EXPRESSION" is parsed
// setting program counter via "* = VALUE"
void notreallypo_setpc(void)
{
	int		segment_flags	= 0;
	intval_t	new_addr	= ALU_defined_int();

	// check for modifiers
	while (Input_accept_comma()) {
		// parse modifier. if no keyword given, give up
		if (Input_read_and_lower_keyword() == 0)
			return;

		if (strcmp(GlobalDynaBuf->buffer, "overlay") == 0) {
			segment_flags |= SEGMENT_FLAG_OVERLAY;
		} else if (strcmp(GlobalDynaBuf->buffer, "invisible") == 0) {
			segment_flags |= SEGMENT_FLAG_INVISIBLE;
		} else {
			Throw_error("Unknown \"* =\" segment modifier.");
			return;
		}
	}
	vcpu_set_pc(new_addr, segment_flags);
}


// define default value for empty memory ("!initmem" pseudo opcode)
static enum eos po_initmem(void)
{
	intval_t	content;

	// ignore in all passes but in first
	if (pass_count)
		return SKIP_REMAINDER;

	// get value
	content = ALU_defined_int();
	if ((content > 0xff) || (content < -0x80))
		Throw_error(exception_number_out_of_range);
	if (output_initmem(content & 0xff))
		return SKIP_REMAINDER;
	return ENSURE_EOS;
}


// select output file and format ("!to" pseudo opcode)
static enum eos po_to(void)
{
	// bugfix: first read filename, *then* check for first pass.
	// if skipping right away, quoted colons might be misinterpreted as EOS
	// FIXME - fix the skipping code to handle quotes! :)
	// "!sl" has been fixed as well

	// read filename to global dynamic buffer
	// if no file name given, exit (complaining will have been done)
	if (Input_read_filename(FALSE))
		return SKIP_REMAINDER;

	// only act upon this pseudo opcode in first pass
	if (pass_count)
		return SKIP_REMAINDER;

	if (output_set_output_filename())
		return SKIP_REMAINDER;

	// select output format
	// if no comma found, use default file format
	if (Input_accept_comma() == FALSE) {
		if (output_prefer_cbm_file_format()) {
			// output deprecation warning
			Throw_warning("Used \"!to\" without file format indicator. Defaulting to \"cbm\".");
		}
		return ENSURE_EOS;
	}

	// parse output format name
	// if no keyword given, give up
	if (Input_read_and_lower_keyword() == 0)
		return SKIP_REMAINDER;

	if (output_set_output_format()) {
		// error occurred
		Throw_error("Unknown output format.");
		return SKIP_REMAINDER;
	}
	return ENSURE_EOS;	// success
}


// helper function for !8, !16, !24 and !32 pseudo opcodes
static enum eos iterate(void (*fn)(intval_t))
{
	do
		fn(ALU_any_int());
	while (Input_accept_comma());
	return ENSURE_EOS;
}


// Insert 8-bit values ("!08" / "!8" / "!by" / "!byte" pseudo opcode)
static enum eos po_8(void)
{
	return iterate(output_8);
}


// Insert 16-bit values ("!16" / "!wo" / "!word" pseudo opcode)
static enum eos po_16(void)
{
	return iterate(output_le16);
}


// Insert 24-bit values ("!24" pseudo opcode)
static enum eos po_24(void)
{
	return iterate(output_le24);
}


// Insert 32-bit values ("!32" pseudo opcode)
static enum eos po_32(void)
{
	return iterate(output_le32);
}


// Include binary file ("!binary" pseudo opcode)
// FIXME - split this into "parser" and "worker" fn and move worker fn somewhere else.
static enum eos po_binary(void)
{
	FILE		*fd;
	int		byte;
	intval_t	size	= -1,	// means "not given" => "until EOF"
			skip	= 0;

	// if file name is missing, don't bother continuing
	if (Input_read_filename(TRUE))
		return SKIP_REMAINDER;
	// try to open file
	fd = fopen(GLOBALDYNABUF_CURRENT, FILE_READBINARY);
	if (fd == NULL) {
		Throw_error(exception_cannot_open_input_file);
		return SKIP_REMAINDER;
	}
	// read optional arguments
	if (Input_accept_comma()) {
		if (ALU_optional_defined_int(&size)
		&& (size < 0))
			Throw_serious_error("Negative size argument.");
		if (Input_accept_comma())
			ALU_optional_defined_int(&skip);	// read skip
	}
	// check whether including is a waste of time
	if ((size >= 0) && (pass_undefined_count || pass_real_errors)) {
		Output_fake(size);	// really including is useless anyway
	} else {
		// really insert file
		fseek(fd, skip, SEEK_SET);	// set read pointer
		// if "size" non-negative, read "size" bytes.
		// otherwise, read until EOF.
		while (size != 0) {
			byte = getc(fd);
			if (byte == EOF)
				break;
			Output_byte(byte);
			--size;
		}
		// if more should have been read, warn and add padding
		if (size > 0) {
			Throw_warning("Padding with zeroes.");
			do
				Output_byte(0);
			while (--size);
		}
	}
	fclose(fd);
	// if verbose, produce some output
	if ((pass_count == 0) && (Process_verbosity > 1)) {
		int	amount	= vcpu_get_statement_size();

		printf("Loaded %d (0x%04x) bytes from file offset %ld (0x%04lx).\n",
			amount, amount, skip, skip);
	}
	return ENSURE_EOS;
}


// Reserve space by sending bytes of given value ("!fi" / "!fill" pseudo opcode)
static enum eos po_fill(void)
{
	intval_t	fill	= FILLVALUE_FILL,
			size	= ALU_defined_int();

	if (Input_accept_comma())
		fill = ALU_any_int();
	while (size--)
		output_8(fill);
	return ENSURE_EOS;
}


// force explicit label definitions to set "address" flag ("!addr"). Has to be re-entrant.
static enum eos po_addr(void)	// now GotByte = illegal char
{
	SKIPSPACE();
	if (GotByte == CHAR_SOB) {
		typesystem_force_address_block();
		return ENSURE_EOS;
	}
	typesystem_force_address_statement(TRUE);
	return PARSE_REMAINDER;
}


// (re)set symbol
static enum eos po_set(void)	// now GotByte = illegal char
{
	struct result	result;
	int		force_bit;
	struct symbol	*symbol;
	zone_t		zone;

	if (Input_read_zone_and_keyword(&zone) == 0)	// skips spaces before
		// now GotByte = illegal char
		return SKIP_REMAINDER;

	force_bit = Input_get_force_bit();	// skips spaces after
	symbol = symbol_find(zone, force_bit);
	if (GotByte != '=') {
		Throw_error(exception_syntax);
		return SKIP_REMAINDER;
	}

	// symbol = parsed value
	GetByte();	// proceed with next char
	ALU_any_result(&result);
	// clear symbol's force bits and set new ones
	symbol->result.flags &= ~(MVALUE_FORCEBITS | MVALUE_ISBYTE);
	if (force_bit) {
		symbol->result.flags |= force_bit;
		result.flags &= ~(MVALUE_FORCEBITS | MVALUE_ISBYTE);
	}
	symbol_set_value(symbol, &result, TRUE);
	return ENSURE_EOS;
}


// set file name for symbol list
static enum eos po_sl(void)
{
	// bugfix: first read filename, *then* check for first pass.
	// if skipping right away, quoted colons might be misinterpreted as EOS
	// FIXME - why not just fix the skipping code to handle quotes? :)
	// "!to" has been fixed as well

	// read filename to global dynamic buffer
	// if no file name given, exit (complaining will have been done)
	if (Input_read_filename(FALSE))
		return SKIP_REMAINDER;

	// only process this pseudo opcode in first pass
	if (pass_count)
		return SKIP_REMAINDER;

	// if symbol list file name already set, complain and exit
	if (symbollist_filename) {
		Throw_warning("Symbol list file name already chosen.");
		return SKIP_REMAINDER;
	}

	// get malloc'd copy of filename
	symbollist_filename = DynaBuf_get_copy(GlobalDynaBuf);
	// ensure there's no garbage at end of line
	return ENSURE_EOS;
}

/*
// TODO - add "!skip AMOUNT" pseudo opcode as alternative to "* = * + AMOUNT" (needed for assemble-to-end-address)
// the new pseudo opcode would skip the given amount of bytes without starting a new segment
static enum eos po_skip(void)	// now GotByte = illegal char
{
}
*/

// switch to new zone ("!zone" or "!zn"). has to be re-entrant.
static enum eos po_zone(void)
{
	struct section	entry_values;	// buffer for outer zone
	char		*new_title;
	int		allocated;

	// remember everything about current structure
	entry_values = *Section_now;
	// set default values in case there is no valid title
	new_title = s_untitled;
	allocated = FALSE;
	// Check whether a zone title is given. If yes and it can be read,
	// get copy, remember pointer and remember to free it later on.
	if (BYTEFLAGS(GotByte) & CONTS_KEYWORD) {
		// Because we know of one character for sure,
		// there's no need to check the return value.
		Input_read_keyword();
		new_title = DynaBuf_get_copy(GlobalDynaBuf);
		allocated = TRUE;
	}
	// setup new section
	// section type is "subzone", just in case a block follows
	Section_new_zone(Section_now, "Subzone", new_title, allocated);
	if (Parse_optional_block()) {
		// Block has been parsed, so it was a SUBzone.
		Section_finalize(Section_now);	// end inner zone
		*Section_now = entry_values;	// restore entry values
	} else {
		// no block found, so it's a normal zone change
		Section_finalize(&entry_values);	// end outer zone
		Section_now->type = s_Zone;	// change type to "Zone"
	}
	return ENSURE_EOS;
}

// "!subzone" or "!sz" pseudo opcode (now obsolete)
static enum eos po_subzone(void)
{
	Throw_error("\"!subzone {}\" is obsolete; use \"!zone {}\" instead.");
	// call "!zone" instead
	return po_zone();
}


// include source file ("!source" or "!src"). has to be re-entrant.
static enum eos po_source(void)	// now GotByte = illegal char
{
	FILE		*fd;
	char		local_gotbyte;
	struct input	new_input,
			*outer_input;

	// enter new nesting level
	// quit program if recursion too deep
	if (--source_recursions_left < 0)
		Throw_serious_error("Too deeply nested. Recursive \"!source\"?");
	// read file name. quit function on error
	if (Input_read_filename(TRUE))
		return SKIP_REMAINDER;

	// if file could be opened, parse it. otherwise, complain
	if ((fd = fopen(GLOBALDYNABUF_CURRENT, FILE_READBINARY))) {
		char	filename[GlobalDynaBuf->size];

		strcpy(filename, GLOBALDYNABUF_CURRENT);
		outer_input = Input_now;	// remember old input
		local_gotbyte = GotByte;	// CAUTION - ugly kluge
		Input_now = &new_input;	// activate new input
		Parse_and_close_file(fd, filename);
		Input_now = outer_input;	// restore previous input
		GotByte = local_gotbyte;	// CAUTION - ugly kluge
	} else {
		Throw_error(exception_cannot_open_input_file);
	}
	// leave nesting level
	++source_recursions_left;
	return ENSURE_EOS;
}


// conditional assembly ("!if"). has to be re-entrant.
static enum eos po_if(void)	// now GotByte = illegal char
{
	intval_t	cond;

	cond = ALU_defined_int();
	if (GotByte != CHAR_SOB)
		Throw_serious_error(exception_no_left_brace);
	flow_parse_block_else_block(!!cond);
	return ENSURE_EOS;
}


// conditional assembly ("!ifdef" and "!ifndef"). has to be re-entrant.
static enum eos ifdef_ifndef(int is_ifndef)	// now GotByte = illegal char
{
	struct rwnode	*node;
	struct symbol	*symbol;
	zone_t		zone;
	int		defined	= FALSE;

	if (Input_read_zone_and_keyword(&zone) == 0)	// skips spaces before
		return SKIP_REMAINDER;

	Tree_hard_scan(&node, symbols_forest, zone, FALSE);
	if (node) {
		symbol = (struct symbol *) node->body;
		// in first pass, count usage
		if (pass_count == 0)
			symbol->usage++;
		if (symbol->result.flags & MVALUE_DEFINED)
			defined = TRUE;
	}
	SKIPSPACE();
	// if "ifndef", invert condition
	if (is_ifndef)
		defined = !defined;
	if (GotByte != CHAR_SOB)
		return defined ? PARSE_REMAINDER : SKIP_REMAINDER;

	flow_parse_block_else_block(defined);
	return ENSURE_EOS;
}


// conditional assembly ("!ifdef"). has to be re-entrant.
static enum eos po_ifdef(void)	// now GotByte = illegal char
{
	return ifdef_ifndef(FALSE);
}


// conditional assembly ("!ifndef"). has to be re-entrant.
static enum eos po_ifndef(void)	// now GotByte = illegal char
{
	return ifdef_ifndef(TRUE);
}


// macro definition ("!macro").
static enum eos po_macro(void)	// now GotByte = illegal char
{
	// in first pass, parse. In all other passes, skip.
	if (pass_count == 0) {
		Macro_parse_definition();	// now GotByte = '}'
	} else {
		// skip until CHAR_SOB ('{') is found.
		// no need to check for end-of-statement, because such an
		// error would already have been detected in first pass.
		// for the same reason, there is no need to check for quotes.
		while (GotByte != CHAR_SOB)
			GetByte();
		Input_skip_or_store_block(FALSE);	// now GotByte = '}'
	}
	GetByte();	// Proceed with next character
	return ENSURE_EOS;
}


// constants
#define USERMSG_DYNABUF_INITIALSIZE	80


// variables
static struct dynabuf	*user_message;	// dynamic buffer (!warn/error/serious)


// helper function to show user-defined messages
static enum eos throw_string(const char prefix[], void (*fn)(const char *))
{
	struct result	result;

	DYNABUF_CLEAR(user_message);
	DynaBuf_add_string(user_message, prefix);
	do {
		if (GotByte == '"') {
			// parse string
			GetQuotedByte();	// read initial character
			// send characters until closing quote is reached
			while (GotByte && (GotByte != '"')) {
				DYNABUF_APPEND(user_message, GotByte);
				GetQuotedByte();
			}
			if (GotByte == CHAR_EOS)
				return AT_EOS_ANYWAY;
			// after closing quote, proceed with next char
			GetByte();
		} else {
			// parse value
			ALU_any_result(&result);
			if (result.flags & MVALUE_IS_FP) {
				// floating point
				if (result.flags & MVALUE_DEFINED) {
					char	buffer[40];

					// write up to 30 significant characters.
					// remaining 10 should suffice for sign,
					// decimal point, exponent, terminator etc.
					sprintf(buffer, "%.30g", result.val.fpval);
					DynaBuf_add_string(user_message, buffer);
				} else {
					DynaBuf_add_string(user_message, "<UNDEFINED FLOAT>");
				}
			} else {
				// integer
				if (result.flags & MVALUE_DEFINED) {
					char	buffer[32];	// 11 for dec, 8 for hex

					sprintf(buffer, "%ld (0x%lx)", (long) result.val.intval, (long) result.val.intval);
					DynaBuf_add_string(user_message, buffer);
				} else {
					DynaBuf_add_string(user_message, "<UNDEFINED INT>");
				}
			}
		}
	} while (Input_accept_comma());
	DynaBuf_append(user_message, '\0');
	fn(user_message->buffer);
	return ENSURE_EOS;
}


////
//static enum eos po_debug(void)
//static enum eos po_info(void)
//{
//	return throw_string();
//}


// throw warning as given in source code
static enum eos po_warn(void)
{
	return throw_string("!warn: ", Throw_warning);

}


// throw error as given in source code
static enum eos po_error(void)
{
	return throw_string("!error: ", Throw_error);
}


// throw serious error as given in source code
static enum eos po_serious(void)
{
	return throw_string("!serious: ", Throw_serious_error);
}


// end of source file ("!endoffile" or "!eof")
static enum eos po_eof(void)
{
	// well, it doesn't end right here and now, but at end-of-line! :-)
	Input_ensure_EOS();
	Input_now->state = INPUTSTATE_EOF;
	return AT_EOS_ANYWAY;
}

// pseudo opcode table
static struct ronode	pseudo_opcode_list[]	= {
	PREDEFNODE("initmem",		po_initmem),
	PREDEFNODE("to",		po_to),
	PREDEFNODE(s_8,			po_8),
	PREDEFNODE(s_08,		po_8),
	PREDEFNODE("by",		po_8),
	PREDEFNODE("byte",		po_8),
	PREDEFNODE(s_16,		po_16),
	PREDEFNODE("wo",		po_16),
	PREDEFNODE("word",		po_16),
	PREDEFNODE("24",		po_24),
	PREDEFNODE("32",		po_32),
	PREDEFNODE("bin",		po_binary),
	PREDEFNODE("binary",		po_binary),
	PREDEFNODE("fi",		po_fill),
	PREDEFNODE("fill",		po_fill),
	PREDEFNODE("addr",		po_addr),
	PREDEFNODE("address",		po_addr),
	PREDEFNODE("set",		po_set),
	PREDEFNODE(s_sl,		po_sl),
	PREDEFNODE("symbollist",	po_sl),
//	PREDEFNODE("skip",		po_skip),
	PREDEFNODE("zn",		po_zone),
	PREDEFNODE(s_zone,		po_zone),
	PREDEFNODE("sz",		po_subzone),
	PREDEFNODE(s_subzone,		po_subzone),
	PREDEFNODE("src",		po_source),
	PREDEFNODE("source",		po_source),
	PREDEFNODE("if",		po_if),
	PREDEFNODE("ifdef",		po_ifdef),
	PREDEFNODE("ifndef",		po_ifndef),
	PREDEFNODE("macro",		po_macro),
//	PREDEFNODE("debug",		po_debug),
//	PREDEFNODE("info",		po_info),
	PREDEFNODE("warn",		po_warn),
	PREDEFNODE(s_error,		po_error),
	PREDEFNODE("serious",		po_serious),
	PREDEFNODE("eof",		po_eof),
	PREDEFLAST("endoffile",		po_eof),
	//    ^^^^ this marks the last element
};


// register pseudo opcodes and create dynamic buffer
void pseudoopcodes_init(void)
{
	user_message = DynaBuf_create(USERMSG_DYNABUF_INITIALSIZE);
	Tree_add_table(&pseudo_opcode_tree, pseudo_opcode_list);
}


// parse a pseudo opcode. has to be re-entrant.
void pseudoopcode_parse(void)	// now GotByte = "!"
{
	void		*node_body;
	enum eos	(*fn)(void),
			then	= SKIP_REMAINDER;	// prepare for errors

	GetByte();	// read next byte
	// on missing keyword, return (complaining will have been done)
	if (Input_read_and_lower_keyword()) {
		// search for tree item
		if ((Tree_easy_scan(pseudo_opcode_tree, &node_body, GlobalDynaBuf))
		&& node_body) {
			fn = (enum eos (*)(void)) node_body;
			SKIPSPACE();
			// call function
			then = fn();
		} else {
			Throw_error("Unknown pseudo opcode.");
		}
	}
	if (then == SKIP_REMAINDER)
		Input_skip_remainder();
	else if (then == ENSURE_EOS)
		Input_ensure_EOS();
	// the other two possibilities (PARSE_REMAINDER and AT_EOS_ANYWAY)
	// will lead to the remainder of the line being parsed by the mainloop.
}
