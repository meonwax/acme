// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// Platform specific stuff

#include "platform.h"


#ifdef __riscos__
//
// RISC OS
//
#include <kernel.h>	// defines _kernel_swi_regs
#define OS_FILE	0x00008	// constant to call relevant SWI

// setting the created files' types
void platform_set_file_type_text(const char *filename)
{
	_kernel_swi_regs	register_set;

	register_set.r[0] = 18;// = SetFileType
	register_set.r[1] = (int) filename;
	register_set.r[2] = 0xfff;
	_kernel_swi(OS_FILE, &register_set, &register_set);
}

#else
//
// other OS (not that much here)
//


#endif
