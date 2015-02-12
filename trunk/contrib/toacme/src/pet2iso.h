// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// Converting CBM PetSCII code to ISO 8859/1
#ifndef pet2iso_H
#define pet2iso_H


#include "config.h"


// constants
extern const char	PET2ISO_table[256];	// conversion table
#define PET2ISO(v)	(PET2ISO_table[(unsigned char) v])


#endif
