// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// Converting CBM screen code to ISO 8859/1
#ifndef scr2iso_H
#define scr2iso_H

#include "config.h"


// constants
extern const char	SCR2ISO_table[256];	// Conversion table
#define SCR2ISO(v)	(SCR2ISO_table[(unsigned char) v])


#endif
