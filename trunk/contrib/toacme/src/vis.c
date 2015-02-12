// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2015 Marco Baye
// Have a look at "main.c" for further info
//
// VisAss stuff

#include "ab.h"
#include "acme.h"
#include "io.h"


static int number_parser(void)
{
	return 0;
}


// config struct for shared VisAss/AB code
struct vab	visass_conf	= {
	VABFLAG_HASLENGTHBYTE,
	number_parser,
	visass_ab3_pseudo_opcode_table,
	visass_ab3_mnemonic_table,
	2,	// we fake absolute addressing because VisAss does not encode the addr mode
	// meaning of input bytes (0x80-0xec differ between VisAss, AB3 and F8AB)
	0,	// first mnemonic
	0x48,	// count
	0x48,	// first pseudo opcode
	14,	// count
	0x100,	// first unused value (dummy)
	1	// count (dummy)
};


// main
void visass_main(void)
{
	IO_set_input_padding(AB_ENDOFLINE);
	visass_ab3_illegals();
	IO_process_load_address();
	// first byte after load address should be AB_ENDOFLINE in VisAss sources
	if (IO_get_byte() == AB_ENDOFLINE) {
		IO_get_byte();	// skip it and pre-read first valid byte
		fputs("Input has VisAss header.\n", stderr);
	} else {
		fputs("Input does not have any known VisAss header.\n", stderr);
	}
	AB_main(&visass_conf);
}
