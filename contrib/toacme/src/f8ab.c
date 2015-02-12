// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2015 Marco Baye
// Have a look at "main.c" for further info
//
// Flash8-AssBlaster stuff

#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "ab.h"
#include "acme.h"
#include "mnemo.h"
#include "io.h"
#include "scr2iso.h"


// constants

#define F8AB_ADDRESSING_MODES		23	// (FIXME - check back later!)

// mnemonic table in Flash8-AssBlaster order (without: JML, WDM)
static const char	*mnemonic_table[]	= {
	MnemonicADC,	// $80 6502
	MnemonicAND,	// $81 6502
	MnemonicASL,	// $82 6502
	MnemonicBCC,	// $83 6502
	MnemonicBCS,	// $84 6502
	MnemonicBEQ,	// $85 6502
	MnemonicBIT,	// $86 6502
	MnemonicBMI,	// $87 6502
	MnemonicBNE,	// $88 6502
	MnemonicBPL,	// $89 6502
	MnemonicBRA,	// $8a 65c02
	MnemonicBRK,	// $8b 6502
	MnemonicBRL,	// $8c 65816
	MnemonicBVC,	// $8d 6502
	MnemonicBVS,	// $8e 6502
	MnemonicCLC,	// $8f 6502
	MnemonicCLD,	// $90 6502
	MnemonicCLI,	// $91 6502
	MnemonicCLV,	// $92 6502
	MnemonicCMP,	// $93 6502
	MnemonicCOP,	// $94 65816
	MnemonicCPX,	// $95 6502
	MnemonicCPY,	// $96 6502
	MnemonicDEC,	// $97	F8AB uses DEA as the 65816's "DEC implied"
	MnemonicDEC,	// $98 6502
	MnemonicDEX,	// $99 6502
	MnemonicDEY,	// $9a 6502
	MnemonicEOR,	// $9b 6502
	MnemonicINC,	// $9c	F8AB uses INA as the 65816's "INC implied"
	MnemonicINC,	// $9d 6502
	MnemonicINX,	// $9e 6502
	MnemonicINY,	// $9f 6502
	// MnemonicJML (65816) seems to be unknown to F8AB ...
	MnemonicJMP,	// $a0 6502
	MnemonicJSL,	// $a1 65816	...but it *does* know JSL? Strange.
	MnemonicJSR,	// $a2 6502
	MnemonicLDA,	// $a3 6502
	MnemonicLDX,	// $a4 6502
	MnemonicLDY,	// $a5 6502
	MnemonicLSR,	// $a6 6502
	"+F8AB_BROKEN_MVN",	// $a7 65816	F8AB uses non-standard argument
	"+F8AB_BROKEN_MVP",	// $a8 65816	ordering with MVP/MVN
	MnemonicNOP,	// $a9 6502
	MnemonicORA,	// $aa 6502
	MnemonicPEA,	// $ab 65816
	MnemonicPEI,	// $ac 65816
	MnemonicPER,	// $ad 65816
	MnemonicPHA,	// $ae 6502
	MnemonicPHB,	// $af 65816
	MnemonicPHD,	// $b0 65816
	MnemonicPHK,	// $b1 65816
	MnemonicPHP,	// $b2 6502
	MnemonicPHX,	// $b3 65c02
	MnemonicPHY,	// $b4 65c02
	MnemonicPLA,	// $b5 6502
	MnemonicPLB,	// $b6 65816
	MnemonicPLD,	// $b7 65816
	MnemonicPLP,	// $b8 6502
	MnemonicPLX,	// $b9 65c02
	MnemonicPLY,	// $ba 65c02
	MnemonicREP,	// $bb 65816
	MnemonicROL,	// $bc 6502
	MnemonicROR,	// $bd 6502
	MnemonicRTI,	// $be 6502
	MnemonicRTL,	// $bf 65816
	MnemonicRTS,	// $c0 6502
	MnemonicSBC,	// $c1 6502
	MnemonicSED,	// $c2 6502	strange order - SED before SEC?
	MnemonicSEC,	// $c3 6502
	MnemonicSEI,	// $c4 6502
	MnemonicSEP,	// $c5 65816
	MnemonicSTA,	// $c6 6502
	MnemonicSTP,	// $c7 65816
	MnemonicSTX,	// $c8 6502
	MnemonicSTY,	// $c9 6502
	MnemonicSTZ,	// $ca 65c02
	MnemonicTAX,	// $cb 6502
	MnemonicTAY,	// $cc 6502
	MnemonicTCD,	// $cd 65816
	MnemonicTCS,	// $ce 65816
	MnemonicTDC,	// $cf 65816
	MnemonicTRB,	// $d0 65c02
	MnemonicTSB,	// $d1 65c02
	MnemonicTSC,	// $d2 65816
	MnemonicTSX,	// $d3 6502
	MnemonicTXA,	// $d4 6502
	MnemonicTXS,	// $d5 6502
	MnemonicTXY,	// $d6 65816
	MnemonicTYA,	// $d7 6502
	MnemonicTYX,	// $d8 65816
	MnemonicWAI,	// $d9 65816
	// MnemonicWDM (65816) seems to be unknown to F8AB.
	MnemonicXBA,	// $da 65816
	MnemonicXCE,	// $db 65816
};


