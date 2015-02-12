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

// 6502 code table (mnemonics only)		 *illegals*
const char*	mnemo_of_code[]	= {
MnemonicBRK,	MnemonicORA,	NULL,		MnemonicSLO,	// $00-$03
NULL,		MnemonicORA,	MnemonicASL,	MnemonicSLO,	// $04-$07
MnemonicPHP,	MnemonicORA,	MnemonicASL,	NULL,		// $08-$0b
NULL,		MnemonicORA,	MnemonicASL,	MnemonicSLO,	// $0c-$0f
MnemonicBPL,	MnemonicORA,	NULL,		MnemonicSLO,	// $10-$13
NULL,		MnemonicORA,	MnemonicASL,	MnemonicSLO,	// $14-$17
MnemonicCLC,	MnemonicORA,	NULL,		MnemonicSLO,	// $18-$1b
NULL,		MnemonicORA,	MnemonicASL,	MnemonicSLO,	// $1c-$1f
MnemonicJSR,	MnemonicAND,	NULL,		MnemonicRLA,	// $20-$23
MnemonicBIT,	MnemonicAND,	MnemonicROL,	MnemonicRLA,	// $24-$27
MnemonicPLP,	MnemonicAND,	MnemonicROL,	NULL,		// $28-$2b
MnemonicBIT,	MnemonicAND,	MnemonicROL,	MnemonicRLA,	// $2c-$2f
MnemonicBMI,	MnemonicAND,	NULL,		MnemonicRLA,	// $30-$33
NULL,		MnemonicAND,	MnemonicROL,	MnemonicRLA,	// $34-$37
MnemonicSEC,	MnemonicAND,	NULL,		MnemonicRLA,	// $38-$3b
NULL,		MnemonicAND,	MnemonicROL,	MnemonicRLA,	// $3c-$3f
MnemonicRTI,	MnemonicEOR,	NULL,		MnemonicSRE,	// $40-$43
NULL,		MnemonicEOR,	MnemonicLSR,	MnemonicSRE,	// $44-$47
MnemonicPHA,	MnemonicEOR,	MnemonicLSR,	NULL,		// $48-$4b
MnemonicJMP,	MnemonicEOR,	MnemonicLSR,	MnemonicSRE,	// $4c-$4f
MnemonicBVC,	MnemonicEOR,	NULL,		MnemonicSRE,	// $50-$53
NULL,		MnemonicEOR,	MnemonicLSR,	MnemonicSRE,	// $54-$57
MnemonicCLI,	MnemonicEOR,	NULL,		MnemonicSRE,	// $58-$5b
NULL,		MnemonicEOR,	MnemonicLSR,	MnemonicSRE,	// $5c-$5f
MnemonicRTS,	MnemonicADC,	NULL,		MnemonicRRA,	// $60-$63
NULL,		MnemonicADC,	MnemonicROR,	MnemonicRRA,	// $64-$67
MnemonicPLA,	MnemonicADC,	MnemonicROR,	NULL,		// $68-$6b
MnemonicJMP,	MnemonicADC,	MnemonicROR,	MnemonicRRA,	// $6c-$6f
MnemonicBVS,	MnemonicADC,	NULL,		MnemonicRRA,	// $70-$73
NULL,		MnemonicADC,	MnemonicROR,	MnemonicRRA,	// $74-$77
MnemonicSEI,	MnemonicADC,	NULL,		MnemonicRRA,	// $78-$7b
NULL,		MnemonicADC,	MnemonicROR,	MnemonicRRA,	// $7c-$7f
NULL,		MnemonicSTA,	NULL,		MnemonicSAX,	// $80-$83
MnemonicSTY,	MnemonicSTA,	MnemonicSTX,	MnemonicSAX,	// $84-$87
MnemonicDEY,	NULL,		MnemonicTXA,	NULL,		// $88-$8b
MnemonicSTY,	MnemonicSTA,	MnemonicSTX,	MnemonicSAX,	// $8c-$8f
MnemonicBCC,	MnemonicSTA,	NULL,		NULL,		// $90-$93
MnemonicSTY,	MnemonicSTA,	MnemonicSTX,	MnemonicSAX,	// $94-$97
MnemonicTYA,	MnemonicSTA,	MnemonicTXS,	NULL,		// $98-$9b
NULL,		MnemonicSTA,	NULL,		NULL,		// $9c-$9f
MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	MnemonicLAX,	// $a0-$a3
MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	MnemonicLAX,	// $a4-$a7
MnemonicTAY,	MnemonicLDA,	MnemonicTAX,	NULL,		// $a8-$ab
MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	MnemonicLAX,	// $ac-$af
MnemonicBCS,	MnemonicLDA,	NULL,		MnemonicLAX,	// $b0-$b3
MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	MnemonicLAX,	// $b4-$b7
MnemonicCLV,	MnemonicLDA,	MnemonicTSX,	NULL,		// $b8-$bb
MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	MnemonicLAX,	// $bc-$bf
MnemonicCPY,	MnemonicCMP,	NULL,		MnemonicDCP,	// $c0-$c3
MnemonicCPY,	MnemonicCMP,	MnemonicDEC,	MnemonicDCP,	// $c4-$c7
MnemonicINY,	MnemonicCMP,	MnemonicDEX,	NULL,		// $c8-$cb
MnemonicCPY,	MnemonicCMP,	MnemonicDEC,	MnemonicDCP,	// $cc-$cf
MnemonicBNE,	MnemonicCMP,	NULL,		MnemonicDCP,	// $d0-$d3
NULL,		MnemonicCMP,	MnemonicDEC,	MnemonicDCP,	// $d4-$d7
MnemonicCLD,	MnemonicCMP,	NULL,		MnemonicDCP,	// $d8-$db
NULL,		MnemonicCMP,	MnemonicDEC,	MnemonicDCP,	// $dc-$df
MnemonicCPX,	MnemonicSBC,	NULL,		MnemonicISC,	// $e0-$e3
MnemonicCPX,	MnemonicSBC,	MnemonicINC,	MnemonicISC,	// $e4-$e7
MnemonicINX,	MnemonicSBC,	MnemonicNOP,	NULL,		// $e8-$eb
MnemonicCPX,	MnemonicSBC,	MnemonicINC,	MnemonicISC,	// $ec-$ef
MnemonicBEQ,	MnemonicSBC,	NULL,		MnemonicISC,	// $f0-$f3
NULL,		MnemonicSBC,	MnemonicINC,	MnemonicISC,	// $f4-$f7
MnemonicSED,	MnemonicSBC,	NULL,		MnemonicISC,	// $f8-$fb
NULL,		MnemonicSBC,	MnemonicINC,	MnemonicISC,	// $fc-$ff
};

