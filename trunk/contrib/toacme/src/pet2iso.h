// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// Converting CBM PetSCII code to ISO 8859/1
//
#ifndef pet2iso_H
#define pet2iso_H


// Includes
//
#include "config.h"


// Constants
//
extern const char	Pet2ISO_Table[256];	// Conversion table
#define PET2ISO(v)	(Pet2ISO_Table[(unsigned char) v])


#endif
