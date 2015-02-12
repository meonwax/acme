// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// F8-AssBlaster stuff
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

// Mnemonic table in F8-AssBlaster order (without: MnemonicJML, MnemonicWDM)
const char*	f8ab_mnemonics[]	= {
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
	MnemonicDEC,	// $97	F8AB uses DEA as the 65816's "DEC-implicit"
	MnemonicDEC,	// $98 6502
	MnemonicDEX,	// $99 6502
	MnemonicDEY,	// $9a 6502
	MnemonicEOR,	// $9b 6502
	MnemonicINC,	// $9c	F8AB uses INA as the 65816's "INC-implicit"
	MnemonicINC,	// $9d 6502
	MnemonicINX,	// $9e 6502
	// seems as if F8AB does not know MnemonicJML (65816)...
	MnemonicINY,	// $9f 6502
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
	// seems as if F8AB does not know MnemonicWDM (65816)
	MnemonicXBA,	// $da 65816
	MnemonicXCE,	// $db 65816
};

// PseudoOpcode table in F8-AssBlaster order
const char*	f8ab_pseudo_opcodes[]	= {
	NULL,			// (la) $dc
	// NULL because ACME does not need a pseudo opcode for label defs
	PseudoOp_SetPC,		// (ba) $dd
	PseudoOp_Byte,		// (by) $de
	PseudoOp_Fill,		// (br) $df
	PseudoOp_PetTxt,	// (tx) $e0
	PseudoOp_MacroDef,	// (md) $e1 (see AB_PSEUDOOFFSET_MACRODEF)
	PseudoOp_EndMacroDef,	// (de) $e2
	PseudoOp_MacroCall,	// (ma) $e3 (see AB_PSEUDOOFFSET_MACROCALL)
	PseudoOp_EOF,		// (st) $e4
//	PseudoOp_ScrTxt is not available in F8AB. Huh?!
	"; ToACME: Cannot convert \\wa.\n",
				// (wa) $e5
	PseudoOp_ToFile,	// (on) $e6 (see AB_PSEUDOOFFSET_OUTFILE)
	PseudoOp_Word,		// (wo) $e7
	"; ToACME: Cannot convert \\kc.\n",
				// (kc) $e8
	PseudoOp_rl,		// (rl) $e9
	PseudoOp_rs,		// (rs) $ea
	PseudoOp_al,		// (al) $eb
	PseudoOp_as,		// (as) $ec
};

// Parse AssBlaster's packed number format. Returns error bits.
//
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
int f8ab_parse_number(void) {	// now GotByte = first byte of packed number
	int			Flags	= GotByte,
				ErrBits	= 0;
	unsigned long int	Value	= 0,
				Add	= 0;

	// decode value
	if(Flags & F8AB_NUMVAL_ADD_65536)
		Add += 65536;
	if(Flags & F8AB_NUMVAL_ADD_256)
		Add += 256;
	if(Flags & F8AB_NUMVAL_ADD_1)
		Add += 1;
	switch(Flags & F8AB_NUMVAL_SIZEMASK) {

		case F8AB_NUMVAL__SIZE_0:// no bytes follow (0, 1, 256, 257)
		Value = Add;
		break;

		case F8AB_NUMVAL__SIZE_1:// one byte follows (2 to 511)
		Value = Add + GetByte();
		break;

		case F8AB_NUMVAL__SIZE_2:// two bytes follow (512 to 65535)
		Value = Add + GetLE16();
		break;

		case F8AB_NUMVAL__SIZE_3:// three bytes follow (anything else)
		Value = Add + GetByte() + (GetLE16() << 8);

	}
	// continue parsing on next byte
	GetByte();

	// decode output format
	switch(Flags & F8AB_NUMVAL_FORMATMASK) {

		case F8AB_NUMVAL__FORMAT_BIN:
		PutByte('%');
		ab_output_binary(Value);
		break;

		case F8AB_NUMVAL__FORMAT_DEC:
		fprintf(global_output_stream, "%lu", Value);
		break;

		case F8AB_NUMVAL__FORMAT_HEX:
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
struct ab_t	f8ab_conf	= {
	f8ab_parse_number,
	f8ab_pseudo_opcodes,
	f8ab_mnemonics,
	23,	// number of addressing modes (FIXME - check back later!)
	// meaning of input bytes (0x80-0xec differ between AB 3.x and F8-AB)
	// 0x80: mnemos (56 6502 + 8 65c02 + 26 65816 + jml/jsl = 92)
	0xdc,	// first pseudo opcode
	0xed,	// first unused byte value
	// 0xed-0xfe are unused in F8-AB (FIXME - true? I only checked 0xed)
};

// main
//
void f8ab_main(void) {
	const char*	header_message;

	header_message = "Input does not have any known F8AB header.\n";
	input_set_padding(AB_ENDOFLINE);
	PutString(
"; ToACME: Adding pseudo opcode to enable 65816 opcodes:\n"
"!cpu 65816\n"
"; ToACME: Adding two macros to fix F8AB's non-standard argument order\n"
"; ToACME:   concerning MVP/MVN. While the commands are assembled with\n"
"; ToACME:   the destination bank byte first, the WDC docs say that in\n"
"; ToACME:   source codes, the source bank byte is given first.\n"
"!macro F8AB_BROKEN_MVP .dest,.source {mvp .source,.dest}\n"
"!macro F8AB_BROKEN_MVN .dest,.source {mvn .source,.dest}\n"
	);
	io_process_load_address();
	// most AB files have this format:
	// load_address_low, load_address_high, AB_ENDOFLINE, actual content
	// newer versions of F8AB seem to use this:
	// $ff, $00, $00, $03, AB_ENDOFLINE, actual content
	if(GetByte() == AB_ENDOFLINE) {
		GetByte();	// skip it and pre-read first valid byte
		header_message = "Input has F8AB 1.0 header.\n";
	} else {
		if((GotByte == 0)
		&& (GetByte() == 3)
		&& (GetByte() == AB_ENDOFLINE)) {
			GetByte();// skip and pre-read first valid byte
			header_message = "Input has F8AB 1.2 header.\n";
		}
	}
	fputs(header_message, stderr);
	ab_main(&f8ab_conf);
}
