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
	"!illegal aax",	// $87 illegal aax (sta+stx)	this is broken in AB3!
	"!illegal asr",	// $88 illegal asr (?)
	"!illegal arr",	// $89 illegal arr (?)
	"!illegal axs",	// $8a illegal axs (x=(a&x)-#)
	"!illegal dcp",	// $8b illegal dcp (dec+cmp)
	"!illegal dop",	// $8c illegal dop (double nop, skip next byte)
	"!illegal isc",	// $8d illegal isc (inc+sbc)			aka isb
	"!illegal kil",	// $8e illegal kil (kill/crash/halt)
	"!illegal lar",	// $8f illegal lar (allocated, but never used in AB3?)
	"!illegal lax",	// $90 illegal lax (lda+ldx)
	"!illegal rla",	// $91 illegal rla (rol+and)
	"!illegal rra",	// $92 illegal rra (ror+adc)
	"!illegal slo",	// $93 illegal slo (asl+ora)
	"!illegal sre",	// $94 illegal sre (lsr+eor)
	"!illegal top",	// $95 illegal top (triple nop, skip next word)
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
	MnemonicNOP,						// $b3
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
	PseudoOp_MacroDef,	// (md) $cd	index 5 in this table
	PseudoOp_EndMacroDef,	// (me) $ce
	PseudoOp_MacroCall,	// (ma) $cf	index 7 in this table
	PseudoOp_EOF,		// (st) $d0
	PseudoOp_ScrTxt,	// (ts) $d1
	PseudoOp_ToFile,	// (to) $d2
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
	io_process_load_address();
	// first byte after load address should be AB_ENDOFLINE in AB3 sources
	if(GetByte() == AB_ENDOFLINE) {
		GetByte();	// skip it and pre-read first valid byte
		fputs("Input has AB3 header.\n", stderr);
	} else
		fputs("Input does not have any known AB3 header.\n", stderr);
	ab_main(&ab3_conf);
}
