// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// Character encoding stuff
#ifndef encoding_H
#define encoding_H


// prototypes

// register pseudo opcodes (FIXME - remove!)
extern void Encoding_init(void);
// convert character using current encoding
extern char encoding_encode_char(char);
// set "raw" as default encoding
extern void encoding_passinit(void);


#endif
