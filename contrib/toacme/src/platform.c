// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// Platform specific stuff
//


// Includes
//
#include "platform.h"


#ifdef __riscos__
//
// RISC OS
//
#include <kernel.h>	// defines _kernel_swi_regs
#define OS_FILE	0x00008	// constant to call relevant SWI

// Setting the created files' types
void Platform_SetFileType(const char *Filename, int Filetype) {
	_kernel_swi_regs	RegSet;

	RegSet.r[0] = 18;// = SetFileType
	RegSet.r[1] = (int) Filename;
	RegSet.r[2] = Filetype;
	_kernel_swi(OS_FILE, &RegSet, &RegSet);
}

#else
//
// other OS (not that much here)
//

#endif
