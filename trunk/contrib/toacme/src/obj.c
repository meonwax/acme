// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2005 Marco Baye
// Have a look at "main.c" for further info
//
// disassembly stuff
//


// Includes
//
#include <stdio.h>
#include "config.h"
#include "acme.h"
#include "mnemo.h"
#include "io.h"


// Constants
//

// 6502 code table (mnemonics only)
const char*	mnemo_of_code[]	= {
	MnemonicBRK,	MnemonicORA,	NULL,		NULL,	// $00-$03
	NULL,		MnemonicORA,	MnemonicASL,	NULL,	// $04-$07
	MnemonicPHP,	MnemonicORA,	MnemonicASL,	NULL,	// $08-$0b
	NULL,		MnemonicORA,	MnemonicASL,	NULL,	// $0c-$0f
	MnemonicBPL,	MnemonicORA,	NULL,		NULL,	// $10-$13
	NULL,		MnemonicORA,	MnemonicASL,	NULL,	// $14-$17
	MnemonicCLC,	MnemonicORA,	NULL,		NULL,	// $18-$1b
	NULL,		MnemonicORA,	MnemonicASL,	NULL,	// $1c-$1f
	MnemonicJSR,	MnemonicAND,	NULL,		NULL,	// $20-$23
	MnemonicBIT,	MnemonicAND,	MnemonicROL,	NULL,	// $24-$27
	MnemonicPLP,	MnemonicAND,	MnemonicROL,	NULL,	// $28-$2b
	MnemonicBIT,	MnemonicAND,	MnemonicROL,	NULL,	// $2c-$2f
	MnemonicBMI,	MnemonicAND,	NULL,		NULL,	// $30-$33
	NULL,		MnemonicAND,	MnemonicROL,	NULL,	// $34-$37
	MnemonicSEC,	MnemonicAND,	NULL,		NULL,	// $38-$3b
	NULL,		MnemonicAND,	MnemonicROL,	NULL,	// $3c-$3f
	MnemonicRTI,	MnemonicEOR,	NULL,		NULL,	// $40-$43
	NULL,		MnemonicEOR,	MnemonicLSR,	NULL,	// $44-$47
	MnemonicPHA,	MnemonicEOR,	MnemonicLSR,	NULL,	// $48-$4b
	MnemonicJMP,	MnemonicEOR,	MnemonicLSR,	NULL,	// $4c-$4f
	MnemonicBVC,	MnemonicEOR,	NULL,		NULL,	// $50-$53
	NULL,		MnemonicEOR,	MnemonicLSR,	NULL,	// $54-$57
	MnemonicCLI,	MnemonicEOR,	NULL,		NULL,	// $58-$5b
	NULL,		MnemonicEOR,	MnemonicLSR,	NULL,	// $5c-$5f
	MnemonicRTS,	MnemonicADC,	NULL,		NULL,	// $60-$63
	NULL,		MnemonicADC,	MnemonicROR,	NULL,	// $64-$67
	MnemonicPLA,	MnemonicADC,	MnemonicROR,	NULL,	// $68-$6b
	MnemonicJMP,	MnemonicADC,	MnemonicROR,	NULL,	// $6c-$6f
	MnemonicBVS,	MnemonicADC,	NULL,		NULL,	// $70-$73
	NULL,		MnemonicADC,	MnemonicROR,	NULL,	// $74-$77
	MnemonicSEI,	MnemonicADC,	NULL,		NULL,	// $78-$7b
	NULL,		MnemonicADC,	MnemonicROR,	NULL,	// $7c-$7f
	NULL,		MnemonicSTA,	NULL,		NULL,	// $80-$83
	MnemonicSTY,	MnemonicSTA,	MnemonicSTX,	NULL,	// $84-$87
	MnemonicDEY,	NULL,		MnemonicTXA,	NULL,	// $88-$8b
	MnemonicSTY,	MnemonicSTA,	MnemonicSTX,	NULL,	// $8c-$8f
	MnemonicBCC,	MnemonicSTA,	NULL,		NULL,	// $90-$93
	MnemonicSTY,	MnemonicSTA,	MnemonicSTX,	NULL,	// $94-$97
	MnemonicTYA,	MnemonicSTA,	MnemonicTXS,	NULL,	// $98-$9b
	NULL,		MnemonicSTA,	NULL,		NULL,	// $9c-$9f
	MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	NULL,	// $a0-$a3
	MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	NULL,	// $a4-$a7
	MnemonicTAY,	MnemonicLDA,	MnemonicTAX,	NULL,	// $a8-$ab
	MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	NULL,	// $ac-$af
	MnemonicBCS,	MnemonicLDA,	NULL,		NULL,	// $b0-$b3
	MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	NULL,	// $b4-$b7
	MnemonicCLV,	MnemonicLDA,	MnemonicTSX,	NULL,	// $b8-$bb
	MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	NULL,	// $bc-$bf
	MnemonicCPY,	MnemonicCMP,	NULL,		NULL,	// $c0-$c3
	MnemonicCPY,	MnemonicCMP,	MnemonicDEC,	NULL,	// $c4-$c7
	MnemonicINY,	MnemonicCMP,	MnemonicDEX,	NULL,	// $c8-$cb
	MnemonicCPY,	MnemonicCMP,	MnemonicDEC,	NULL,	// $cc-$cf
	MnemonicBNE,	MnemonicCMP,	NULL,		NULL,	// $d0-$d3
	NULL,		MnemonicCMP,	MnemonicDEC,	NULL,	// $d4-$d7
	MnemonicCLD,	MnemonicCMP,	NULL,		NULL,	// $d8-$db
	NULL,		MnemonicCMP,	MnemonicDEC,	NULL,	// $dc-$df
	MnemonicCPX,	MnemonicSBC,	NULL,		NULL,	// $e0-$e3
	MnemonicCPX,	MnemonicSBC,	MnemonicINC,	NULL,	// $e4-$e7
	MnemonicINX,	MnemonicSBC,	MnemonicNOP,	NULL,	// $e8-$eb
	MnemonicCPX,	MnemonicSBC,	MnemonicINC,	NULL,	// $ec-$ef
	MnemonicBEQ,	MnemonicSBC,	NULL,		NULL,	// $f0-$f3
	NULL,		MnemonicSBC,	MnemonicINC,	NULL,	// $f4-$f7
	MnemonicSED,	MnemonicSBC,	NULL,		NULL,	// $f8-$fb
	NULL,		MnemonicSBC,	MnemonicINC,	NULL,	// $fc-$ff
};

