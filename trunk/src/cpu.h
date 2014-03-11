// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// CPU type stuff
#ifndef cpu_type_H
#define cpu_type_H


#include "config.h"


// CPU type structure definition
struct cpu_type {
	// This function is not allowed to change GlobalDynaBuf
	// because that's where the mnemonic is stored!
	int	(*keyword_is_mnemonic)(int);
	int	flags;	// see below for bit meanings
	char	default_align_value;
};
#define	CPUFLAG_INDIRECTJMPBUGGY	(1u << 0)	// warn if "jmp ($xxff)" is assembled
#define CPUFLAG_SUPPORTSLONGREGS	(1u << 1)	// allow "!al" and "!rl" pseudo opcodes
#define CPUFLAG_AB_NEEDS_0_ARG		(1u << 2)	// warn if "lxa #$xx" uses non-zero arg

// create cpu type tree (is done early)
extern void CPUtype_init(void);

// register pseudo opcodes (done later)
extern void CPU_init(void);
// set default value for pass
extern void CPU_passinit(const struct cpu_type *cpu_type);
// try to find CPU type held in DynaBuf. Returns whether succeeded.
// FIXME - why not simply return struct ptr, or NULL in case of failure?
extern int CPU_find_cpu_struct(const struct cpu_type **target);


#endif
