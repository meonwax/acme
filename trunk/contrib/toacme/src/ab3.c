// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// AssBlaster 3.x stuff
//


// Includes
//
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "ab.h"
#include "acme.h"
#include "mnemo.h"
#include "io.h"
#include "scr2iso.h"


// Constants
//

// Mnemonic table in AssBlaster 3.x order
const char*	ab3_mnemonics[]	= {
	NULL,		// $80 unused
	MnemonicCPX,	// $81
	MnemonicCPY,	// $82
	MnemonicLDX,	// $83
	MnemonicLDY,	// $84
	MnemonicSTX,	// $85
	MnemonicSTY,	// $86
// start of illegals
// There are no official mnemonics for undocumented opcodes, but f8ab seems
// to confuse its ASR and ARR mnemonics. See below what they do.
	MnemonicSAX,	// $87 sta + stx	totally broken in AB3! aka AAX
// SAX is called AAX in f8ab, but it is totally broken: it generates wrong
// opcodes, one addressing mode actually generates the same opcode as LDX abs8
	"!illegal ASR",	// $88 0x6b (asr #8)			aka ARR
// opcode 0x6b does an AND, then *rotates* A to the right and sets flags.
// ARR would be a much better name. Possibly confused.
	"!illegal ARR",	// $89 0x4b (arr #8)			aka ASR, ALR
// opcode 0x4b does an AND, then *shifts* A to the right.
// ASR would be a much better name. Possibly confused.
	"!illegal AXS",	// $8a 0xcb (axs #8)			aka SBX, SAX
// opcode 0xcb does X=(A&X)-#
	MnemonicDCP,	// $8b dec + cmp
	"!illegal DOP",	// $8c 0x64 (double nop, skip byte)	aka SKB
// ...0x04,0x14,0x34,0x44,0x54,0x64,0x74,x80,0x82,0x89,0xc2,0xd4,0xe2,0xf4
	MnemonicISC,	// $8d inc + sbc			aka ISB
	"!illegal KIL",	// $8e 0x62 (kill/crash/hal)t	aka JAM, CRA, HLT
// ...0x02,0x12,0x22,0x32,0x42,0x52,0x62,0x72,0x92,0xb2,0xd2,0xf2
	"!illegal LAR",	// $8f lar	never used in AB3?	aka LAE, LAS
// in case opcode 0xbb was meant - that's a uselessly complex operation
	MnemonicLAX,	// $90 lda + ldx	partially broken in AB3!
// LAX abs16,y generates opcode 0xbb (useless) instead of 0xbf.
	MnemonicRLA,	// $91 rol + and
	MnemonicRRA,	// $92 ror + adc
	MnemonicSLO,	// $93 asl + ora
	MnemonicSRE,	// $94 lsr + eor
	"!illegal TOP",	// $95 0x5c (triple nop, skip word)	aka SKW
// ...0x0c, 0x1c, 0x3c, 0x5c, 0x7c, 0xdc, 0xfc
// end of illegals
	MnemonicADC,	// $96
	MnemonicAND,	// $97
	MnemonicASL,	// $98
	MnemonicBIT,	// $99
	MnemonicBCS, MnemonicBEQ, MnemonicBCC, MnemonicBMI,	// $9a-$9d
	MnemonicBNE, MnemonicBPL, MnemonicBVS, MnemonicBVC,	// $9e-$a1
	MnemonicBRK,						// $a2
	MnemonicCLC, MnemonicCLD, MnemonicCLI, MnemonicCLV,	// $a3-$a6
	MnemonicCMP,						// $a7
	MnemonicDEC, MnemonicDEX, MnemonicDEY,			// $a8-$aa
	MnemonicEOR,						// $ab
	MnemonicINC, MnemonicINX, MnemonicINY,			// $ac-$ae
	MnemonicJMP, MnemonicJSR,				// $af-$b0
	MnemonicLDA,						// $b1
	MnemonicLSR,						// $b2
	MnemonicNOP,// 0x1a,0x3a,0x5a,0x7a,0xda,0xfa (legal 0xea)  $b3
	MnemonicORA,						// $b4
	MnemonicPHA, MnemonicPHP, MnemonicPLA, MnemonicPLP,	// $b5-$b8
	MnemonicROL, MnemonicROR,				// $b9-$ba
	MnemonicRTI, MnemonicRTS,	// ($bf-$c0 in F8-AB)	   $bb-$bc
	MnemonicSBC,						// $bd
	MnemonicSEC, MnemonicSED, MnemonicSEI,			// $bc-$c0
	MnemonicSTA,						// $c1
	MnemonicTAX, MnemonicTAY, MnemonicTSX,			// $c2-$c4
	MnemonicTXA, MnemonicTXS, MnemonicTYA,			// $c5-$c7
};