// PseudoOpcode table in Flash8-AssBlaster order
static const char	*pseudo_opcode_table[]	= {
	NULL,			// (la) $dc	// NULL because ACME does not need a pseudo opcode for label defs
	ACME_set_pc,		// (ba) $dd
	ACME_po_byte,		// (by) $de
	ACME_po_fill,		// (br) $df
	ACME_po_pet,		// (tx) $e0
	ACME_po_macro,		// (md) $e1 (see AB_PSEUDOOFFSET_MACRODEF)
	ACME_endmacro,		// (de) $e2
	ACME_macro_call,	// (ma) $e3 (see AB_PSEUDOOFFSET_MACROCALL)
	ACME_po_eof,		// (st) $e4
//	ACME_po_scr is not available in F8AB. Huh?!
	"; ToACME: Cannot convert \\wa.\n",	// (wa) $e5
	ACME_po_to,	// (on) $e6 (see AB_PSEUDOOFFSET_OUTFILE)
	ACME_po_word,		// (wo) $e7	
	"; ToACME: Cannot convert \\kc.\n",	// (kc) $e8
	ACME_po_rl,		// (rl) $e9
	ACME_po_rs,		// (rs) $ea
	ACME_po_al,		// (al) $eb
	ACME_po_as,		// (as) $ec
				// 0xed-0xfe are unused in F8AB
				// (FIXME - true? I only checked 0xed)
};