// output 2-digit hex argument with correct addressing mode
//
static void put_argument2(const char pre[], int byte, const char post[]) {
	fputs(pre, global_output_stream);
	io_put_low_byte_hex(byte);
	fputs(post, global_output_stream);
}

// output 4-digit hex argument with correct addressing mode
//
static void put_argument4(const char pre[], int word, const char post[]) {
	fputs(pre, global_output_stream);
	io_put_low_16b_hex(word);
	fputs(post, global_output_stream);
}

static int	pc;	// needed by "relative" addressing mode handler

// addressing mode handler functions
// all of these output the opcode's argument and return the number to add
// to the program counter

// addressing mode handler function for 1-byte-instructions
static int am_implicit(void) {
	return(1);
}
// addressing mode handler functions for 2-byte-instructions
static int am_immediate(void) {
	put_argument2(" #$", GetByte(), "");
	if(GotByte > 15) {
		fprintf(global_output_stream, "	; (= %d", GotByte);
		if((GotByte > 31) && (GotByte != 127))
			fprintf(global_output_stream, " = '%c'", GotByte);
		PutByte(')');
	}
	return(2);
}
static int am_absolute8(void) {
	put_argument2(" $", GetByte(), "");
	return(2);
}
static int am_abs_x8(void) {
	put_argument2(" $", GetByte(), ",x");
	return(2);
}
static int am_abs_y8(void) {
	put_argument2(" $", GetByte(), ",y");
	return(2);
}
static int am_indirect_x(void) {
	put_argument2(" ($", GetByte(), ",x)");
	return(2);
}
static int am_indirect_y(void) {
	put_argument2(" ($", GetByte(), "),y");
	return(2);
}
static int am_relative(void) {
	put_argument4(" L", pc + 2 + (signed char) GetByte(), "");
	return(2);
}
// addressing mode handler functions for 3-byte-instructions
static int am_absolute16(void) {
	put_argument4(" L", GetLE16(), "");
	return(3);
}
static int am_abs_x16(void) {
	put_argument4(" L", GetLE16(), ",x");
	return(3);
}
static int am_abs_y16(void) {
	put_argument4(" L", GetLE16(), ",y");
	return(3);
}
static int am_indirect16(void) {
	put_argument4(" (L", GetLE16(), ")");
	return(3);
}

