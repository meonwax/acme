// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// Configurable stuff
// ...this file gets included by almost all others, even *.h files
#ifndef config_H
#define config_H


// constants
#define SPACE		0x20
#define SHIFTSPACE	0xa0

#ifndef FALSE
typedef int	bool;
#define FALSE	0
#define TRUE	1
#endif


#endif
