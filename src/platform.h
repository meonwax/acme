// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// Platform specific stuff


// Amiga
#ifdef _AMIGA
#define PLATFORM_VERSION	"Ported to AmigaOS by Christoph Mammitzsch."
#include "_amiga.h"
#endif

// DOS, OS/2 and Windows
#if defined(__DJGPP__) || defined(__OS2__) || defined(__Windows__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define PLATFORM_VERSION	"DOS/OS2/Win32 version. Compiled by Dirk Hoepfner"
#include "_dos.h"
#endif

// RISC OS
#ifdef __riscos__
#define PLATFORM_VERSION	"RISC OS version."
#include "_riscos.h"
#endif

// add further platform files here

// Unix/Linux/others (surprisingly also works on Windows)
#ifndef PLATFORM_VERSION
#define PLATFORM_VERSION	"Platform independent version."
#endif
#include "_std.h"
