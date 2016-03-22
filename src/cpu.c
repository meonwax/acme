// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// CPU type stuff
#include "cpu.h"
#include "config.h"
#include "alu.h"
#include "dynabuf.h"
#include "global.h"	// FIXME - remove when no longer needed
#include "input.h"
#include "mnemo.h"
#include "output.h"
#include "tree.h"


// constants
static struct cpu_type	cpu_type_6502	= {
	keyword_is_6502mnemo,
	CPUFLAG_INDIRECTJMPBUGGY,	// JMP ($xxFF) is buggy
	234	// !align fills with "NOP"
};
static struct cpu_type	cpu_type_6510	= {
	keyword_is_6510mnemo,
	CPUFLAG_INDIRECTJMPBUGGY | CPUFLAG_8B_AND_AB_NEED_0_ARG,	// JMP ($xxFF) is buggy, ANE/LXA #$xx are unstable unless arg is $00
	234	// !align fills with "NOP"
};
static struct cpu_type	cpu_type_c64dtv2	= {
	keyword_is_c64dtv2mnemo,
	CPUFLAG_INDIRECTJMPBUGGY | CPUFLAG_8B_AND_AB_NEED_0_ARG,	// JMP ($xxFF) is buggy, ANE/LXA #$xx are unstable unless arg is $00
	234	// !align fills with "NOP"
};
static struct cpu_type	cpu_type_65c02	= {
	keyword_is_65c02mnemo,
	0,	// no flags
	234	// !align fills with "NOP"
};
/*
static struct cpu_type	cpu_type_Rockwell65c02	= {
	keyword_is_Rockwell65c02mnemo,
	0,	// no flags
	234	// !align fills with "NOP"
};
static struct cpu_type	cpu_type_WDC65c02	= {
	keyword_is_WDC65c02mnemo,
	0,	// no flags
	234	// !align fills with "NOP"
};
*/
static struct cpu_type	cpu_type_65816	= {
	keyword_is_65816mnemo,
	CPUFLAG_SUPPORTSLONGREGS,	// allows A and XY to be 16bits wide
	234	// !align fills with "NOP"
};


// variables

// predefined stuff
static struct ronode	*cputype_tree	= NULL;
static struct ronode	cputype_list[]	= {
#define KNOWN_TYPES	"'6502', '6510', 'c64dtv2', '65c02', '65816'"	// shown in CLI error message for unknown types
//	PREDEFNODE("z80",		&cpu_type_Z80),
	PREDEFNODE("6502",		&cpu_type_6502),
	PREDEFNODE("6510",		&cpu_type_6510),
	PREDEFNODE("c64dtv2",		&cpu_type_c64dtv2),
	PREDEFNODE("65c02",		&cpu_type_65c02),
//	PREDEFNODE("Rockwell65c02",	&cpu_type_Rockwell65c02),
//	PREDEFNODE("WDC65c02",		&cpu_type_WDC65c02),
	PREDEFLAST("65816",		&cpu_type_65816),
	//    ^^^^ this marks the last element
};
const char	cputype_names[]	= KNOWN_TYPES;	// string to show if cputype_find() returns NULL

// lookup cpu type held in DynaBuf and return its struct pointer (or NULL on failure)
const struct cpu_type *cputype_find(void)
{
	void	*node_body;

	// make sure tree is initialised
	if (cputype_tree == NULL)
		Tree_add_table(&cputype_tree, cputype_list);
	// perform lookup
	if (!Tree_easy_scan(cputype_tree, &node_body, GlobalDynaBuf))
		return NULL;

	return node_body;
}


// if cpu type and value match, set register length variable to value.
// if cpu type and value don't match, complain instead.
// FIXME - error message might be confusing if it is thrown not because of
// initial change, but because of reverting back to old cpu type after "{}" block!
void vcpu_check_and_set_reg_length(int *var, int make_long)
{
	if (((CPU_state.type->flags & CPUFLAG_SUPPORTSLONGREGS) == 0) && make_long)
		Throw_error("Chosen CPU does not support long registers.");
	else
		*var = make_long;
}


// set default values for pass
void cputype_passinit(const struct cpu_type *cpu_type)
{
	// handle cpu type (default is 6502)
	CPU_state.type = cpu_type ? cpu_type : &cpu_type_6502;
}