// parse AssBlaster's packed number format. returns error bits.
//#define AB_NUMVAL_FLAGBIT	0x80	// 10000000 indicates packed number
#define F8AB_NUMVAL_ADD_65536	0x40	// 01000000
#define F8AB_NUMVAL_ADD_256	0x20	// 00100000
#define F8AB_NUMVAL_ADD_1	0x10	// 00010000
#define F8AB_NUMVAL_FORMATMASK	0x0c	// 00001100
#define F8AB_NUMVAL__FORMAT_BIN	0x00	// 00000000
#define F8AB_NUMVAL__FORMAT_DEC	0x04	// 00000100
#define F8AB_NUMVAL__FORMAT_HEX	0x08	// 00001000
#define F8AB_NUMVAL__FORMAT_ILL	0x0c	// 00001100 never used by F8AB
#define F8AB_NUMVAL_SIZEMASK	0x03	// 00000011
#define F8AB_NUMVAL__SIZE_0	0x00	// 00000000
#define F8AB_NUMVAL__SIZE_1	0x01	// 00000001
#define F8AB_NUMVAL__SIZE_2	0x02	// 00000010
#define F8AB_NUMVAL__SIZE_3	0x03	// 00000011
static int parse_number(void)	// now GotByte = first byte of packed number
{
	int			flags		= GotByte,
				err_bits	= 0;
	unsigned long int	value		= 0,
				add		= 0;

	// decode value
	if (flags & F8AB_NUMVAL_ADD_65536)
		add += 65536;
	if (flags & F8AB_NUMVAL_ADD_256)
		add += 256;
	if (flags & F8AB_NUMVAL_ADD_1)
		add += 1;
	switch (flags & F8AB_NUMVAL_SIZEMASK) {
	case F8AB_NUMVAL__SIZE_0:	// no bytes follow (0, 1, 256, 257)
		value = add;
		break;
	case F8AB_NUMVAL__SIZE_1:	// one byte follows (2 to 511)
		value = add + IO_get_byte();
		break;
	case F8AB_NUMVAL__SIZE_2:	// two bytes follow (512 to 65535)
		value = add + IO_get_le16();
		break;
	case F8AB_NUMVAL__SIZE_3:	// three bytes follow (anything else)
		value = add + IO_get_le24();
	}
	// continue parsing on next byte
	IO_get_byte();

	// decode output format
	switch (flags & F8AB_NUMVAL_FORMATMASK) {
	case F8AB_NUMVAL__FORMAT_BIN:
		IO_put_byte('%');
		AB_output_binary(value);
		break;
	case F8AB_NUMVAL__FORMAT_DEC:
		fprintf(global_output_stream, "%lu", value);
		break;
	case F8AB_NUMVAL__FORMAT_HEX:
hex_fallback:	IO_put_byte('$');
		AB_output_hexadecimal(value);
		break;
	default:	// unknown output format
		// remember to warn
		err_bits |= AB_ERRBIT_UNKNOWN_NUMBER_FORMAT;
		goto hex_fallback;
	}
	return err_bits;
}


// config struct for shared ab code
struct vab	f8ab_conf	= {
	VABFLAG_ADD_DOT,
	parse_number,
	pseudo_opcode_table,
	mnemonic_table,
	F8AB_ADDRESSING_MODES,
	// meaning of input bytes (0x80-0xec differ between AB3 and F8AB)
	0x80,	// first mnemonic
	92,	// count
	0xdc,	// first pseudo opcode
	17,	// count
	0xed,	// first unused value
	18	// count
};


// main
void f8ab_main(void) {
	const char	*header_message;

	header_message = "Input does not have any known F8AB header.\n";
	IO_set_input_padding(AB_ENDOFLINE);
	IO_put_string(
"; ToACME: Adding pseudo opcode to enable 65816 opcodes:\n"
"\t!cpu 65816\n"
"; ToACME: Adding two macros to fix F8AB's non-standard argument order\n"
"; ToACME:   concerning MVP/MVN. While the commands are assembled with\n"
"; ToACME:   the destination bank byte first, the WDC docs say that in\n"
"; ToACME:   source codes, the source bank byte is given first.\n"
"; ToACME:   In other words: The macros make sure that assembling this\n"
"; ToACME:   source with ACME will produce the same binary F8AB produced.\n"
"\t!macro F8AB_BROKEN_MVP .dest, .source {mvp .source, .dest}\n"
"\t!macro F8AB_BROKEN_MVN .dest, .source {mvn .source, .dest}\n"
	);
	IO_process_load_address();
	// most AB files have this format:
	// load_address_low, load_address_high, AB_ENDOFLINE, actual content
	// newer versions of F8AB seem to use this:
	// $ff, $00, $00, $03, AB_ENDOFLINE, actual content
	if (IO_get_byte() == AB_ENDOFLINE) {
		IO_get_byte();	// skip it and pre-read first valid byte
		header_message = "Input has F8AB 1.0 header.\n";
	} else {
		if ((GotByte == 0)
		&& (IO_get_byte() == 3)
		&& (IO_get_byte() == AB_ENDOFLINE)) {
			IO_get_byte();// skip and pre-read first valid byte
			header_message = "Input has F8AB 1.2 header.\n";
		}
	}
	fputs(header_message, stderr);
	AB_main(&f8ab_conf);
}
