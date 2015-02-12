// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2015 Marco Baye
// Have a look at "main.c" for further info
//
// AssBlaster 3.x stuff

#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "ab.h"
#include "acme.h"
#include "io.h"
#include "scr2iso.h"


// constants

#define AB3_ADDRESSING_MODES	12

// parse AssBlaster's packed number format. returns error bits.
//#define AB_NUMVAL_FLAGBIT	0x80	// 10000000 indicates packed number
#define AB3_NUMVAL_ADD_1	0x40	// 01000000
#define AB3_NUMVAL_ADD_256	0x20	// 00100000
#define AB3_NUMVAL_FORMATMASK	0x1a	// 00011010
#define AB3_NUMVAL__FORMAT_HEX	0x10	// 00010000=16 (oh bob, the base is
#define AB3_NUMVAL__FORMAT_DEC	0x0a	// 00001010=10  given directly, without
#define AB3_NUMVAL__FORMAT_BIN	0x02	// 00000010= 2  any encoding... :))
#define AB3_NUMVAL_SIZEMASK	0x05	// 00000101
#define AB3_NUMVAL__SIZE_0	0x01	// 00000001
#define AB3_NUMVAL__SIZE_1	0x04	// 00000100
#define AB3_NUMVAL__SIZE_2	0x00	// 00000000
static int parse_number(void)	// now GotByte = first byte of packed number
{
	int			flags		= GotByte,
				err_bits	= 0;
	unsigned long int	value		= 0,
				add		= 0;

	// decode value
	if (flags & AB3_NUMVAL_ADD_1)
		add += 1;
	if (flags & AB3_NUMVAL_ADD_256)
		add += 256;
	switch (flags & AB3_NUMVAL_SIZEMASK) {
	case AB3_NUMVAL__SIZE_0:	// no bytes follow (0, 1, 256, 257)
		value = add;
		break;
	case AB3_NUMVAL__SIZE_1:	// one byte follows (2 to 511)
		value = add + IO_get_byte();
		break;
	case AB3_NUMVAL__SIZE_2:	// two bytes follow (512 to 65535)
		value = add + IO_get_le16();
		break;
	default:	// unknown number compression
		// remember to generate error
		err_bits |= AB_ERRBIT_UNKNOWN_NUMBER_COMPRESSION;
	}
	// continue parsing on next byte
	IO_get_byte();

	// decode output format
	switch (flags & AB3_NUMVAL_FORMATMASK) {
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
	return err_bits;
}


// config struct for shared VisAss/AB code
struct vab	ab3_conf	= {
	VABFLAG_ADD_DOT,
	parse_number,
	visass_ab3_pseudo_opcode_table,
	visass_ab3_mnemonic_table,
	AB3_ADDRESSING_MODES,
	// meaning of input bytes (0x80-0xec differ between AB3 and F8AB)
	0x80,	// first mnemonic
	40,	// count
	0xc8,	// first pseudo opcode
	13,	// count
	0xd5,	// first unused value
	42	// count
};


// main
void ab3_main(void)
{
	IO_set_input_padding(AB_ENDOFLINE);
	visass_ab3_illegals();
	IO_process_load_address();
	// first byte after load address should be AB_ENDOFLINE in AB3 sources
	if (IO_get_byte() == AB_ENDOFLINE) {
		IO_get_byte();	// skip it and pre-read first valid byte
		fputs("Input has AB3 header.\n", stderr);
	} else {
		fputs("Input does not have any known AB3 header.\n", stderr);
	}
	AB_main(&ab3_conf);
}
