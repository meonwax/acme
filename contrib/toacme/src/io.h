// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// input/output
#ifndef io_H
#define io_H


#include <stdio.h>
#include "config.h"


// variables
extern int	GotByte;
extern bool	IO_reached_eof;
extern FILE	*global_input_stream;
extern FILE	*global_output_stream;


// prototypes
extern void IO_set_input_padding(int);
extern int IO_get_byte(void);
extern unsigned int IO_get_le16(void);	// get little-endian 16-bit value
extern unsigned int IO_get_le24(void);	// get little-endian 24-bit value
extern void IO_put_string(const char string[]);
extern void IO_put_byte(char b);
extern void IO_put_low_byte_hex(int v);
extern void IO_put_low_16b_hex(int w);
extern void IO_process_load_address(void);


#endif
