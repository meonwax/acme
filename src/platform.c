// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// Platform specific stuff
#include "platform.h"
#include "config.h"


// Amiga
#ifdef _AMIGA
#ifndef platform_C
#define platform_C
// Nothing here - Amigas don't need no stinkin' platform-specific stuff!
#endif
#endif

// DOS, OS/2 and Windows
#if defined(__DJGPP__) || defined(__OS2__) || defined(__Windows__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include "_dos.c"
#endif

// RISC OS
#ifdef __riscos__
#include "_riscos.c"
#endif

// add further platform files here

// Unix/Linux/others (surprisingly also works on at least some versions of Windows)
#include "_std.c"
