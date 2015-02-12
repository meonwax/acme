// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// "HypraAss" stuff

#include <stdio.h>
#include "config.h"
#include "acme.h"
#include "gighyp.h"
#include "io.h"
#include "pet2iso.h"


// functions

// complain about unknown pseudo opcodes
static void complain(char a, char b)
{
	IO_put_string("; ToACME: .");
	if (a)
		IO_put_byte(a);
	if (b)
		IO_put_byte(b);
	IO_put_string(" cannot be converted\n");
}


// handle ".ba" and ".by"
static int process_po_b(char second)
{
	int	flags	= 0;

	switch (second) {
	case 'a':	// ".ba" = set base address
		IO_put_string(ACME_set_pc);
		break;
	case 'y':	// ".by" = insert bytes
		IO_put_string(ACME_po_byte);
		flags |= FLAG_INSERT_SPACE;
		break;
	default:
		complain('b', second);
	}
	return flags;
}


// handle ".ei", ".el", ".en" and ".eq"
static int process_po_e(char second)
{
	int	flags	= 0;

	switch (second) {
	case 'i':	// ".ei" = endif
		IO_put_string(ACME_endif);
		break;
	case 'l':	// ".el" = else
		IO_put_string(ACME_else);
		break;
	case 'n':	// ".en" = end
		IO_put_string(ACME_po_eof);
		flags |= FLAG_INSERT_SPACE;
		break;
	case 'q':	// ".eq" = label def
		break;
	default:
		complain('e', second);
	}
	return flags;
}


// handle ".tx" and ".ts"
static int process_po_t(char second)
{
	int	flags	= 0;

	switch (second) {
	case 'x':	// ".tx" = insert string
		IO_put_string(ACME_po_pet);
		flags |= FLAG_INSERT_SPACE;
		break;
	case 's':	// ".ts" = screen code string
		IO_put_string(ACME_po_scr);
		flags |= FLAG_INSERT_SPACE;
		break;
	default:
		complain('t', second);
	}
	return flags;
}


#define ARE(a, b)	((first == a) && (second == b))
// process pseudo opcode
static int process_pseudo_opcode(void)	// '.' was last read
{
	int	first,
		second;

	// get first byte. if illegal, complain and exit immediately
	first = PET2ISO(IO_get_byte());
	if ((first == SPACE) || (first == ';') || (first == '\0')) {
		complain(first, '\0');
		return 0;
	}
	// get second byte. if illegal, complain and exit immediately
	second = PET2ISO(IO_get_byte());
	if ((second == SPACE) || (second == ';') || (second == '\0')) {
		complain(first, second);
		return 0;
	}
	IO_get_byte();// pre-read unused byte
	// check pseudo opcodes (switch/case was actually harder to read)
	if (first == 'b') {		// handle ".ba" and ".by"
		process_po_b(second);
		return FLAG_INSERT_SPACE;
	}
	if (first == 'e')	// handle ".ei", ".el", ".en" and ".eq"
		return process_po_e(second);

	if (first == 't')	// handle ".tx" and ".ts"
		return process_po_t(second);

	if (ARE('.', '.')) {	// "..." = macro call
		IO_put_string(ACME_macro_call);
		return FLAG_INSERT_SPACE | FLAG_SKIP_OPENING;
	}
	if (ARE('m', 'a')) {	// ".ma" = macro definition
		IO_put_string(ACME_po_macro);
		return FLAG_INSERT_SPACE | FLAG_SKIP_OPENING | FLAG_ADD_LEFT_BRACE;
	}
	if (ARE('o', 'b')) {	// ".ob" = output to file
		IO_put_string(ACME_po_to);
		return FLAG_INSERT_SPACE | FLAG_ADD_CBM;
	}
	if (ARE('s', 'y')) {	// ".sy" = symbol dump
		IO_put_string(ACME_po_sl);
		IO_put_string("\"symboldump.txt\";");
		return 0;
	}
	if (ARE('i', 'f')) {	// ".if" = cond. assembly
		IO_put_string(ACME_po_if);
		return FLAG_INSERT_SPACE | FLAG_ADD_LEFT_BRACE;
	}
	if (ARE('g', 'l'))	// ".gl" = global label def
		return 0;

	if (ARE('a', 'p'))	// ".ap" = append source
		IO_put_string(ACME_po_source);
	else if (ARE('r', 't'))	// ".rt" = end of macro def
		IO_put_string(ACME_endmacro);
	else if (ARE('w', 'o'))	// ".wo" = insert words
		IO_put_string(ACME_po_word);
	else
		complain(first, second);
	return FLAG_INSERT_SPACE;
}


// process opcode
static void real_opcode(void)	// character was last read
{
	IO_put_byte(PET2ISO(GotByte));
	IO_get_byte();
	if ((GotByte == SPACE) || (GotByte == ';') || (GotByte == '\0'))
		return;

	IO_put_byte(PET2ISO(GotByte));
	IO_get_byte();
	if ((GotByte == SPACE) || (GotByte == ';') || (GotByte == '\0'))
		return;

	IO_put_byte(PET2ISO(GotByte));
	IO_get_byte();	// exit with unused byte pre-read
}


// main routine for HypraAss conversion
void hypra_main(void)
{
	int	indent;

	IO_set_input_padding(0);
	IO_process_load_address();
	ACME_switch_to_pet();
	// loop: once for every line in the file
	while (!IO_reached_eof) {
		// skip link pointer (if it's zero, report as end marker)
		if (IO_get_le16() == 0)
			IO_put_string("; ToACME: Found BASIC end marker.\n");
		IO_get_le16();	// skip line number
		// process line
		IO_get_byte();
		indent = 0;
		if ((GotByte != SPACE) && (GotByte != ';') && (GotByte != '\0'))
			indent = GigaHypra_label_definition();
		// skip spaces
		while (GotByte == SPACE)
			IO_get_byte();
		// if there is an opcode, process it
		if ((GotByte != ';') && (GotByte != '\0')) {
			GigaHypra_indent(indent);
			// branch to relevant routine
			if (GotByte == '.') {
				GigaHypra_argument(process_pseudo_opcode());
			} else {
				real_opcode();
				GigaHypra_argument(FLAG_INSERT_SPACE);
			}
		}
		// skip comment, if there is one
		if (GotByte == ';')
			GigaHypra_comment();
		// end of line
		IO_put_byte('\n');
	}
}
