// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2017 Marco Baye
// Have a look at "acme.c" for further info
//
// pseudo opcode stuff
#include "pseudoopcodes.h"
#include <stdlib.h>
#include <stdio.h>
#include "acme.h"
#include "config.h"
#include "cpu.h"
#include "alu.h"
#include "dynabuf.h"
#include "encoding.h"
#include "flow.h"
#include "input.h"
#include "macro.h"
#include "global.h"
#include "output.h"
#include "section.h"
#include "symbol.h"
#include "tree.h"
#include "typesystem.h"


// different ways to handle end-of-statement:
enum eos {
	SKIP_REMAINDER,		// skip remainder of line - (after errors)
	ENSURE_EOS,		// make sure there's nothing left in statement
	PARSE_REMAINDER,	// parse what's left
	AT_EOS_ANYWAY		// actually, same as PARSE_REMAINDER
};

// constants
static const char	s_08[]	= "08";
#define s_8	(s_08 + 1)	// Yes, I know I'm sick
#define s_sl	(s_asl + 1)	// Yes, I know I'm sick
#define s_rl	(s_brl + 1)	// Yes, I know I'm sick


// variables
static struct ronode	*pseudo_opcode_tree	= NULL;	// tree to hold pseudo opcodes


// not really a pseudo opcode, but close enough to be put here:
// called when "* = EXPRESSION" is parsed
// setting program counter via "* = VALUE"
void notreallypo_setpc(void)
{
	int		segment_flags	= 0;
	struct result	intresult;

	ALU_defined_int(&intresult);	// read new address
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
	vcpu_set_pc(intresult.val.intval, segment_flags);
}


// define default value for empty memory ("!initmem" pseudo opcode)
static enum eos po_initmem(void)
{
	struct result	intresult;

	// ignore in all passes but in first
	if (pass_count)
		return SKIP_REMAINDER;

	// get value
	ALU_defined_int(&intresult);
	if ((intresult.val.intval > 0xff) || (intresult.val.intval < -0x80))
		Throw_error(exception_number_out_of_range);
	if (output_initmem(intresult.val.intval & 0xff))
		return SKIP_REMAINDER;
	return ENSURE_EOS;
}


// change output "encryption" ("!xor" pseudo opcode)
static enum eos po_xor(void)
{
	char		old_value;
	intval_t	change;

	old_value = output_get_xor();
	change = ALU_any_int();
	if ((change > 0xff) || (change < -0x80)) {
		Throw_error(exception_number_out_of_range);
		change = 0;
	}
	output_set_xor(old_value ^ change);
	// if there's a block, parse that and then restore old value!
	if (Parse_optional_block())
		output_set_xor(old_value);
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
	if (Input_read_filename(FALSE, NULL))
		return SKIP_REMAINDER;

	// only act upon this pseudo opcode in first pass
	if (pass_count)
		return SKIP_REMAINDER;

	if (outputfile_set_filename())
		return SKIP_REMAINDER;

	// select output format
	// if no comma found, use default file format
	if (Input_accept_comma() == FALSE) {
		if (outputfile_prefer_cbm_format()) {
			// output deprecation warning
			Throw_warning("Used \"!to\" without file format indicator. Defaulting to \"cbm\".");
		}
		return ENSURE_EOS;
	}

	// parse output format name
	// if no keyword given, give up
	if (Input_read_and_lower_keyword() == 0)
		return SKIP_REMAINDER;