// output 2-digit hex argument with correct addressing mode
//
static void put_argument2(const char pre[], int byte, const char post[]) {
	PutString(pre);
	io_put_low_byte_hex(byte);
	PutString(post);
}

// output 4-digit hex argument with correct addressing mode
//
static void put_argument4(const char pre[], int word, const char post[]) {
	PutString(pre);
	io_put_low_16b_hex(word);
	PutString(post);
}

static int	pc;	// needed by "relative" addressing mode handler

// addressing mode handler functions
// all of these output the opcode's argument and return the number to add
// to the program counter

// addressing mode handler function for 1-byte-instructions
static int am_implied(void) {
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
am_implied,	am_indirect_x,	am_implied,	am_indirect_x,	// $00-$03
am_implied,	am_absolute8,	am_absolute8,	am_absolute8,	// $04-$07
am_implied,	am_immediate,	am_implied,	am_implied,	// $08-$0b
am_implied,	am_absolute16,	am_absolute16,	am_absolute16,	// $0c-$0f
am_relative,	am_indirect_y,	am_implied,	am_indirect_y,	// $10-$13
am_implied,	am_abs_x8,	am_abs_x8,	am_abs_x8,	// $14-$17
am_implied,	am_abs_y16,	am_implied,	am_abs_y16,	// $18-$1b
am_implied,	am_abs_x16,	am_abs_x16,	am_abs_x16,	// $1c-$1f
am_absolute16,	am_indirect_x,	am_implied,	am_indirect_x,	// $20-$23
am_absolute8,	am_absolute8,	am_absolute8,	am_absolute8,	// $24-$27
am_implied,	am_immediate,	am_implied,	am_implied,	// $28-$2b
am_absolute16,	am_absolute16,	am_absolute16,	am_absolute16,	// $2c-$2f
am_relative,	am_indirect_y,	am_implied,	am_indirect_y,	// $30-$33
am_implied,	am_abs_x8,	am_abs_x8,	am_abs_x8,	// $34-$37
am_implied,	am_abs_y16,	am_implied,	am_abs_y16,	// $38-$3b
am_implied,	am_abs_x16,	am_abs_x16,	am_abs_x16,	// $3c-$3f
am_implied,	am_indirect_x,	am_implied,	am_indirect_x,	// $40-$43
am_implied,	am_absolute8,	am_absolute8,	am_absolute8,	// $44-$47
am_implied,	am_immediate,	am_implied,	am_implied,	// $48-$4b
am_absolute16,	am_absolute16,	am_absolute16,	am_absolute16,	// $4c-$4f
am_relative,	am_indirect_y,	am_implied,	am_indirect_y,	// $50-$53
am_implied,	am_abs_x8,	am_abs_x8,	am_abs_x8,	// $54-$57
am_implied,	am_abs_y16,	am_implied,	am_abs_y16,	// $58-$5b
am_implied,	am_abs_x16,	am_abs_x16,	am_abs_x16,	// $5c-$5f
am_implied,	am_indirect_x,	am_implied,	am_indirect_x,	// $60-$63
am_implied,	am_absolute8,	am_absolute8,	am_absolute8,	// $64-$67
am_implied,	am_immediate,	am_implied,	am_implied,	// $68-$6b
am_indirect16,	am_absolute16,	am_absolute16,	am_absolute16,	// $6c-$6f
am_relative,	am_indirect_y,	am_implied,	am_indirect_y,	// $70-$73
am_implied,	am_abs_x8,	am_abs_x8,	am_abs_x8,	// $74-$77
am_implied,	am_abs_y16,	am_implied,	am_abs_y16,	// $78-$7b
am_implied,	am_abs_x16,	am_abs_x16,	am_abs_x16,	// $7c-$7f
am_implied,	am_indirect_x,	am_implied,	am_indirect_x,	// $80-$83
am_absolute8,	am_absolute8,	am_absolute8,	am_absolute8,	// $84-$87
am_implied,	am_implied,	am_implied,	am_implied,	// $88-$8b
am_absolute16,	am_absolute16,	am_absolute16,	am_absolute16,	// $8c-$8f
am_relative,	am_indirect_y,	am_implied,	am_implied,	// $90-$93
am_abs_x8,	am_abs_x8,	am_abs_y8,	am_abs_y8,	// $94-$97
am_implied,	am_abs_y16,	am_implied,	am_implied,	// $98-$9b
am_implied,	am_abs_x16,	am_implied,	am_implied,	// $9c-$9f
am_immediate,	am_indirect_x,	am_immediate,	am_indirect_x,	// $a0-$a3
am_absolute8,	am_absolute8,	am_absolute8,	am_absolute8,	// $a4-$a7
am_implied,	am_immediate,	am_implied,	am_implied,	// $a8-$ab
am_absolute16,	am_absolute16,	am_absolute16,	am_absolute16,	// $ac-$af
am_relative,	am_indirect_y,	am_implied,	am_indirect_y,	// $b0-$b3
am_abs_x8,	am_abs_x8,	am_abs_y8,	am_abs_y8,	// $b4-$b7
am_implied,	am_abs_y16,	am_implied,	am_implied,	// $b8-$bb
am_abs_x16,	am_abs_x16,	am_abs_y16,	am_abs_y16,	// $bc-$bf
am_immediate,	am_indirect_x,	am_implied,	am_indirect_x,	// $c0-$c3
am_absolute8,	am_absolute8,	am_absolute8,	am_absolute8,	// $c4-$c7
am_implied,	am_immediate,	am_implied,	am_implied,	// $c8-$cb
am_absolute16,	am_absolute16,	am_absolute16,	am_absolute16,	// $cc-$cf
am_relative,	am_indirect_y,	am_implied,	am_indirect_y,	// $d0-$d3
am_implied,	am_abs_x8,	am_abs_x8,	am_abs_x8,	// $d4-$d7
am_implied,	am_abs_y16,	am_implied,	am_abs_y16,	// $d8-$db
am_implied,	am_abs_x16,	am_abs_x16,	am_abs_x16,	// $dc-$df
am_immediate,	am_indirect_x,	am_implied,	am_indirect_x,	// $e0-$e3
am_absolute8,	am_absolute8,	am_absolute8,	am_absolute8,	// $e4-$e7
am_implied,	am_immediate,	am_implied,	am_implied,	// $e8-$eb
am_absolute16,	am_absolute16,	am_absolute16,	am_absolute16,	// $ec-$ef
am_relative,	am_indirect_y,	am_implied,	am_indirect_y,	// $f0-$f3
am_implied,	am_abs_x8,	am_abs_x8,	am_abs_x8,	// $f4-$f7
am_implied,	am_abs_y16,	am_implied,	am_abs_y16,	// $f8-$fb
am_implied,	am_abs_x16,	am_abs_x16,	am_abs_x16,	// $fc-$ff
};

// output mnemonic of given byte
//
void output_mnemonic(int byte) {
	const char*	mnemo	= mnemo_of_code[byte];

	if(mnemo)
		PutString(mnemo);
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
	PutString("; ToACME: Reached end-of-file.\n");
}
