// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2015 Marco Baye
// Have a look at "acme.c" for further info
//
// Section stuff
#ifndef section_H
#define section_H


#include "config.h"


// "section" structure type definition
struct section {
	zone_t		zone;	// current zone value
	const char	*type;	// "Zone", "Subzone" or "Macro"
	char		*title;	// zone title, subzone title or macro title
	int		allocated;	// whether title was malloc()'d
};


// Constants
// TODO: rename internal stuff from "zone" to "scope",
// then add cheap locals (so there's SCOPE_GLOBAL, scope_zone and scope_cheap)
#define ZONE_GLOBAL	0	// Number of "global zone"


// Variables

// current section structure
extern struct section	*Section_now;


// Prototypes

// Write given info into given zone structure and activate it
extern void Section_new_zone(struct section *section, const char *type, char *title, int allocated);
// Setup outermost section
extern void Section_passinit(void);
// Tidy up: If necessary, release section title.
extern void Section_finalize(struct section *section);


#endif
