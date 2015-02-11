// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// input/output
//


// Includes
//
#include <stdio.h>
#include "config.h"
#include "io.h"


// Variables
//
byte_t	PaddingValue;
FILE	*global_input_stream;
FILE	*global_output_stream;
byte_t	GotByte		= 0;
bool	ReachedEOF	= FALSE;


// Input functions
//


// Set byte sent after EOF
//
inline void input_set_padding(byte_t pad) {

	PaddingValue = pad;
}

// Fetch and buffer byte
//
byte_t GetByte(void) {
	int	w;

	if(ReachedEOF)
		GotByte = PaddingValue;
	else {
		w = getc(global_input_stream);
		if(w == EOF)
			ReachedEOF = TRUE;
		GotByte = (byte_t) w;
	}
	return(GotByte);
}

// Read little-endian 16-bit value
//
unsigned int GetLE16(void) {
	return(GetByte() | (GetByte() << 8));
}


// Output functions
//


// Write byte to output file
//
inline void PutByte(const char b) {
	putc(b, global_output_stream);
}

// output low nibble of argument as hexadecimal digit
//
void put_low_nibble_hex(int v) {
	putc("0123456789abcdef"[v & 15], global_output_stream);
}

// output low byte of argument as two hexadecimal digits
//
void io_put_low_byte_hex(int v) {
	put_low_nibble_hex(v >> 4);
	put_low_nibble_hex(v);
}

// output low 16 bits of arg as four hexadecimal digits
//
void io_put_low_16b_hex(int w) {
	io_put_low_byte_hex(w >> 8);
	io_put_low_byte_hex(w);
}

// read load address from input file and write as comment to output file
//
void io_process_load_address(void) {
	int	load_address;

	load_address = GetLE16();
	fputs("; ToACME: Original source code file had load address $", global_output_stream);
	io_put_low_16b_hex(load_address);
	PutByte('\n');
}
