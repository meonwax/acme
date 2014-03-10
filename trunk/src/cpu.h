// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// CPU stuff
#ifndef cpu_H
#define cpu_H


#include "config.h"


// FIXME - create cpu_type.c file and move cpu type stuff to it
// CPU type structure definition
struct cpu_type {
	// This function is not allowed to change GlobalDynaBuf
	// because that's where the mnemonic is stored!
	int	(*keyword_is_mnemonic)(int);
	int	flags;
	char	default_align_value;
};
#define	CPUFLAG_INDIRECTJMPBUGGY	(1u << 0)	// warn if "jmp ($xxff)" is assembled
#define CPUFLAG_SUPPORTSLONGREGS	(1u << 1)	// allow "!al" and "!rl" pseudo opcodes
#define CPUFLAG_AB_NEEDS_0_ARG		(1u << 2)	// warn if "lxa #$xx" uses non-zero arg

// current CPU state
// FIXME - move struct definition to .c file and change other .c files' accesses to fn calls
struct cpu {
	const struct cpu_type	*type;		// current CPU type (default 6502)
	struct result_int_t	pc;		// current program counter (pseudo value)
	int			add_to_pc;	// add to PC after statement
	int			a_is_long;
	int			xy_are_long;
};


// variables
// FIXME - restrict visibility to cpu.c file
extern struct cpu	CPU_state;	// current CPU state


// FIXME - move to new cpu_type.h file
// create cpu type tree (is done early)
extern void CPUtype_init(void);

// register pseudo opcodes (done later)
extern void CPU_init(void);
// set default values for pass
extern void CPU_passinit(const struct cpu_type *cpu_type);
// set program counter to defined value (FIXME - allow undefined!)
extern void CPU_set_pc(intval_t new_pc, int flags);

// FIXME - move to new cpu_type.h file
// try to find CPU type held in DynaBuf. Returns whether succeeded.
// FIXME - why not simply return struct ptr, or NULL in case of failure?
extern int CPU_find_cpu_struct(const struct cpu_type **target);


#endif
