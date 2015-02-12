// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// input/output

#include <stdio.h>
#include "config.h"
#include "io.h"


// variables
int	padding_value;
FILE	*global_input_stream;
FILE	*global_output_stream;
int	GotByte		= 0;
bool	IO_reached_eof	= FALSE;


// input functions


// set byte sent after EOF
inline void IO_set_input_padding(int pad)
{
	padding_value = pad;
}


// fetch and buffer byte
int IO_get_byte(void)
{
	int	w;

	if (IO_reached_eof) {
		GotByte = padding_value;
	} else {
		w = getc(global_input_stream);
		if (w == EOF)
			IO_reached_eof = TRUE;
		GotByte = w;
	}
	return GotByte;
}


// read little-endian 16-bit value
unsigned int IO_get_le16(void)
{
	unsigned int	result	= IO_get_byte();

	// CAUTION! using
	//   return(IO_get_byte() | (IO_get_byte() << 8));
	// would be compiler-dependent
	return result | (IO_get_byte() << 8);
}


// read little-endian 24-bit value
unsigned int IO_get_le24(void)
{
	unsigned int	result	= IO_get_le16();

	// CAUTION! see above
	return result | (IO_get_byte() << 16);
}


// output functions


// output string
inline void IO_put_string(const char string[])
{
	fputs(string, global_output_stream);
}


// write byte to output file
inline void IO_put_byte(char b)
{
	putc(b, global_output_stream);
}


// output low nibble of argument as hexadecimal digit
static void put_low_nibble_hex(int v)
{
	putc("0123456789abcdef"[v & 15], global_output_stream);
}


// output low byte of argument as two hexadecimal digits
void IO_put_low_byte_hex(int v)
{
	put_low_nibble_hex(v >> 4);
	put_low_nibble_hex(v);
}


// output low 16 bits of arg as four hexadecimal digits
void IO_put_low_16b_hex(int w)
{
	IO_put_low_byte_hex(w >> 8);
	IO_put_low_byte_hex(w);
}


// read load address from input file and write as comment to output file
void IO_process_load_address(void)
{
	int	load_address;

	load_address = IO_get_le16();
	IO_put_string("; ToACME: Original source code file had load address $");
	IO_put_low_16b_hex(load_address);
	IO_put_byte('\n');
}
