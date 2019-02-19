// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2017 Marco Baye
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


#define SCOPE_INCREMENT	2	// inc by 2 so locals are even and cheaps are odd

// fake section structure (for error msgs before any real section is in use)
static struct section	initial_section	= {
	0,		// local scope value
	1,		// cheap scope value
	"during",	// "type"	=> normally "zone Title" or
	"init",		// "title"	=>  "macro test", now "during init"
	FALSE,		// no, title was not malloc'd
};


// variables
struct section		*section_now	= &initial_section;	// current section
static struct section	outer_section;	// outermost section struct
static scope_t		local_scope_max;	// highest scope number yet
static scope_t		cheap_scope_max;	// highest scope number yet


// write given info into given structure and activate it
void section_new(struct section *section, const char *type, char *title, int allocated)
{
	// new scope for locals
	local_scope_max += SCOPE_INCREMENT;
	section->local_scope = local_scope_max;
	// keep scope for cheap locals
	section->cheap_scope = section_now->cheap_scope;
	// copy other data
	section->type = type;
	section->title = title;
	section->allocated = allocated;
	// activate new section
	section_now = section;
	//printf("[new section %d: %s, %s]\n", section->local_scope, section->type, section->title);
}


// change scope of cheap locals in given section
void section_new_cheap_scope(struct section *section)
{
	// new scope for cheap locals
	cheap_scope_max += SCOPE_INCREMENT;
	section->cheap_scope = cheap_scope_max;
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
	//printf("[old maxima: locals=%d, cheap=%d]\n", local_scope_max, cheap_scope_max);
	local_scope_max = 0;	// will be incremented by 2 by next line
	section_new(&outer_section, s_Zone, s_untitled, FALSE);
	cheap_scope_max = -1;	// will be incremented by 2 by next line
	section_new_cheap_scope(&outer_section);
}
