// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// section stuff (move to symbol.h?)
#include "section.h"
#include "config.h"
#include "dynabuf.h"
#include "global.h"	// FIXME - remove when no longer needed
#include "input.h"
#include "symbol.h"
#include "tree.h"


// fake section structure (for error msgs before any real section is in use)
static struct section	initial_section	= {
	0,		// scope value
	"during",	// "type"	=> normally "zone Title" or
	"init",		// "title"	=>  "macro test", now "during init"
	FALSE,		// no, title was not malloc'd
};


// variables
struct section		*section_now	= &initial_section;	// current section
static struct section	outer_section;	// outermost section struct
static scope_t		scope_localcount;	// highest scope number yet


// write given info into given structure and activate it
void section_new(struct section *section, const char *type, char *title, int allocated)
{
	section->scope = ++scope_localcount;
	section->type = type;
	section->title = title;
	section->allocated = allocated;
	// activate new section
	section_now = section;
	//printf("[new zone %d: %s, %s]\n", section->scope, section->type, section->title);
}

// Tidy up: If necessary, release section title.
// Warning - the state of the component "Allocd" may have
// changed in the meantime, so don't rely on a local variable.
void section_finalize(struct section *section)
{
	if (section->allocated)
		free(section->title);
}

// setup outermost section
void section_passinit(void)
{
	scope_localcount = SCOPE_GLOBAL;	// will be incremented by next line
	section_new(&outer_section, s_Zone, s_untitled, FALSE);
}
