// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2009 Marco Baye
// Have a look at "acme.c" for further info
//
// Flow control stuff (loops, conditional assembly etc.)
#ifndef flow_H
#define flow_H


#include <stdio.h>
#include "config.h"


// Prototypes

// register pseudo opcodes
extern void Flow_init(void);
// parse a whole source code file
extern void Parse_and_close_file(FILE *fd, const char *filename);
// parse {block} [else {block}]
extern void flow_parse_block_else_block(int parse_first);


#endif
