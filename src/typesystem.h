// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// Type system stuff
#ifndef typesystem_H
#define typesystem_H


#include "config.h"


// return whether explicit symbol definitions should force "address" mode
extern int typesystem_says_address(void);
// parse a block while forcing address mode
extern void typesystem_force_address_block(void);
// force address mode on or off for the next statement
extern void typesystem_force_address_statement(int value);
// warn if result is not integer
extern void typesystem_want_imm(struct result *result);
// warn if result is not address
extern void typesystem_want_addr(struct result *result);


#endif
