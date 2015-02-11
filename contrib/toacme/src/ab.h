// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// stuff needed for both "AssBlaster 3.x" and "F8-AssBlaster"
//
#ifndef ab_H
#define ab_H


// Includes
//
#include "config.h"


// Types
//
struct ab_t {
	int		(*number_parser)(void);
	const char**	pseudo_opcodes;
	const char**	mnemonics;
	int		address_mode_count;
	int		first_pseudo_opcode;
	int		first_unused_byte_value;
};


// Constants
//
#define AB_ENDOFLINE	0xff
// meaning of internal error word. errors are collected until *after* a line
// has been finished so the warning messages don't interfere with the generated
// source code.
#define AB_ERRBIT_UNKNOWN_ADDRMODE		0x01
#define AB_ERRBIT_UNKNOWN_NUMBER_COMPRESSION	0x02	// invalid contents of SIZEMASK
#define AB_ERRBIT_UNKNOWN_NUMBER_FORMAT		0x04	// invalid contents of FORMATMASK




// Prototypes
//
extern void	ab_output_binary(unsigned long int v);
extern void	ab_output_hexadecimal(unsigned long int v);
extern void	ab_main(struct ab_t* client_config);

#endif
