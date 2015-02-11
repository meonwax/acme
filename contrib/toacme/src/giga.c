// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2005 Marco Baye
// Have a look at "main.c" for further info
//
// "GigaAss" stuff
//


// Includes
//
#include <stdio.h>
#include "config.h"
#include "acme.h"
#include "gighyp.h"
#include "mnemo.h"
#include "io.h"
#include "pet2iso.h"


// Constants
//

// token-to-(pseudo)opcode conversion table (FIXME)
const char*	giga_token[]	= {
	"FIXME-CALL",		// $a0	.CALL
	PseudoOp_MacroDef,	// $a1	.MACRO
	PseudoOp_EndMacroDef,	// $a2	.ENDMACRO
	NULL,			// $a3	.GLOBAL	(ACME does not need a pseudo
	NULL,			// $a4	.EQUATE	 opcode for label definitions)
	// bis hier wird nicht eingerückt
	// ab hier wird eingerückt
	PseudoOp_Byte,		// $a5	.BYTE
	PseudoOp_Word,		// $a6	.WORD
	PseudoOp_Fill,		// $a7	.DS
	PseudoOp_PetTxt,	// $a8	.TEXT
	// bis hier wird eingerückt
	// ab hier wird nicht eingerückt
	PseudoOp_ToFile,	// $a9	.OBJECT
	PseudoOp_SetPC,		// $aa	.BASE
	"FIXME-CODE",		// $ab	.CODE
	"FIXME-ON",		// $ac	.ON
	"FIXME-GOTO",		// $ad	.GOTO
	PseudoOp_If,		// $ae	.IF
	PseudoOp_Else,		// $af	.ELSE
	PseudoOp_EndIf,		// $b0	.ENDIF
	PseudoOp_LabelDump,	// $b1	.SYMBOLS
	"FIXME-LISTING",	// $b2	.LISTING
	PseudoOp_EOF,		// $b3	.END
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
	// bis hier wird nicht eingerückt
	// ab hier wird eingerückt
	MnemonicCPX,		// $c0
	MnemonicCPY,		// $c1
	MnemonicLDX,		// $c2
	MnemonicLDY,		// $c3
	MnemonicCMP,		// $c4
	MnemonicADC,		// $c5
	MnemonicAND,		// $c6
	MnemonicDEC,		// $c7
	MnemonicEOR,		// $c8
	MnemonicINC,		// $c9
	MnemonicLDA,		// $ca
	MnemonicASL,		// $cb
	MnemonicBIT,		// $cc
	MnemonicLSR,		// $cd
	MnemonicORA,		// $ce
	MnemonicROL,		// $cf
	MnemonicROR,		// $d0
	MnemonicSBC,		// $d1
	MnemonicSTA,		// $d2
	MnemonicSTX,		// $d3
	MnemonicSTY,		// $d4
	MnemonicJMP,		// $d5
	MnemonicJSR,		// $d6
	MnemonicTXA,		// $d7
	MnemonicTAX,		// $d8
	MnemonicTYA,		// $d9
	MnemonicTAY,		// $da
	MnemonicTSX,		// $db
	MnemonicTXS,		// $dc
	MnemonicPHP,		// $dd
	MnemonicPLP,		// $de
	MnemonicPHA,		// $df
	MnemonicPLA,		// $e0
	MnemonicBRK,		// $e1
	MnemonicRTI,		// $e2
	MnemonicRTS,		// $e3
	MnemonicNOP,		// $e4
	MnemonicCLC,		// $e5
	MnemonicSEC,		// $e6
	MnemonicCLI,		// $e7
	MnemonicSEI,		// $e8
	MnemonicCLV,		// $e9
	MnemonicCLD,		// $ea
	MnemonicSED,		// $eb
	MnemonicDEY,		// $ec
	MnemonicINY,		// $ed
	MnemonicDEX,		// $ee
	MnemonicINX,		// $ef
	MnemonicBPL,		// $f0
	MnemonicBMI,		// $f1
	MnemonicBVC,		// $f2
	MnemonicBVS,		// $f3
	MnemonicBCC,		// $f4
	MnemonicBCS,		// $f5
	MnemonicBNE,		// $f6
	MnemonicBEQ,		// $f7
	// bis hier wird eingerückt
	// ab hier wird nicht eingerückt
	"FIXME-$f8",		// $f8
	"FIXME-$f9",		// $f9
	"FIXME-$fa",		// $fa
	"FIXME-$fb",		// $fb
	"FIXME-$fc",		// $fc
	"FIXME-$fd",		// $fd
	"FIXME-$fe",		// $fe
	"FIXME-$ff",		// $ff
};


// Functions
//


// I don't know whether it's correct, but I had to start somewhere
#define FIRST_TOKEN	0xa0
// Process opcode or pseudo opcode (tokenized)
//
int giga_Tokenized(void) {
	const char*	token;

	if(GotByte < FIRST_TOKEN)
		fprintf(global_output_stream, "small value:$%x", GotByte);
	else {
		token = giga_token[GotByte-FIRST_TOKEN];
		if(token != NULL)
			fputs(token, global_output_stream);
	}
	GetByte();
	return(0);
}

// When tokens are known, maybe use the PseudoOpcode function from hypra?
// ...for now deleted
// [...]

// Main routine for GigaAss conversion
//
void giga_main(void) {
	int	indent;

	input_set_padding(0);
	io_process_load_address();
	acme_SwitchToPet();
	// loop: once for every line in the file
	while(!ReachedEOF) {
		// skip link pointer (if it's zero, report as end marker)
		if(GetLE16() == 0)
			fputs("; ToACME: Found BASIC end marker.\n", global_output_stream);

		GetLE16();	// skip line number

		// process line
		GetByte();
		if((GotByte == ' ') || (GotByte == ';') || (GotByte == '\0') ||(GotByte>0x7f))
			indent = 0;
		else
			indent = gigahypra_LabelDef();

		// skip spaces
		while(GotByte == ' ')
			GetByte();

		// if there is an opcode, process it
		if((GotByte != ';') && (GotByte != '\0')) {
			gigahypra_Indent(indent);
			gigahypra_Opcode(giga_Tokenized());
		}

		// skip comment, if there is one
		if(GotByte == ';')
			gigahypra_ConvComment();

		// end of line
		PutByte('\n');
	}
}
