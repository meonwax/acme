// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// pseudo opcode stuff
#ifndef pseudoopcodes_H
#define pseudoopcodes_H


// FIXME - after grouping all pseudo opcodes in .c file, make this static:
extern struct ronode	*pseudo_opcode_tree;	// tree to hold pseudo opcodes


// call when "* = EXPRESSION" is parsed
extern void notreallypo_setpc(void);
// register pseudo opcodes
extern void pseudoopcodes_init(void);


#endif
