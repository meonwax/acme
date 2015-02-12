// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// stuff needed for "VisAss", "AssBlaster 3.x" and "Flash8-AssBlaster"
#ifndef ab_H
#define ab_H


#include "config.h"


// Definition of "Type of VisAss/AssBlaster" structure
struct vab {
	int		flags;
	int		(*number_parser) (void);
	const char	**pseudo_opcodes;
	const char	**mnemonics;
	int		address_mode_count;
	int		first_mnemonic;
	int		mnemonic_count;
	int		first_pseudo_opcode;
	int		pseudo_opcode_count;
	int		first_unused_byte_value;
	int		unused_values_count;
};
#define VABFLAG_HASLENGTHBYTE	(1u << 0)
#define VABFLAG_ADD_DOT		(1u << 1)


// Constants
extern const char	nothing[];
#define AB_ENDOFLINE	0xff
// meaning of internal error word. errors are collected until *after* a line
// has been finished so the warning messages don't interfere with the generated
// source code.
#define AB_ERRBIT_UNKNOWN_ADDRMODE		(1u << 0)
#define AB_ERRBIT_UNKNOWN_NUMBER_COMPRESSION	(1u << 1) // SIZEMASK invalid
#define AB_ERRBIT_UNKNOWN_NUMBER_FORMAT		(1u << 2) // FORMATMASK invalid
extern const char	*visass_ab3_mnemonic_table[];
extern const char	*visass_ab3_pseudo_opcode_table[];


// Prototypes
extern void	visass_ab3_illegals(void);
extern void	AB_output_binary(unsigned long int value);
extern void	AB_output_hexadecimal(unsigned long int value);
extern void	AB_main(struct vab *client_config);


#endif
