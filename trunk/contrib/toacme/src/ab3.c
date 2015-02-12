// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// AssBlaster 3.x stuff

#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "ab.h"
#include "acme.h"
#include "mnemo.h"
#include "io.h"
#include "scr2iso.h"


// Constants

#define AB3_ADDRESSING_MODES	12

// Mnemonic table in AssBlaster 3.x order
static const char*	mnemonic_table[]	= {
	NULL,		// $80 unused
	MnemonicCPX,	// $81
	MnemonicCPY,	// $82
	MnemonicLDX,	// $83
	MnemonicLDY,	// $84
	MnemonicSTX,	// $85
	MnemonicSTY,	// $86
//============================= start of illegals =============================
	MnemonicSAX,	// $87	(AAX in AB3)		broken in AB3, see docs
	MnemonicASR,	// $88				broken in AB3, see docs
	MnemonicARR,	// $89				broken in AB3, see docs
	MnemonicSBX,	// $8a	(AXS in AB3)
	MnemonicDCP,	// $8b
	MnemonicDOP,	// $8c			different opcodes, same action
	MnemonicISC,	// $8d
	MnemonicJAM,	// $8e	(KIL in AB3)	different opcodes, same action
	"!error \"See the ToACME docs about the illegal opcode LAR.\";",
			// $8f				broken in AB3? see docs
	MnemonicLAX,	// $90				broken in AB3, see docs
	MnemonicRLA,	// $91
	MnemonicRRA,	// $92
	MnemonicSLO,	// $93
	MnemonicSRE,	// $94
	MnemonicTOP,	// $95			different opcodes, same action
//============================== end of illegals ==============================
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
	MnemonicRTI, MnemonicRTS,	// ($bf-$c0 in F8AB)	   $bb-$bc
	MnemonicSBC,						// $bd
	MnemonicSEC, MnemonicSED, MnemonicSEI,			// $bc-$c0
	MnemonicSTA,						// $c1
	MnemonicTAX, MnemonicTAY, MnemonicTSX,			// $c2-$c4
	MnemonicTXA, MnemonicTXS, MnemonicTYA,			// $c5-$c7
};

// PseudoOpcode table in AssBlaster 3.x order
static const char*	pseudo_opcode_table[]	= {
#define AB3_FIRST_PSEUDO_OPCODE		0xc8
	NULL,			// (la) $c8
	// NULL because ACME does not need a pseudo opcode for label defs
	ACME_set_pc,		// (ba) $c9
	ACME_po_byte,		// (by) $ca
	ACME_po_fill,		// (br) $cb
	ACME_po_pet,		// (tx) $cc
	ACME_po_macro,		// (md) $cd (see AB_PSEUDOOFFSET_MACRODEF)
	ACME_endmacro,		// (me) $ce
	ACME_macro_call,	// (ma) $cf (see AB_PSEUDOOFFSET_MACROCALL)
	ACME_po_eof,		// (st) $d0
	ACME_po_scr,		// (ts) $d1
	ACME_po_to,		// (to) $d2 (see AB_PSEUDOOFFSET_OUTFILE)
	ACME_po_word,		// (wo) $d3
	"; ToACME: Cannot convert \\kc.\n",
				// (kc) $d4
#define AB3_FIRST_UNUSED_CODE		0xd5
				// 0xd5-0xfe are unused in AB3
};

// Parse AssBlaster's packed number format. Returns error bits.
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
static int parse_number(void) {	// now GotByte = first byte of packed number
	int			flags		= GotByte,
				err_bits	= 0;
	unsigned long int	value		= 0,
				add		= 0;

	// decode value
	if(flags & AB3_NUMVAL_ADD_1)
		add += 1;
	if(flags & AB3_NUMVAL_ADD_256)
		add += 256;
	switch(flags & AB3_NUMVAL_SIZEMASK) {

		case AB3_NUMVAL__SIZE_0:// no bytes follow (0, 1, 256, 257)
		value = add;
		break;

		case AB3_NUMVAL__SIZE_1:// one byte follows (2 to 511)
		value = add + IO_get_byte();
		break;

		case AB3_NUMVAL__SIZE_2:// two bytes follow (512 to 65535)
		value = add + IO_get_le16();
		break;

		default:	// unknown number compression
		// remember to generate error
		err_bits |= AB_ERRBIT_UNKNOWN_NUMBER_COMPRESSION;
	}
	// continue parsing on next byte
	IO_get_byte();

	// decode output format
	switch(flags & AB3_NUMVAL_FORMATMASK) {

		case AB3_NUMVAL__FORMAT_BIN:
		IO_put_byte('%');
		AB_output_binary(value);
		break;

		case AB3_NUMVAL__FORMAT_DEC:
		fprintf(global_output_stream, "%lu", value);
		break;

		case AB3_NUMVAL__FORMAT_HEX:
hex_fallback:	IO_put_byte('$');
		AB_output_hexadecimal(value);
		break;

		default:	// unknown output format
		// remember to warn
		err_bits |= AB_ERRBIT_UNKNOWN_NUMBER_FORMAT;
		goto hex_fallback;
	}
	return(err_bits);
}

// config struct for shared ab code
struct ab_t	ab3_conf	= {
	parse_number,
	pseudo_opcode_table,
	mnemonic_table,
	AB3_ADDRESSING_MODES,
	// meaning of input bytes (0x80-0xec differ between AB3 and F8AB)
	AB3_FIRST_PSEUDO_OPCODE,
	AB3_FIRST_UNUSED_CODE,
};

// main
void ab3_main(void) {
	IO_set_input_padding(AB_ENDOFLINE);
	IO_put_string(
"; ToACME: Adding pseudo opcode to enable undocumented (\"illegal\") opcodes:\n"
"\t!cpu 6510\n"
"; ToACME: AssBlaster's support for illegal opcodes is somewhat broken.\n"
"; ToACME:   Make sure you read the ToACME docs to know what you'll have to\n"
"; ToACME:   look out for.\n"
"; ToACME:   Should work: DCP, DOP, ISC, JAM (was called KIL in AssBlaster),\n"
"; ToACME:     RLA, RRA, SBX (was called AXS in AssBlaster), SLO, SRE, TOP.\n"
"; ToACME:   Trouble: ARR, ASR, LAX, SAX (was called AAX in AssBlaster).\n"
	);
	IO_process_load_address();
	// first byte after load address should be AB_ENDOFLINE in AB3 sources
	if(IO_get_byte() == AB_ENDOFLINE) {
		IO_get_byte();	// skip it and pre-read first valid byte
		fputs("Input has AB3 header.\n", stderr);
	} else
		fputs("Input does not have any known AB3 header.\n", stderr);
	AB_main(&ab3_conf);
}