// 6502 code table (addressing mode handler functions)
int (*addressing_mode_of_code[])(void)	= {
	am_implicit,	am_indirect_x,	am_implicit,	am_implicit, // $00-$03
	am_implicit,	am_absolute8,	am_absolute8,	am_implicit, // $04-$07
	am_implicit,	am_immediate,	am_implicit,	am_implicit, // $08-$0b
	am_implicit,	am_absolute16,	am_absolute16,	am_implicit, // $0c-$0f
	am_relative,	am_indirect_y,	am_implicit,	am_implicit, // $10-$13
	am_implicit,	am_abs_x8,	am_abs_x8,	am_implicit, // $14-$17
	am_implicit,	am_abs_y16,	am_implicit,	am_implicit, // $18-$1b
	am_implicit,	am_abs_x16,	am_abs_x16,	am_implicit, // $1c-$1f
	am_absolute16,	am_indirect_x,	am_implicit,	am_implicit, // $20-$23
	am_absolute8,	am_absolute8,	am_absolute8,	am_implicit, // $24-$27
	am_implicit,	am_immediate,	am_implicit,	am_implicit, // $28-$2b
	am_absolute16,	am_absolute16,	am_absolute16,	am_implicit, // $2c-$2f
	am_relative,	am_indirect_y,	am_implicit,	am_implicit, // $30-$33
	am_implicit,	am_abs_x8,	am_abs_x8,	am_implicit, // $34-$37
	am_implicit,	am_abs_y16,	am_implicit,	am_implicit, // $38-$3b
	am_implicit,	am_abs_x16,	am_abs_x16,	am_implicit, // $3c-$3f
	am_implicit,	am_indirect_x,	am_implicit,	am_implicit, // $40-$43
	am_implicit,	am_absolute8,	am_absolute8,	am_implicit, // $44-$47
	am_implicit,	am_immediate,	am_implicit,	am_implicit, // $48-$4b
	am_absolute16,	am_absolute16,	am_absolute16,	am_implicit, // $4c-$4f
	am_relative,	am_indirect_y,	am_implicit,	am_implicit, // $50-$53
	am_implicit,	am_abs_x8,	am_abs_x8,	am_implicit, // $54-$57
	am_implicit,	am_abs_y16,	am_implicit,	am_implicit, // $58-$5b
	am_implicit,	am_abs_x16,	am_abs_x16,	am_implicit, // $5c-$5f
	am_implicit,	am_indirect_x,	am_implicit,	am_implicit, // $60-$63
	am_implicit,	am_absolute8,	am_absolute8,	am_implicit, // $64-$67
	am_implicit,	am_immediate,	am_implicit,	am_implicit, // $68-$6b
	am_indirect16,	am_absolute16,	am_absolute16,	am_implicit, // $6c-$6f
	am_relative,	am_indirect_y,	am_implicit,	am_implicit, // $70-$73
	am_implicit,	am_abs_x8,	am_abs_x8,	am_implicit, // $74-$77
	am_implicit,	am_abs_y16,	am_implicit,	am_implicit, // $78-$7b
	am_implicit,	am_abs_x16,	am_abs_x16,	am_implicit, // $7c-$7f
	am_implicit,	am_indirect_x,	am_implicit,	am_implicit, // $80-$83
	am_absolute8,	am_absolute8,	am_absolute8,	am_implicit, // $84-$87
	am_implicit,	am_implicit,	am_implicit,	am_implicit, // $88-$8b
	am_absolute16,	am_absolute16,	am_absolute16,	am_implicit, // $8c-$8f
	am_relative,	am_indirect_y,	am_implicit,	am_implicit, // $90-$93
	am_abs_x8,	am_abs_x8,	am_abs_y8,	am_implicit, // $94-$97
	am_implicit,	am_abs_y16,	am_implicit,	am_implicit, // $98-$9b
	am_implicit,	am_abs_x16,	am_implicit,	am_implicit, // $9c-$9f
	am_immediate,	am_indirect_x,	am_immediate,	am_implicit, // $a0-$a3
	am_absolute8,	am_absolute8,	am_absolute8,	am_implicit, // $a4-$a7
	am_implicit,	am_immediate,	am_implicit,	am_implicit, // $a8-$ab
	am_absolute16,	am_absolute16,	am_absolute16,	am_implicit, // $ac-$af
	am_relative,	am_indirect_y,	am_implicit,	am_implicit, // $b0-$b3
	am_abs_x8,	am_abs_x8,	am_abs_y8,	am_implicit, // $b4-$b7
	am_implicit,	am_abs_y16,	am_implicit,	am_implicit, // $b8-$bb
	am_abs_x16,	am_abs_x16,	am_abs_y16,	am_implicit, // $bc-$bf
	am_immediate,	am_indirect_x,	am_implicit,	am_implicit, // $c0-$c3
	am_absolute8,	am_absolute8,	am_absolute8,	am_implicit, // $c4-$c7
	am_implicit,	am_immediate,	am_implicit,	am_implicit, // $c8-$cb
	am_absolute16,	am_absolute16,	am_absolute16,	am_implicit, // $cc-$cf
	am_relative,	am_indirect_y,	am_implicit,	am_implicit, // $d0-$d3
	am_implicit,	am_abs_x8,	am_abs_x8,	am_implicit, // $d4-$d7
	am_implicit,	am_abs_y16,	am_implicit,	am_implicit, // $d8-$db
	am_implicit,	am_abs_x16,	am_abs_x16,	am_implicit, // $dc-$df
	am_immediate,	am_indirect_x,	am_implicit,	am_implicit, // $e0-$e3
	am_absolute8,	am_absolute8,	am_absolute8,	am_implicit, // $e4-$e7
	am_implicit,	am_immediate,	am_implicit,	am_implicit, // $e8-$eb
	am_absolute16,	am_absolute16,	am_absolute16,	am_implicit, // $ec-$ef
	am_relative,	am_indirect_y,	am_implicit,	am_implicit, // $f0-$f3
	am_implicit,	am_abs_x8,	am_abs_x8,	am_implicit, // $f4-$f7
	am_implicit,	am_abs_y16,	am_implicit,	am_implicit, // $f8-$fb
	am_implicit,	am_abs_x16,	am_abs_x16,	am_implicit, // $fc-$ff
};

// output mnemonic of given byte
//
void output_mnemonic(int byte) {
	const char*	mnemo	= mnemo_of_code[byte];

	if(mnemo)
		fputs(mnemo, global_output_stream);
	else
		put_argument2("$", byte, "");
}

// Main routine for disassembly
//
void obj_main(void) {

	input_set_padding(0);

	// process load address
	pc = GetLE16();
	put_argument4("\t\t*=$", pc, "\n");
	GetByte();
	while(!ReachedEOF) {
		put_argument4("L", pc, "\t\t");
		output_mnemonic(GotByte);
		pc += addressing_mode_of_code[GotByte]();
		PutByte('\n');
		GetByte();
	}
	// report end-of-file
	fputs("; ToACME: Reached end-of-file.\n", global_output_stream);
}
