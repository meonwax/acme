// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// disassembly stuff

#include <stdio.h>
#include "config.h"
#include "acme.h"
#include "mnemo.h"
#include "io.h"


// constants

// 6502 code table (mnemonics only)		 	*illegals*
const char	*mnemo_of_code[]	= {
	MnemonicBRK,	MnemonicORA,	" JAM;0x02",	MnemonicSLO,	// $00-$03
	" DOP;0x04",	MnemonicORA,	MnemonicASL,	MnemonicSLO,	// $04-$07
	MnemonicPHP,	MnemonicORA,	MnemonicASL,	"!by$0b;ANC#",	// $08-$0b
	" TOP;0x0c",	MnemonicORA,	MnemonicASL,	MnemonicSLO,	// $0c-$0f
	MnemonicBPL,	MnemonicORA,	" JAM;0x12",	MnemonicSLO,	// $10-$13
	" DOP;0x14",	MnemonicORA,	MnemonicASL,	MnemonicSLO,	// $14-$17
	MnemonicCLC,	MnemonicORA,	" NOP;0x1a",	MnemonicSLO,	// $18-$1b
	" TOP;0x1c",	MnemonicORA,	MnemonicASL,	MnemonicSLO,	// $1c-$1f
	MnemonicJSR,	MnemonicAND,	" JAM;0x22",	MnemonicRLA,	// $20-$23
	MnemonicBIT,	MnemonicAND,	MnemonicROL,	MnemonicRLA,	// $24-$27
	MnemonicPLP,	MnemonicAND,	MnemonicROL,	"!by$2b;ANC#",	// $28-$2b
	MnemonicBIT,	MnemonicAND,	MnemonicROL,	MnemonicRLA,	// $2c-$2f
	MnemonicBMI,	MnemonicAND,	" JAM;0x32",	MnemonicRLA,	// $30-$33
	" DOP;0x34",	MnemonicAND,	MnemonicROL,	MnemonicRLA,	// $34-$37
	MnemonicSEC,	MnemonicAND,	" NOP;0x3a",	MnemonicRLA,	// $38-$3b
	" TOP;0x3c",	MnemonicAND,	MnemonicROL,	MnemonicRLA,	// $3c-$3f
	MnemonicRTI,	MnemonicEOR,	" JAM;0x42",	MnemonicSRE,	// $40-$43
	" DOP;0x44",	MnemonicEOR,	MnemonicLSR,	MnemonicSRE,	// $44-$47
	MnemonicPHA,	MnemonicEOR,	MnemonicLSR,	MnemonicASR,	// $48-$4b
	MnemonicJMP,	MnemonicEOR,	MnemonicLSR,	MnemonicSRE,	// $4c-$4f
	MnemonicBVC,	MnemonicEOR,	" JAM;0x52",	MnemonicSRE,	// $50-$53
	" DOP;0x54",	MnemonicEOR,	MnemonicLSR,	MnemonicSRE,	// $54-$57
	MnemonicCLI,	MnemonicEOR,	" NOP;0x5a",	MnemonicSRE,	// $58-$5b
	" TOP;0x5c",	MnemonicEOR,	MnemonicLSR,	MnemonicSRE,	// $5c-$5f
	MnemonicRTS,	MnemonicADC,	" JAM;0x62",	MnemonicRRA,	// $60-$63
	" DOP;0x64",	MnemonicADC,	MnemonicROR,	MnemonicRRA,	// $64-$67
	MnemonicPLA,	MnemonicADC,	MnemonicROR,	MnemonicARR,	// $68-$6b
	MnemonicJMP,	MnemonicADC,	MnemonicROR,	MnemonicRRA,	// $6c-$6f
	MnemonicBVS,	MnemonicADC,	" JAM;0x72",	MnemonicRRA,	// $70-$73
	" DOP;0x74",	MnemonicADC,	MnemonicROR,	MnemonicRRA,	// $74-$77
	MnemonicSEI,	MnemonicADC,	" NOP;0x7a",	MnemonicRRA,	// $78-$7b
	" TOP;0x7c",	MnemonicADC,	MnemonicROR,	MnemonicRRA,	// $7c-$7f
	" DOP;0x80",	MnemonicSTA,	" DOP;0x82",	MnemonicSAX,	// $80-$83
	MnemonicSTY,	MnemonicSTA,	MnemonicSTX,	MnemonicSAX,	// $84-$87
	MnemonicDEY,	" DOP;0x89",	MnemonicTXA,	NULL,		// $88-$8b
	MnemonicSTY,	MnemonicSTA,	MnemonicSTX,	MnemonicSAX,	// $8c-$8f
	MnemonicBCC,	MnemonicSTA,	" JAM;0x92",	NULL,		// $90-$93
	MnemonicSTY,	MnemonicSTA,	MnemonicSTX,	MnemonicSAX,	// $94-$97
	MnemonicTYA,	MnemonicSTA,	MnemonicTXS,	NULL,		// $98-$9b
	NULL,		MnemonicSTA,	MnemonicSHX,	NULL,		// $9c-$9f
	MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	MnemonicLAX,	// $a0-$a3
	MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	MnemonicLAX,	// $a4-$a7
	MnemonicTAY,	MnemonicLDA,	MnemonicTAX,	NULL,		// $a8-$ab
	MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	MnemonicLAX,	// $ac-$af
	MnemonicBCS,	MnemonicLDA,	" JAM;0xb2",	MnemonicLAX,	// $b0-$b3
	MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	MnemonicLAX,	// $b4-$b7
	MnemonicCLV,	MnemonicLDA,	MnemonicTSX,	NULL,		// $b8-$bb
	MnemonicLDY,	MnemonicLDA,	MnemonicLDX,	MnemonicLAX,	// $bc-$bf
	MnemonicCPY,	MnemonicCMP,	" DOP;0xc2",	MnemonicDCP,	// $c0-$c3
	MnemonicCPY,	MnemonicCMP,	MnemonicDEC,	MnemonicDCP,	// $c4-$c7
	MnemonicINY,	MnemonicCMP,	MnemonicDEX,	MnemonicSBX,	// $c8-$cb
	MnemonicCPY,	MnemonicCMP,	MnemonicDEC,	MnemonicDCP,	// $cc-$cf
	MnemonicBNE,	MnemonicCMP,	" JAM;0xd2",	MnemonicDCP,	// $d0-$d3
	" DOP;0xd4",	MnemonicCMP,	MnemonicDEC,	MnemonicDCP,	// $d4-$d7
	MnemonicCLD,	MnemonicCMP,	" NOP;0xda",	MnemonicDCP,	// $d8-$db
	" TOP;0xdc",	MnemonicCMP,	MnemonicDEC,	MnemonicDCP,	// $dc-$df
	MnemonicCPX,	MnemonicSBC,	" DOP;0xe2",	MnemonicISC,	// $e0-$e3
	MnemonicCPX,	MnemonicSBC,	MnemonicINC,	MnemonicISC,	// $e4-$e7
	MnemonicINX,	MnemonicSBC,	MnemonicNOP,	"!by$eb;SBC#",	// $e8-$eb
	MnemonicCPX,	MnemonicSBC,	MnemonicINC,	MnemonicISC,	// $ec-$ef
	MnemonicBEQ,	MnemonicSBC,	" JAM;0xf2",	MnemonicISC,	// $f0-$f3
	" DOP;0xf4",	MnemonicSBC,	MnemonicINC,	MnemonicISC,	// $f4-$f7
	MnemonicSED,	MnemonicSBC,	" NOP;0xfa",	MnemonicISC,	// $f8-$fb
	" TOP;0xfc",	MnemonicSBC,	MnemonicINC,	MnemonicISC,	// $fc-$ff
};


