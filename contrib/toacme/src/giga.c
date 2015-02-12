// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// "GigaAss" stuff

#include <stdio.h>
#include "config.h"
#include "acme.h"
#include "gighyp.h"
#include "mnemo.h"
#include "io.h"
#include "pet2iso.h"


// constants

// token-to-(pseudo)opcode conversion table (FIXME)
const char	*giga_token[]	= {
	"FIXME-CALL",		// $a0	.CALL
	ACME_po_macro,		// $a1	.MACRO	(see MACRO_DEF_TOKEN below)
	ACME_endmacro,		// $a2	.ENDMACRO
	NULL,			// $a3	.GLOBAL	(ACME does not need a pseudo
	NULL,			// $a4	.EQUATE	 opcode for label definitions)
		// these are indented in the output file
		ACME_po_byte,	// $a5	.BYTE
		ACME_po_word,	// $a6	.WORD
		ACME_po_fill,	// $a7	.DS
		ACME_po_pet,	// $a8	.TEXT	(see MACRO_TEXT below)
	ACME_po_to,		// $a9	.OBJECT	(see MACRO_OUTFILE below)
	ACME_set_pc,		// $aa	.BASE
	"FIXME-CODE",		// $ab	.CODE
	"FIXME-ON",		// $ac	.ON
	"FIXME-GOTO",		// $ad	.GOTO
	ACME_po_if,		// $ae	.IF
	ACME_else,		// $af	.ELSE
	ACME_endif,		// $b0	.ENDIF
	ACME_po_sl,		// $b1	.SYMBOLS
	"FIXME-LISTING",	// $b2	.LISTING
	ACME_po_eof,		// $b3	.END
	"FIXME-STOP",		// $b4	.STOP
	"FIXME-PAGE",		// $b5	.PAGE
	"FIXME-NOCODE",		// $b6	.NOCODE
	"FIXME-START",		// $b7	.START
	"FIXME-NOEXP",		// $b8	.NOEXP
	"FIXME-$b9",		// $b9
	"FIXME-$ba",		// $ba
	"FIXME-$bb",		// $bb
	"FIXME-$bc",		// $bc
	"FIXME-$bd",		// $bd
	"FIXME-$be",		// $be
	"FIXME-$bf",		// $bf
		// these are indented in the output file
		MnemonicCPX,	// $c0
		MnemonicCPY,	// $c1
		MnemonicLDX,	// $c2
		MnemonicLDY,	// $c3
		MnemonicCMP,	// $c4
		MnemonicADC,	// $c5
		MnemonicAND,	// $c6
		MnemonicDEC,	// $c7
		MnemonicEOR,	// $c8
		MnemonicINC,	// $c9
		MnemonicLDA,	// $ca
		MnemonicASL,	// $cb
		MnemonicBIT,	// $cc
		MnemonicLSR,	// $cd
		MnemonicORA,	// $ce
		MnemonicROL,	// $cf
		MnemonicROR,	// $d0
		MnemonicSBC,	// $d1
		MnemonicSTA,	// $d2
		MnemonicSTX,	// $d3
		MnemonicSTY,	// $d4
		MnemonicJMP,	// $d5
		MnemonicJSR,	// $d6
		MnemonicTXA,	// $d7
		MnemonicTAX,	// $d8
		MnemonicTYA,	// $d9
		MnemonicTAY,	// $da
		MnemonicTSX,	// $db
		MnemonicTXS,	// $dc
		MnemonicPHP,	// $dd
		MnemonicPLP,	// $de
		MnemonicPHA,	// $df
		MnemonicPLA,	// $e0
		MnemonicBRK,	// $e1
		MnemonicRTI,	// $e2
		MnemonicRTS,	// $e3
		MnemonicNOP,	// $e4
		MnemonicCLC,	// $e5
		MnemonicSEC,	// $e6
		MnemonicCLI,	// $e7
		MnemonicSEI,	// $e8
		MnemonicCLV,	// $e9
		MnemonicCLD,	// $ea
		MnemonicSED,	// $eb
		MnemonicDEY,	// $ec
		MnemonicINY,	// $ed
		MnemonicDEX,	// $ee
		MnemonicINX,	// $ef
		MnemonicBPL,	// $f0
		MnemonicBMI,	// $f1
		MnemonicBVC,	// $f2
		MnemonicBVS,	// $f3
		MnemonicBCC,	// $f4
		MnemonicBCS,	// $f5
		MnemonicBNE,	// $f6
		MnemonicBEQ,	// $f7
	"FIXME-$f8",		// $f8
	"FIXME-$f9",		// $f9
	"FIXME-$fa",		// $fa
	"FIXME-$fb",		// $fb
	"FIXME-$fc",		// $fc
	"FIXME-$fd",		// $fd
	"FIXME-$fe",		// $fe
	"FIXME-$ff",		// $ff
};


// functions

// I don't know whether it's correct, but I had to start somewhere
#define FIRST_TOKEN	0xa0
#define MACRO_DEF_TOKEN	0xa1	// ugly kluge to add '{' at end of statement
#define MACRO_TEXT	0xa8	// ugly kluge for giga string specialties
#define MACRO_OUTFILE	0xa9	// ugly kluge for adding outfile format
// process opcode or pseudo opcode (tokenized)
static int process_tokenized(void)
{
	const char	*token;
	int		flags	= 0;

	if (GotByte < FIRST_TOKEN) {
		// macro call?
		IO_put_byte('+');	// add macro call character
		// fprintf(global_output_stream, "small value:$%x", GotByte);
	} else {
		switch (GotByte) {
		case MACRO_DEF_TOKEN:
			flags |= FLAG_ADD_LEFT_BRACE;
			break;
		case MACRO_TEXT:
			flags |= FLAG_ADD_ZERO | FLAG_CHANGE_LEFTARROW;
			break;
		case MACRO_OUTFILE:
			flags |= FLAG_ADD_CBM;
		}
		flags |= FLAG_INSERT_SPACE;
		token = giga_token[GotByte - FIRST_TOKEN];
		if (token != NULL)
			IO_put_string(token);
		IO_get_byte();
	}
	return flags;
}


// When tokens are known, maybe use the PseudoOpcode function from hypra?
// ...for now deleted
// [...]

// main routine for GigaAss conversion
void giga_main(void)
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
		if ((GotByte == SPACE) || (GotByte == ';')
		|| (GotByte == '\0') || (GotByte > 0x7f))
			indent = 0;
		else
			indent = GigaHypra_label_definition();
		// skip spaces
		while (GotByte == SPACE)
			IO_get_byte();
		// if there is an opcode, process it
		if ((GotByte != ';') && (GotByte != '\0')) {
			GigaHypra_indent(indent);
			GigaHypra_argument(process_tokenized());
		}
		// skip comment, if there is one
		if (GotByte == ';')
			GigaHypra_comment();
		// end of line
		IO_put_byte('\n');
	}
}
