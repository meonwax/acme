// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// Platform specific stuff (in this case, for RISC OS)
#ifndef platform_C
#define platform_C


#include <stdlib.h>
#include <kernel.h>
#include "acme.h"
#include "input.h"


// constants

// SWIs
#define  OS_FILE			0x00008
#define XDDEUTILS_THROWBACKSTART	0x62587
#define XDDEUTILS_THROWBACKSEND		0x62588
#define XDDEUTILS_THROWBACKEND		0x62589


// variables
int	RISCOS_flags	= 0;	// used to store platform-specific flags


// exit handler: if throwback was used, de-register now
void RISCOS_exit(void)
{
	_kernel_swi_regs	regs;

	if (RISCOS_flags & RISCOSFLAG_THROWN) {
		_kernel_swi(XDDEUTILS_THROWBACKEND, &regs, &regs);
		RISCOS_flags &= ~RISCOSFLAG_THROWN;
	}
}


// used as PLATFORM_INIT: registers exit handler
void RISCOS_entry(void)
{
	atexit(RISCOS_exit);
}


// convert UNIX-style pathname to RISC OS-style pathname
char RISCOS_convert_path_char(char byte)
{
	if (byte == '.')
		return '/';
	if (byte == '/')
		return '.';
	if (byte == '?')
		return '#';
	if (byte == '#')
		return '?';
	return byte;
}


// setting the created files' types
void RISCOS_set_filetype(const char *filename, int file_type)
{
	_kernel_swi_regs	regs;

	regs.r[0] = 18;	// reason code (set file type)
	regs.r[1] = (int) filename;
	regs.r[2] = file_type;
	_kernel_swi(OS_FILE, &regs, &regs);
}


// throwback protocol: "type" can be 0, 1 or 2 (DDEUtils message types)
void RISCOS_throwback(const char *message, int type)
{
	_kernel_swi_regs	regs;

	// only use throwback protocol if wanted
	if ((RISCOS_flags & RISCOSFLAG_THROWBACK) == 0)
		return;
	// if this is the first throwback, set it up and send info
	if ((RISCOS_flags & RISCOSFLAG_THROWN) == 0) {
		RISCOS_flags |= RISCOSFLAG_THROWN;
		_kernel_swi(XDDEUTILS_THROWBACKSTART, &regs, &regs);
		regs.r[0] = 0;
		regs.r[1] = 0;
	//	regs.r[2] = (int) toplevel_source;
		regs.r[2] = (int) Input_now->original_filename;
		_kernel_swi(XDDEUTILS_THROWBACKSEND, &regs, &regs);
	}
	// send throwback message
	regs.r[0] = 1;
	regs.r[1] = 0;
	regs.r[2] = (int) Input_now->original_filename;
	regs.r[3] = Input_now->line_number;
	regs.r[4] = type;
	regs.r[5] = (int) message;
	_kernel_swi(XDDEUTILS_THROWBACKSEND, &regs, &regs);
}


#endif