// output 2-digit hex argument with correct addressing mode
static void put_argument2(const char pre[], int byte, const char post[])
{
	IO_put_string(pre);
	IO_put_low_byte_hex(byte);
	IO_put_string(post);
}


// output 4-digit hex argument with correct addressing mode
static void put_argument4(const char pre[], int word, const char post[])
{
	IO_put_string(pre);
	IO_put_low_16b_hex(word);
	IO_put_string(post);
}


static int	pc;	// needed by "relative" addressing mode handler

// addressing mode handler functions
// all of these output the opcode's argument and return the number to add
// to the program counter

// addressing mode handler function for 1-byte-instructions
static int am_implied(void)
{
	return 1;
}
// addressing mode handler functions for 2-byte-instructions
static int am_immediate(void)
{
	put_argument2(" #$", IO_get_byte(), "");
	if (GotByte > 15) {
		fprintf(global_output_stream, "	; (= %d", GotByte);
		if ((GotByte > 31) && (GotByte != 127))
			fprintf(global_output_stream, " = '%c'", GotByte);
		IO_put_byte(')');
	}
	return 2;
}
static int am_absolute8(void)
{
	put_argument2(" $", IO_get_byte(), "");
	return 2;
}
static int am_abs_x8(void)
{
	put_argument2(" $", IO_get_byte(), ", x");
	return 2;
}
static int am_abs_y8(void)
{
	put_argument2(" $", IO_get_byte(), ", y");
	return 2;
}
static int am_indirect_x(void)
{
	put_argument2(" ($", IO_get_byte(), ", x)");
	return 2;
}
static int am_indirect_y(void)
{
	put_argument2(" ($", IO_get_byte(), "), y");
	return 2;
}
static int am_relative(void)
{
	put_argument4(" L", pc + 2 + (signed char) IO_get_byte(), "");
	return 2;
}
// addressing mode handler functions for 3-byte-instructions
static int am_absolute16(void)
{
	put_argument4(" L", IO_get_le16(), "");
	return 3;
}
static int am_abs_x16(void)
{
	put_argument4(" L", IO_get_le16(), ", x");
	return 3;
}
static int am_abs_y16(void)
{
	put_argument4(" L", IO_get_le16(), ", y");
	return 3;
}
static int am_indirect16(void)
{
	put_argument4(" (L", IO_get_le16(), ")");
	return 3;
}

