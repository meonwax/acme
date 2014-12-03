// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// pseudo opcode stuff
#include <stdlib.h>
#include <stdio.h>
//#include "acme.h"
#include "config.h"
#include "cpu.h"
#include "alu.h"
#include "dynabuf.h"
#include "input.h"
#include "global.h"
#include "output.h"
#include "tree.h"
#include "typesystem.h"
#include "pseudoopcodes.h"


// constants
static const char	s_08[]	= "08";
#define s_8	(s_08 + 1)	// Yes, I know I'm sick
#define s_16	(s_65816 + 3)	// Yes, I know I'm sick


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

/*
// TODO - add "!skip AMOUNT" pseudo opcode as alternative to "* = * + AMOUNT" (needed for assemble-to-end-address)
// the new pseudo opcode would skip the given amount of bytes without starting a new segment
static enum eos po_skip(void)	// now GotByte = illegal char
{
}
*/

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


// pseudo opcode table
static struct ronode	pseudo_opcodes[]	= {
	PREDEFNODE("initmem",	po_initmem),
	PREDEFNODE("to",	po_to),
	PREDEFNODE(s_08,	po_8),
	PREDEFNODE(s_8,		po_8),
	PREDEFNODE("by",	po_8),
	PREDEFNODE("byte",	po_8),
	PREDEFNODE(s_16,	po_16),
	PREDEFNODE("wo",	po_16),
	PREDEFNODE("word",	po_16),
	PREDEFNODE("24",	po_24),
	PREDEFNODE("32",	po_32),
	PREDEFNODE("bin",	po_binary),
	PREDEFNODE("binary",	po_binary),
	PREDEFNODE("fi",	po_fill),
	PREDEFNODE("fill",	po_fill),
	PREDEFNODE("addr",	po_addr),
	PREDEFNODE("address",	po_addr),
//	PREDEFNODE("skip",	po_skip),
//	PREDEFNODE("debug",	po_debug),
//	PREDEFNODE("info",	po_info),
	PREDEFNODE("warn",	po_warn),
	PREDEFNODE(s_error,	po_error),
	PREDEFLAST("serious",	po_serious),
	//    ^^^^ this marks the last element
};


// register pseudo opcodes and create dynamic buffer
void pseudoopcodes_init(void)
{
	user_message = DynaBuf_create(USERMSG_DYNABUF_INITIALSIZE);
	Tree_add_table(&pseudo_opcode_tree, pseudo_opcodes);
}
