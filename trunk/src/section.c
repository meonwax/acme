// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// Section stuff
#include "section.h"
#include "config.h"
#include "dynabuf.h"
#include "global.h"	// FIXME - remove when no longer needed
#include "input.h"
#include "tree.h"


// fake section structure (for error msgs before any real section is in use)
static struct section	initial_section	= {
	0,		// zone value
	"during",	// "type"	=> normally "zone Title" or
	"init",		// "title"	=>  "macro test", now "during init"
	FALSE,		// no, title was not malloc'd
};


// Variables
struct section		*Section_now	= &initial_section;	// current section
static struct section	outer_section;	// outermost section struct
static zone_t		zone_max;	// Highest zone number yet

// Write given info into given zone structure and activate it
void Section_new_zone(struct section *section, const char *type, char *title, int allocated)
{
	section->zone = ++zone_max;
	section->type = type;
	section->title = title;
	section->allocated = allocated;
	// activate new section
	Section_now = section;
	//printf("[new zone %d: %s, %s]\n", section->zone, section->type, section->title);
}

// Tidy up: If necessary, release section title.
// Warning - the state of the component "Allocd" may have
// changed in the meantime, so don't rely on a local variable.
void Section_finalize(struct section *section)
{
	if (section->allocated)
		free(section->title);
}

// Setup outermost section
void Section_passinit(void)
{
	zone_max = ZONE_GLOBAL;	// will be incremented by next line
	Section_new_zone(&outer_section, s_Zone, s_untitled, FALSE);
}
