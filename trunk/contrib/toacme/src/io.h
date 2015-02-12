// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// input/output
//
#ifndef io_H
#define io_H


// Includes
//
#include <stdio.h>
#include "config.h"


// Variables
//
extern int	GotByte;
extern bool	ReachedEOF;
extern FILE	*global_input_stream;
extern FILE	*global_output_stream;


// Prototypes
//
extern void		input_set_padding(const int);
extern int		GetByte(void);
extern unsigned int	GetLE16(void);	// get little-endian 16-bit value
extern void		PutString(const char string[]);
extern void		PutByte(const char b);
extern void		io_put_low_byte_hex(int v);
extern void		io_put_low_16b_hex(int w);
extern void		io_process_load_address(void);


#endif