// 6502 code table (addressing mode handler functions)
// all ANC/DOP/TOP are given as "implied", so the argument is not processed
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
	am_implied,	am_immediate,	am_implied,	am_immediate,	// $48-$4b
	am_absolute16,	am_absolute16,	am_absolute16,	am_absolute16,	// $4c-$4f
	am_relative,	am_indirect_y,	am_implied,	am_indirect_y,	// $50-$53
	am_implied,	am_abs_x8,	am_abs_x8,	am_abs_x8,	// $54-$57
	am_implied,	am_abs_y16,	am_implied,	am_abs_y16,	// $58-$5b
	am_implied,	am_abs_x16,	am_abs_x16,	am_abs_x16,	// $5c-$5f
	am_implied,	am_indirect_x,	am_implied,	am_indirect_x,	// $60-$63
	am_implied,	am_absolute8,	am_absolute8,	am_absolute8,	// $64-$67
	am_implied,	am_immediate,	am_implied,	am_immediate,	// $68-$6b
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
	am_implied,	am_abs_x16,	am_abs_y16,	am_implied,	// $9c-$9f
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
	am_implied,	am_immediate,	am_implied,	am_immediate,	// $c8-$cb
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
static void output_mnemonic(int byte)
{
	const char	*mnemo	= mnemo_of_code[byte];

	if (mnemo)
		IO_put_string(mnemo);
	else
		put_argument2("$", byte, "");
}


// main routine for disassembly
void obj_main(void)
{
	IO_set_input_padding(0);
	// process load address
	pc = IO_get_le16();
	put_argument4("\t\t*=$", pc, "\n");
	IO_get_byte();
	while (!IO_reached_eof) {
		put_argument4("L", pc, "\t\t");
		output_mnemonic(GotByte);
		pc += addressing_mode_of_code[GotByte]();
		IO_put_byte('\n');
		IO_get_byte();
	}
	// report end-of-file
	IO_put_string("; ToACME: Reached end-of-file.\n");
}