// PseudoOpcode table in AssBlaster 3.x order
const char*	ab3_pseudo_opcodes[]	= {
	NULL,			// (la) $c8
	// NULL because ACME does not need a pseudo opcode for label defs
	PseudoOp_SetPC,		// (ba) $c9
	PseudoOp_Byte,		// (by) $ca
	PseudoOp_Fill,		// (br) $cb
	PseudoOp_PetTxt,	// (tx) $cc
	PseudoOp_MacroDef,	// (md) $cd (see AB_PSEUDOOFFSET_MACRODEF)
	PseudoOp_EndMacroDef,	// (me) $ce
	PseudoOp_MacroCall,	// (ma) $cf (see AB_PSEUDOOFFSET_MACROCALL)
	PseudoOp_EOF,		// (st) $d0
	PseudoOp_ScrTxt,	// (ts) $d1
	PseudoOp_ToFile,	// (to) $d2 (see AB_PSEUDOOFFSET_OUTFILE)
	PseudoOp_Word,		// (wo) $d3
	"; ToACME: Cannot convert \\kc.\n",
				// (kc) $d4
};

// Parse AssBlaster's packed number format. Returns error bits.
//
//#define AB_NUMVAL_FLAGBIT	0x80	// 10000000 indicates packed number
#define AB3_NUMVAL_ADD_1	0x40	// 01000000
#define AB3_NUMVAL_ADD_256	0x20	// 00100000
#define AB3_NUMVAL_FORMATMASK	0x1a	// 00011010
#define AB3_NUMVAL__FORMAT_HEX	0x10	// 00010000=16 (oh my god, the base is
#define AB3_NUMVAL__FORMAT_DEC	0x0a	// 00001010=10  given directly, without
#define AB3_NUMVAL__FORMAT_BIN	0x02	// 00000010= 2  any encoding... :))
#define AB3_NUMVAL_SIZEMASK	0x05	// 00000101
#define AB3_NUMVAL__SIZE_0	0x01	// 00000001
#define AB3_NUMVAL__SIZE_1	0x04	// 00000100
#define AB3_NUMVAL__SIZE_2	0x00	// 00000000
int ab3_parse_number(void) {	// now GotByte = first byte of packed number
	int			Flags	= GotByte,
				ErrBits	= 0;
	unsigned long int	Value	= 0,
				Add	= 0;

	// decode value
	if(Flags & AB3_NUMVAL_ADD_1)
		Add += 1;
	if(Flags & AB3_NUMVAL_ADD_256)
		Add += 256;
	switch(Flags & AB3_NUMVAL_SIZEMASK) {

		case AB3_NUMVAL__SIZE_0:// no bytes follow (0, 1, 256, 257)
		Value = Add;
		break;

		case AB3_NUMVAL__SIZE_1:// one byte follows (2 to 511)
		Value = Add + GetByte();
		break;

		case AB3_NUMVAL__SIZE_2:// two bytes follow (512 to 65535)
		Value = Add + GetLE16();
		break;

		default:	// unknown number compression
		// remember to generate error
		ErrBits |= AB_ERRBIT_UNKNOWN_NUMBER_COMPRESSION;
	}
	// continue parsing on next byte
	GetByte();

	// decode output format
	switch(Flags & AB3_NUMVAL_FORMATMASK) {

		case AB3_NUMVAL__FORMAT_BIN:
		PutByte('%');
		ab_output_binary(Value);
		break;

		case AB3_NUMVAL__FORMAT_DEC:
		fprintf(global_output_stream, "%lu", Value);
		break;

		case AB3_NUMVAL__FORMAT_HEX:
hex_fallback:	PutByte('$');
		ab_output_hexadecimal(Value);
		break;

		default:	// unknown output format
		// remember to warn
		ErrBits |= AB_ERRBIT_UNKNOWN_NUMBER_FORMAT;
		goto hex_fallback;
	}
	return(ErrBits);
}

// config struct for shared ab code
struct ab_t	ab3_conf	= {
	ab3_parse_number,
	ab3_pseudo_opcodes,
	ab3_mnemonics,
	12,	// number of addressing modes
	// meaning of input bytes (0x80-0xec differ between AB 3.x and F8-AB)
	// 0x80 unused, then 56 legals + 15 illegals = 71
	0xc8,	// first pseudo opcode
	0xd5,	// first unused byte value
	// 0xd5-0xfe are unused in AB 3.x
};

// main
//
void ab3_main(void) {
	input_set_padding(AB_ENDOFLINE);
	PutString(
"; ToACME: Adding pseudo opcode to enable undocumented (\"illegal\") opcodes:\n"
"!cpu 6510\n"
	);
	io_process_load_address();
	// first byte after load address should be AB_ENDOFLINE in AB3 sources
	if(GetByte() == AB_ENDOFLINE) {
		GetByte();	// skip it and pre-read first valid byte
		fputs("Input has AB3 header.\n", stderr);
	} else
		fputs("Input does not have any known AB3 header.\n", stderr);
	ab_main(&ab3_conf);
}
