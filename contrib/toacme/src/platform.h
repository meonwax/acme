// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// Platform specific stuff
//
#ifndef platform_H
#define platform_H


// check for RISC OS
#ifdef __riscos__
#define PLATFORM_VERSION	"Ported to RISC OS by Marco Baye."
extern void	Platform_SetFileType(const char *Filename, int Filetype);
#define PLATFORM_SETFILETYPE(a, b)	Platform_SetFileType(a, b);
#define FILETYPE_TEXT	0xfff	// File types used
#endif

// all other platforms
#ifndef PLATFORM_VERSION
#define PLATFORM_VERSION	"Platform independent version."
#define PLATFORM_SETFILETYPE(a, b)
#endif

// fix umlaut for DOS version
#ifdef __DJGPP__
#define STEFAN	"Stefan HÅbner"
#else
#define STEFAN	"Stefan H¸bner"
#endif


#endif
