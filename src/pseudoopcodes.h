// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// pseudo opcode stuff
#ifndef pseudoopcodes_H
#define pseudoopcodes_H


// call when "* = EXPRESSION" is parsed
extern void notreallypo_setpc(void);
// register pseudo opcodes
extern void pseudoopcodes_init(void);
// parse pseudo opcode. has to be re-entrant.
extern void pseudoopcode_parse(void);


#endif
