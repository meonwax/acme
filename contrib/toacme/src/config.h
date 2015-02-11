// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// Configurable stuff
// ...this file gets included by almost all others, even *.h files
//
#ifndef config_H
#define config_H


// Types
//
typedef unsigned char	byte_t;


// Constants
//
#ifndef FALSE
typedef int	bool;
#define FALSE	0
#define TRUE	1
#endif


#endif
