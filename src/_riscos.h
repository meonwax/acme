// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// Platform specific stuff (in this case, for RISC OS)
#ifndef platform_H
#define platform_H

#include "config.h"


// symbolic constants and macros

// called once on program startup (could register exit handler, if needed)
#define PLATFORM_INIT			RISCOS_entry()

// convert UNIX-style pathname to RISC OS-style pathname
#define PLATFORM_CONVERTPATHCHAR(a)	RISCOS_convert_path_char(a)

// directory separator for include paths
#define DIRECTORY_SEPARATOR	'\0'	// actually '.', but paths ending on ':' are ok, so auto-adding '.' is bad)

// string containing the prefix for accessing files from the library tree
#define PLATFORM_LIBPREFIX	"ACME_Lib:"
#define NO_NEED_FOR_ENV_VAR

// setting file types of created files
#define PLATFORM_SETFILETYPE_APPLE(a)	RISCOS_set_filetype(a, 0xffd)	// FIXME - wrong value!
#define PLATFORM_SETFILETYPE_CBM(a)	RISCOS_set_filetype(a, 0x064)
#define PLATFORM_SETFILETYPE_PLAIN(a)	RISCOS_set_filetype(a, 0xffd)
#define PLATFORM_SETFILETYPE_TEXT(a)	RISCOS_set_filetype(a, 0xfff)

// platform specific message output
#define PLATFORM_WARNING(a)		RISCOS_throwback(a, 0)
#define PLATFORM_ERROR(a)		RISCOS_throwback(a, 1)
#define PLATFORM_SERIOUS(a)		RISCOS_throwback(a, 2)

// integer-to-character conversion
#define PLATFORM_UINT2CHAR(x)	\
do {				\
	x ^= x >> 16;		\
	x ^= x >>  8;		\
	x &= 255;		\
} while (0)

// output of platform-specific command line switches
#define PLATFORM_OPTION_HELP	\
"  -t, --throwback        use the DDEUtils module's \"throwback\" protocol\n"

// processing of platform-specific command line switches
#define PLATFORM_SHORTOPTION_CODE			\
	case 't':					\
		RISCOS_flags |= RISCOSFLAG_THROWBACK;	\
		break;
#define PLATFORM_LONGOPTION_CODE			\
	else if (strcmp(string, "throwback") == 0)	\
		RISCOS_flags |= RISCOSFLAG_THROWBACK;


// variables
extern int	RISCOS_flags;	// Holds platform-specific flags
#define RISCOSFLAG_THROWBACK	(1u << 0)	// use throwback protocol
#define RISCOSFLAG_THROWN	(1u << 1)	// throwback is active


// used as PLATFORM_INIT: registers exit handler
extern void RISCOS_entry(void);
// convert UNIX-style pathname to RISC OS-style pathname
extern char RISCOS_convert_path_char(char);
// setting the created files' types
extern void RISCOS_set_filetype(const char *, int);
// use DDEUtils module's "Throwback" protocol
extern void RISCOS_throwback(const char *, int);


#endif