	if (outputfile_set_format()) {
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
static enum eos po_byte(void)
{
	return iterate(output_8);
}


// Insert 16-bit values ("!16" / "!wo" / "!word" pseudo opcode)
static enum eos po_16(void)
{
	return iterate((CPU_state.type->flags & CPUFLAG_ISBIGENDIAN) ? output_be16 : output_le16);
}
// Insert 16-bit values big-endian ("!be16" pseudo opcode)
static enum eos po_be16(void)
{
	return iterate(output_be16);
}
// Insert 16-bit values little-endian ("!le16" pseudo opcode)
static enum eos po_le16(void)
{
	return iterate(output_le16);
}


// Insert 24-bit values ("!24" pseudo opcode)
static enum eos po_24(void)
{
	return iterate((CPU_state.type->flags & CPUFLAG_ISBIGENDIAN) ? output_be24 : output_le24);
}
// Insert 24-bit values big-endian ("!be24" pseudo opcode)
static enum eos po_be24(void)
{
	return iterate(output_be24);
}
// Insert 24-bit values little-endian ("!le24" pseudo opcode)
static enum eos po_le24(void)
{
	return iterate(output_le24);
}


// Insert 32-bit values ("!32" pseudo opcode)
static enum eos po_32(void)
{
	return iterate((CPU_state.type->flags & CPUFLAG_ISBIGENDIAN) ? output_be32 : output_le32);
}
// Insert 32-bit values big-endian ("!be32" pseudo opcode)
static enum eos po_be32(void)
{
	return iterate(output_be32);
}
// Insert 32-bit values little-endian ("!le32" pseudo opcode)
static enum eos po_le32(void)
{
	return iterate(output_le32);
}


// Insert bytes given as pairs of hex digits (helper for source code generators)
static enum eos po_hex(void)	// now GotByte = illegal char
{
	int		digits	= 0;
	unsigned char	byte	= 0;

	for (;;) {
		if (digits == 2) {
			Output_byte(byte);
			digits = 0;
			byte = 0;
		}
		if (GotByte >= '0' && GotByte <= '9') {
			byte = (byte << 4) | (GotByte - '0');
			++digits;
			GetByte();
			continue;
		}
		if (GotByte >= 'a' && GotByte <= 'f') {
			byte = (byte << 4) | (GotByte - 'a' + 10);
			++digits;
			GetByte();
			continue;
		}
		if (GotByte >= 'A' && GotByte <= 'F') {
			byte = (byte << 4) | (GotByte - 'A' + 10);
			++digits;
			GetByte();
			continue;
		}
		// if we're here, the current character is not a hex digit,
		// which is only allowed outside of pairs:
		if (digits == 1) {
			Throw_error("Hex digits are not given in pairs.");
			return SKIP_REMAINDER;	// error exit
		}
		switch (GotByte) {
		case ' ':
		case '\t':
			GetByte();	// spaces and tabs are ignored (maybe add commas, too?)
			continue;
		case CHAR_EOS:
			return AT_EOS_ANYWAY;	// normal exit
		default:
			Throw_error(exception_syntax);	// all other characters are errors
			return SKIP_REMAINDER;	// error exit
		}
	}
}


// "!cbm" pseudo opcode (now obsolete)
static enum eos obsolete_po_cbm(void)
{
	Throw_error("\"!cbm\" is obsolete; use \"!ct pet\" instead.");
	return ENSURE_EOS;
}

// read encoding table from file
static enum eos user_defined_encoding(FILE *stream)
{
	char			local_table[256],
				*buffered_table		= encoding_loaded_table;
	const struct encoder	*buffered_encoder	= encoder_current;

	if (stream) {
		encoding_load_from_file(local_table, stream);
		fclose(stream);
	}
	encoder_current = &encoder_file;	// activate new encoding
	encoding_loaded_table = local_table;		// activate local table
	// If there's a block, parse that and then restore old values
	if (Parse_optional_block()) {
		encoder_current = buffered_encoder;
	} else {
		// if there's *no* block, the table must be used from now on.
		// copy the local table to the "outer" table
		memcpy(buffered_table, local_table, 256);
	}
	// re-activate "outer" table (it might have been changed by memcpy())
	encoding_loaded_table = buffered_table;
	return ENSURE_EOS;
}

// use one of the pre-defined encodings (raw, pet, scr)
static enum eos predefined_encoding(void)
{
	char			local_table[256],
				*buffered_table		= encoding_loaded_table;
	const struct encoder	*buffered_encoder	= encoder_current;

	if (Input_read_and_lower_keyword()) {
		const struct encoder	*new_encoder	= encoding_find();

		if (new_encoder)
			encoder_current = new_encoder;	// activate new encoder
	}
	encoding_loaded_table = local_table;	// activate local table
	// if there's a block, parse that and then restore old values
	if (Parse_optional_block())
		encoder_current = buffered_encoder;
	// re-activate "outer" table
	encoding_loaded_table = buffered_table;
	return ENSURE_EOS;
}
// set current encoding ("!convtab" pseudo opcode)
static enum eos po_convtab(void)
{
	int	uses_lib;
	FILE	*stream;

	if ((GotByte == '<') || (GotByte == '"')) {
		// if file name is missing, don't bother continuing
		if (Input_read_filename(TRUE, &uses_lib))
			return SKIP_REMAINDER;

		stream = includepaths_open_ro(uses_lib);
		return user_defined_encoding(stream);
	} else {
		return predefined_encoding();
	}
}
// insert string(s)
static enum eos encode_string(const struct encoder *inner_encoder, char xor)
{
	const struct encoder	*outer_encoder	= encoder_current;	// buffer encoder

	// make given encoder the current one (for ALU-parsed values)
	encoder_current = inner_encoder;
	do {
		if (GotByte == '"') {
			// read initial character
			GetQuotedByte();
			// send characters until closing quote is reached
			while (GotByte && (GotByte != '"')) {
				output_8(xor ^ encoding_encode_char(GotByte));
				GetQuotedByte();
			}
			if (GotByte == CHAR_EOS)
				return AT_EOS_ANYWAY;

			// after closing quote, proceed with next char
			GetByte();
		} else {
			// Parse value. No problems with single characters
			// because the current encoding is
			// temporarily set to the given one.
			output_8(ALU_any_int());
		}
	} while (Input_accept_comma());
	encoder_current = outer_encoder;	// reactivate buffered encoder
	return ENSURE_EOS;
}
// insert text string (default format)
static enum eos po_text(void)
{
	return encode_string(encoder_current, 0);
}
// insert raw string
static enum eos po_raw(void)
{
	return encode_string(&encoder_raw, 0);
}
// insert PetSCII string
static enum eos po_pet(void)
{
	return encode_string(&encoder_pet, 0);
}
// insert screencode string
static enum eos po_scr(void)
{
	return encode_string(&encoder_scr, 0);
}
// insert screencode string, XOR'd
static enum eos po_scrxor(void)
{
	intval_t	num	= ALU_any_int();

	if (Input_accept_comma() == FALSE) {
		Throw_error(exception_syntax);
		return SKIP_REMAINDER;
	}
	return encode_string(&encoder_scr, num);
}

// Include binary file ("!binary" pseudo opcode)
// FIXME - split this into "parser" and "worker" fn and move worker fn somewhere else.
static enum eos po_binary(void)
{
	int		uses_lib;
	FILE		*stream;
	int		byte;
	intval_t	size	= -1,	// means "not given" => "until EOF"
			skip	= 0;

	// if file name is missing, don't bother continuing
	if (Input_read_filename(TRUE, &uses_lib))
		return SKIP_REMAINDER;

	// try to open file
	stream = includepaths_open_ro(uses_lib);
	if (stream == NULL)
		return SKIP_REMAINDER;

	// read optional arguments
	if (Input_accept_comma()) {
		if (ALU_optional_defined_int(&size)
		&& (size < 0))
			Throw_serious_error(exception_negative_size);
		if (Input_accept_comma())
			ALU_optional_defined_int(&skip);	// read skip
	}
	// check whether including is a waste of time
	// FIXME - future changes ("several-projects-at-once")
	// may be incompatible with this!
	if ((size >= 0) && (pass_undefined_count || pass_real_errors)) {
		output_skip(size);	// really including is useless anyway
	} else {
		// really insert file
		fseek(stream, skip, SEEK_SET);	// set read pointer
		// if "size" non-negative, read "size" bytes.
		// otherwise, read until EOF.
		while (size != 0) {
			byte = getc(stream);
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
	fclose(stream);
	// if verbose, produce some output
	if ((pass_count == 0) && (config.process_verbosity > 1)) {
		int	amount	= vcpu_get_statement_size();

		printf("Loaded %d (0x%04x) bytes from file offset %ld (0x%04lx).\n",
			amount, amount, skip, skip);
	}
	return ENSURE_EOS;
}


// reserve space by sending bytes of given value ("!fi" / "!fill" pseudo opcode)
static enum eos po_fill(void)
{
	struct result	sizeresult;
	intval_t	fill	= FILLVALUE_FILL;

	ALU_defined_int(&sizeresult);	// FIXME - forbid addresses!
	if (Input_accept_comma())
		fill = ALU_any_int();	// FIXME - forbid addresses!
	while (sizeresult.val.intval--)
		output_8(fill);
	return ENSURE_EOS;
}


// skip over some bytes in output without starting a new segment.
// in contrast to "*=*+AMOUNT", "!skip AMOUNT" does not start a new segment.
// (...and it will be needed in future for assemble-to-end-address)
static enum eos po_skip(void)	// now GotByte = illegal char
{
	struct result	amount;

	ALU_defined_int(&amount);	// FIXME - forbid addresses!
	if (amount.val.intval < 0)
		Throw_serious_error(exception_negative_size);
	else
		output_skip(amount.val.intval);
	return ENSURE_EOS;
}


// insert byte until PC fits condition
static enum eos po_align(void)
{
	// FIXME - read cpu state via function call!
	struct result	andresult,
			equalresult;
	intval_t	fill,
			test	= CPU_state.pc.val.intval;

	// make sure PC is defined.
	if ((CPU_state.pc.flags & MVALUE_DEFINED) == 0) {
		Throw_error(exception_pc_undefined);
		CPU_state.pc.flags |= MVALUE_DEFINED;	// do not complain again
		return SKIP_REMAINDER;
	}

	ALU_defined_int(&andresult);	// FIXME - forbid addresses!
	if (!Input_accept_comma())
		Throw_error(exception_syntax);
	ALU_defined_int(&equalresult);	// ...allow addresses (unlikely, but possible)
	if (Input_accept_comma())
		fill = ALU_any_int();
	else
		fill = CPU_state.type->default_align_value;
	while ((test++ & andresult.val.intval) != equalresult.val.intval)
		output_8(fill);
	return ENSURE_EOS;
}


static const char	Error_old_offset_assembly[]	=
	"\"!pseudopc/!realpc\" is obsolete; use \"!pseudopc {}\" instead.";
// start offset assembly
// FIXME - split in two parts and move backend to output.c?
// TODO - maybe add a label argument to assign the block size afterwards (for assemble-to-end-address) (or add another pseudo opcode)
static enum eos po_pseudopc(void)
{
	// FIXME - read pc using a function call!
	struct result	new_pc_result;
	intval_t	new_offset;
	int		outer_flags	= CPU_state.pc.flags;

	// set new
	ALU_defined_int(&new_pc_result);	// FIXME - allow for undefined! (complaining about non-addresses would be logical, but annoying)
	new_offset = (new_pc_result.val.intval - CPU_state.pc.val.intval) & 0xffff;
	CPU_state.pc.val.intval = new_pc_result.val.intval;
	CPU_state.pc.flags |= MVALUE_DEFINED;	// FIXME - remove when allowing undefined!
	// if there's a block, parse that and then restore old value!
	if (Parse_optional_block()) {
		// restore old
		CPU_state.pc.val.intval = (CPU_state.pc.val.intval - new_offset) & 0xffff;
		CPU_state.pc.flags = outer_flags;
	} else {
		// not using a block is no longer allowed
		Throw_error(Error_old_offset_assembly);
	}
	return ENSURE_EOS;
}


// "!realpc" pseudo opcode (now obsolete)
static enum eos obsolete_po_realpc(void)
{
	Throw_error(Error_old_offset_assembly);
	return ENSURE_EOS;
}


// select CPU ("!cpu" pseudo opcode)
static enum eos po_cpu(void)
{
	const struct cpu_type	*cpu_buffer	= CPU_state.type;	// remember current cpu
	const struct cpu_type	*new_cpu_type;

	if (Input_read_and_lower_keyword()) {
		new_cpu_type = cputype_find();
		if (new_cpu_type)
			CPU_state.type = new_cpu_type;	// activate new cpu type
		else
			Throw_error("Unknown processor.");
	}
	// if there's a block, parse that and then restore old value
	if (Parse_optional_block())
		CPU_state.type = cpu_buffer;
	return ENSURE_EOS;
}


// set register length, block-wise if needed.
static enum eos set_register_length(int *var, int make_long)
{
	int	old_size	= *var;

	// set new register length (or complain - whichever is more fitting)
	vcpu_check_and_set_reg_length(var, make_long);
	// if there's a block, parse that and then restore old value!
	if (Parse_optional_block())
		vcpu_check_and_set_reg_length(var, old_size);	// restore old length
	return ENSURE_EOS;
}
// switch to long accumulator ("!al" pseudo opcode)
static enum eos po_al(void)
{
	return set_register_length(&CPU_state.a_is_long, TRUE);
}
// switch to short accumulator ("!as" pseudo opcode)
static enum eos po_as(void)
{
	return set_register_length(&CPU_state.a_is_long, FALSE);
}
// switch to long index registers ("!rl" pseudo opcode)
static enum eos po_rl(void)
{
	return set_register_length(&CPU_state.xy_are_long, TRUE);
}
// switch to short index registers ("!rs" pseudo opcode)
static enum eos po_rs(void)
{
	return set_register_length(&CPU_state.xy_are_long, FALSE);
}


// force explicit label definitions to set "address" flag ("!addr"). Has to be re-entrant.
static enum eos po_address(void)	// now GotByte = illegal char
{
	SKIPSPACE();
	if (GotByte == CHAR_SOB) {
		typesystem_force_address_block();
		return ENSURE_EOS;
	}
	typesystem_force_address_statement(TRUE);
	return PARSE_REMAINDER;
}


#if 0
// enumerate constants
static enum eos po_enum(void)	// now GotByte = illegal char
{
	intval_t	step	= 1;

	ALU_optional_defined_int(&step);
Throw_serious_error("Not yet");	// FIXME
	return ENSURE_EOS;
}
#endif


// (re)set symbol
static enum eos po_set(void)	// now GotByte = illegal char
{
	struct result	result;
	int		force_bit;
	struct symbol	*symbol;
	scope_t		scope;

	if (Input_read_scope_and_keyword(&scope) == 0)	// skips spaces before
		// now GotByte = illegal char
		return SKIP_REMAINDER;

	force_bit = Input_get_force_bit();	// skips spaces after
	symbol = symbol_find(scope, force_bit);
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
static enum eos po_symbollist(void)
{
	// bugfix: first read filename, *then* check for first pass.
	// if skipping right away, quoted colons might be misinterpreted as EOS
	// FIXME - why not just fix the skipping code to handle quotes? :)
	// "!to" has been fixed as well

	// read filename to global dynamic buffer
	// if no file name given, exit (complaining will have been done)
	if (Input_read_filename(FALSE, NULL))
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


// switch to new zone ("!zone" or "!zn"). has to be re-entrant.
static enum eos po_zone(void)
{
	struct section	entry_values;	// buffer for outer zone
	char		*new_title;
	int		allocated;

	// remember everything about current structure
	entry_values = *section_now;
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
	section_new(section_now, "Subzone", new_title, allocated);
	if (Parse_optional_block()) {
		// Block has been parsed, so it was a SUBzone.
		section_finalize(section_now);	// end inner zone
		*section_now = entry_values;	// restore entry values
	} else {
		// no block found, so it's a normal zone change
		section_finalize(&entry_values);	// end outer zone
		section_now->type = s_Zone;	// change type to "Zone"
	}
	return ENSURE_EOS;
}

// "!subzone" or "!sz" pseudo opcode (now obsolete)
static enum eos obsolete_po_subzone(void)
{
	Throw_error("\"!subzone {}\" is obsolete; use \"!zone {}\" instead.");
	// call "!zone" instead
	return po_zone();
}

// include source file ("!source" or "!src"). has to be re-entrant.
static enum eos po_source(void)	// now GotByte = illegal char
{
	int		uses_lib;
	FILE		*stream;
	char		local_gotbyte;
	struct input	new_input,
			*outer_input;

	// enter new nesting level
	// quit program if recursion too deep
	if (--source_recursions_left < 0)
		Throw_serious_error("Too deeply nested. Recursive \"!source\"?");
	// read file name. quit function on error
	if (Input_read_filename(TRUE, &uses_lib))
		return SKIP_REMAINDER;

	// if file could be opened, parse it. otherwise, complain
	stream = includepaths_open_ro(uses_lib);
	if (stream) {
#ifdef __GNUC__
		char	filename[GlobalDynaBuf->size];	// GCC can do this
#else
		char	*filename	= safe_malloc(GlobalDynaBuf->size);	// VS can not
#endif

		strcpy(filename, GLOBALDYNABUF_CURRENT);
		outer_input = Input_now;	// remember old input
		local_gotbyte = GotByte;	// CAUTION - ugly kluge
		Input_now = &new_input;	// activate new input
		flow_parse_and_close_file(stream, filename);
		Input_now = outer_input;	// restore previous input
		GotByte = local_gotbyte;	// CAUTION - ugly kluge
#ifndef __GNUC__
		free(filename);	// GCC auto-frees
#endif
	}
	// leave nesting level
	++source_recursions_left;
	return ENSURE_EOS;
}


// conditional assembly ("!if"). has to be re-entrant.
static enum eos po_if(void)	// now GotByte = illegal char
{
	struct result	cond_result;

	ALU_defined_int(&cond_result);
	if (GotByte != CHAR_SOB)
		Throw_serious_error(exception_no_left_brace);
	flow_parse_block_else_block(!!cond_result.val.intval);
	return ENSURE_EOS;
}


// conditional assembly ("!ifdef" and "!ifndef"). has to be re-entrant.
static enum eos ifdef_ifndef(int is_ifndef)	// now GotByte = illegal char
{
	struct rwnode	*node;
	struct symbol	*symbol;
	scope_t		scope;
	int		defined	= FALSE;

	if (Input_read_scope_and_keyword(&scope) == 0)	// skips spaces before
		return SKIP_REMAINDER;

	Tree_hard_scan(&node, symbols_forest, scope, FALSE);
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


// looping assembly ("!for"). has to be re-entrant.
// old syntax: !for VAR, END { BLOCK }		VAR counts from 1 to END
// new syntax: !for VAR, START, END { BLOCK }	VAR counts from START to END
static enum eos po_for(void)	// now GotByte = illegal char
{
	scope_t		scope;
	int		force_bit;
	struct result	intresult;
	struct for_loop	loop;

	if (Input_read_scope_and_keyword(&scope) == 0)	// skips spaces before
		return SKIP_REMAINDER;

	// now GotByte = illegal char
	force_bit = Input_get_force_bit();	// skips spaces after
	loop.symbol = symbol_find(scope, force_bit);
	if (!Input_accept_comma()) {
		Throw_error(exception_syntax);
		return SKIP_REMAINDER;
	}

	ALU_defined_int(&intresult);	// read first argument
	loop.counter.addr_refs = intresult.addr_refs;
	if (Input_accept_comma()) {
		loop.old_algo = FALSE;	// new format - yay!
		if (!config.warn_on_old_for)
			Throw_first_pass_warning("Found new \"!for\" syntax.");
		loop.counter.first = intresult.val.intval;	// use first argument
		ALU_defined_int(&intresult);	// read second argument
		loop.counter.last = intresult.val.intval;	// use second argument
		// compare addr_ref counts and complain if not equal!
		if (config.warn_on_type_mismatch
		&& (intresult.addr_refs != loop.counter.addr_refs)) {
			Throw_first_pass_warning("Wrong type for loop's END value - must match type of START value.");
		}
		loop.counter.increment = (loop.counter.last < loop.counter.first) ? -1 : 1;
	} else {
		loop.old_algo = TRUE;	// old format - booo!
		if (config.warn_on_old_for)
			Throw_first_pass_warning("Found old \"!for\" syntax.");
		if (intresult.val.intval < 0)
			Throw_serious_error("Loop count is negative.");
		loop.counter.first = 0;	// CAUTION - old algo pre-increments and therefore starts with 1!
		loop.counter.last = intresult.val.intval;	// use given argument
		loop.counter.increment = 1;
	}
	if (GotByte != CHAR_SOB)
		Throw_serious_error(exception_no_left_brace);

	// remember line number of loop pseudo opcode
	loop.block.start = Input_now->line_number;
	// read loop body into DynaBuf and get copy
	loop.block.body = Input_skip_or_store_block(TRUE);	// changes line number!

	flow_forloop(&loop);
	// free memory
	free(loop.block.body);

	// GotByte of OuterInput would be '}' (if it would still exist)
	GetByte();	// fetch next byte
	return ENSURE_EOS;
}


// looping assembly ("!do"). has to be re-entrant.
static enum eos po_do(void)	// now GotByte = illegal char
{
	struct do_loop	loop;

	// read head condition to buffer
	SKIPSPACE();
	flow_store_doloop_condition(&loop.head_cond, CHAR_SOB);	// must be freed!
	if (GotByte != CHAR_SOB)
		Throw_serious_error(exception_no_left_brace);
	// remember line number of loop body,
	// then read block and get copy
	loop.block.start = Input_now->line_number;
	// reading block changes line number!
	loop.block.body = Input_skip_or_store_block(TRUE);	// must be freed!
	// now GotByte = '}'
	NEXTANDSKIPSPACE();	// now GotByte = first non-blank char after block
	// read tail condition to buffer
	flow_store_doloop_condition(&loop.tail_cond, CHAR_EOS);	// must be freed!
	// now GotByte = CHAR_EOS
	flow_doloop(&loop);
	// free memory
	free(loop.head_cond.body);
	free(loop.block.body);
	free(loop.tail_cond.body);
	return AT_EOS_ANYWAY;
}


#if 0
// looping assembly (alternative for people used to c-style loops)
static enum eos po_while(void)	// now GotByte = illegal char
{
Throw_serious_error("Not yet");	// FIXME
	return ENSURE_EOS;
}
#endif


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


#if 0
// show debug data given in source code
static enum eos po_debug(void)
{
	// FIXME - make debug output depend on some cli switch
	return throw_string("!debug: ", throw_message);
}
// show info given in source code
static enum eos po_info(void)
{
	return throw_string("!info: ", throw_message);
}
#endif


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
static enum eos po_endoffile(void)
{
	// well, it doesn't end right here and now, but at end-of-line! :-)
	Input_ensure_EOS();
	Input_now->state = INPUTSTATE_EOF;
	return AT_EOS_ANYWAY;
}

// pseudo opcode table
static struct ronode	pseudo_opcode_list[]	= {
	PREDEFNODE("initmem",		po_initmem),
	PREDEFNODE("xor",		po_xor),
	PREDEFNODE("to",		po_to),
	PREDEFNODE(s_8,			po_byte),
	PREDEFNODE(s_08,		po_byte),
	PREDEFNODE("by",		po_byte),
	PREDEFNODE("byte",		po_byte),
	PREDEFNODE("wo",		po_16),
	PREDEFNODE("word",		po_16),
	PREDEFNODE("16",		po_16),
	PREDEFNODE("be16",		po_be16),
	PREDEFNODE("le16",		po_le16),
	PREDEFNODE("24",		po_24),
	PREDEFNODE("be24",		po_be24),
	PREDEFNODE("le24",		po_le24),
	PREDEFNODE("32",		po_32),
	PREDEFNODE("be32",		po_be32),
	PREDEFNODE("le32",		po_le32),
	PREDEFNODE("h",			po_hex),
	PREDEFNODE("hex",		po_hex),
	PREDEFNODE(s_cbm,		obsolete_po_cbm),
	PREDEFNODE("ct",		po_convtab),
	PREDEFNODE("convtab",		po_convtab),
	PREDEFNODE("tx",		po_text),
	PREDEFNODE("text",		po_text),
	PREDEFNODE(s_raw,		po_raw),
	PREDEFNODE(s_pet,		po_pet),
	PREDEFNODE(s_scr,		po_scr),
	PREDEFNODE(s_scrxor,		po_scrxor),
	PREDEFNODE("bin",		po_binary),
	PREDEFNODE("binary",		po_binary),
	PREDEFNODE("fi",		po_fill),
	PREDEFNODE("fill",		po_fill),
	PREDEFNODE("skip",		po_skip),
	PREDEFNODE("align",		po_align),
	PREDEFNODE("pseudopc",		po_pseudopc),
	PREDEFNODE("realpc",		obsolete_po_realpc),
	PREDEFNODE("pc",		notreallypo_setpc),
	PREDEFNODE("org",		notreallypo_setpc),
	PREDEFNODE("cpu",		po_cpu),
	PREDEFNODE("al",		po_al),
	PREDEFNODE("as",		po_as),
	PREDEFNODE(s_rl,		po_rl),
	PREDEFNODE("rs",		po_rs),
	PREDEFNODE("addr",		po_address),
	PREDEFNODE("address",		po_address),
//	PREDEFNODE("enum",		po_enum),
	PREDEFNODE("set",		po_set),
	PREDEFNODE(s_sl,		po_symbollist),
	PREDEFNODE("symbollist",	po_symbollist),
	PREDEFNODE("zn",		po_zone),
	PREDEFNODE(s_zone,		po_zone),
	PREDEFNODE("sz",		obsolete_po_subzone),
	PREDEFNODE(s_subzone,		obsolete_po_subzone),
	PREDEFNODE("src",		po_source),
	PREDEFNODE("source",		po_source),
	PREDEFNODE("if",		po_if),
	PREDEFNODE("ifdef",		po_ifdef),
	PREDEFNODE("ifndef",		po_ifndef),
	PREDEFNODE("for",		po_for),
	PREDEFNODE("do",		po_do),
//	PREDEFNODE("while",		po_while),
	PREDEFNODE("macro",		po_macro),
//	PREDEFNODE("debug",		po_debug),
//	PREDEFNODE("info",		po_info),
	PREDEFNODE("warn",		po_warn),
	PREDEFNODE(s_error,		po_error),
	PREDEFNODE("serious",		po_serious),
	PREDEFNODE("eof",		po_endoffile),
	PREDEFLAST("endoffile",		po_endoffile),
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
