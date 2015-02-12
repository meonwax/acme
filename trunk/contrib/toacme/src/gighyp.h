// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// stuff needed for both "Hypra-Ass" and "Giga-Ass"
#ifndef gigahypra_H
#define gigahypra_H


#include "config.h"


// Constants
#define FLAG_INSERT_SPACE	(1u << 0)	// insert space before arg
#define FLAG_ADD_LEFT_BRACE	(1u << 1)	// add '{' at end of statement
#define FLAG_ADD_CBM		(1u << 2)	// add file format indicator
#define FLAG_ADD_ZERO		(1u << 3)	// giga string specialty:
	// open quote at end of line is *normal*. Closed quote: add ",0".
#define FLAG_SKIP_OPENING	(1u << 4)	// strip '(' before args
#define FLAG_SKIP_CLOSING	(1u << 5)	// strip ')' after args
#define FLAG_CHANGE_LEFTARROW	(1u << 6)	// giga string specialty:
	// '_' (left arrow on C64) is transformed to CR (0x0d).


// Prototypes
extern void GigaHypra_comment(void);
extern void GigaHypra_operator(void);
extern void GigaHypra_indent(int indent);
extern void GigaHypra_argument(int flags);
extern int GigaHypra_label_definition(void);


#endif
