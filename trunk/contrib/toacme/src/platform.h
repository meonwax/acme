// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// Platform specific stuff
#ifndef platform_H
#define platform_H


// check for RISC OS
#ifdef __riscos__
#define PLATFORM_VERSION	"Ported to RISC OS by Marco Baye."
#define PLATFORM_SETFILETYPE_TEXT(a)	Platform_set_file_type_text(a);
extern void	Platform_set_file_type_text(const char *filename);
#endif

// all other platforms
#ifndef PLATFORM_VERSION
#define PLATFORM_VERSION	"Platform independent version."
#define PLATFORM_SETFILETYPE_TEXT(a)
#endif

// fix umlaut for DOS version
#ifdef __DJGPP__
#define STEFAN	"Stefan HÅbner"
#else
#define STEFAN	"Stefan H¸bner"
#endif


#endif
