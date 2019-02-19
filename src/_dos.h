// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// Platform specific stuff (in this case, for DOS, OS/2 and Windows)
#ifndef platform_H
#define platform_H


#include "config.h"


// symbolic constants and macros

// called once on program startup (could register exit handler, if needed)
#define PLATFORM_INIT		DOS_entry()

// convert UNIX-style pathname to DOS-style pathname
#define PLATFORM_CONVERTPATHCHAR(a)	DOS_convert_path_char(a)

// directory separator for include paths
#define DIRECTORY_SEPARATOR	'\\'

// string containing the prefix for accessing files from the library tree
#define PLATFORM_LIBPREFIX	DOS_lib_prefix

// setting file types of created files
#define PLATFORM_SETFILETYPE_APPLE(a)
#define PLATFORM_SETFILETYPE_CBM(a)
#define PLATFORM_SETFILETYPE_PLAIN(a)
#define PLATFORM_SETFILETYPE_TEXT(a)

// platform specific message output
#define PLATFORM_WARNING(a)
#define PLATFORM_ERROR(a)
#define PLATFORM_SERIOUS(a)

// integer-to-character conversion
#define PLATFORM_UINT2CHAR(x)	\
do {				\
	x ^= x >> 16;		\
	x ^= x >>  8;		\
	x &= 255;		\
} while (0)

// output of platform-specific command line switches
#define PLATFORM_OPTION_HELP

// processing of platform-specific command line switches
#define PLATFORM_SHORTOPTION_CODE
#define PLATFORM_LONGOPTION_CODE


// variables
extern char *DOS_lib_prefix;	// header string of library tree


// used as PLATFORM_INIT: reads "ACME" environment variable
extern void DOS_entry(void);
// Convert UNIX-style pathname character to DOS-style pathname character
extern char DOS_convert_path_char(char);


#endif
