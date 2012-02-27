// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2009 Marco Baye
// Have a look at "acme.c" for further info
//
// CPU stuff
#ifndef cpu_H
#define cpu_H


#include "config.h"


// CPU type structure definition
struct cpu_t {
	// This function is not allowed to change GlobalDynaBuf
	// because that's where the mnemonic is stored!
	int	(*keyword_is_mnemonic)(int);
	int	flags;
	char	default_align_value;
	int	a_is_long;
	int	xy_are_long;
};
#define	CPUFLAG_INDIRECTJMPBUGGY	(1u << 0)
#define CPUFLAG_SUPPORTSLONGREGS	(1u << 1)


// variables
extern struct cpu_t		*CPU_now;	// struct of current CPU type (default 6502)
extern struct result_int_t	CPU_pc;	// current program counter (pseudo value)
extern int			CPU_2add;	// add to PC after statement


// create cpu type tree (is done early)
extern void CPUtype_init(void);
// register pseudo opcodes (done later)
extern void CPU_init(void);
// set default values for pass
extern void CPU_passinit(struct cpu_t *cpu_type);
// set program counter to defined value
extern void CPU_set_pc(intval_t new_pc);
// try to find CPU type held in DynaBuf. Returns whether succeeded.
extern int CPU_find_cpu_struct(struct cpu_t **target);
// return whether offset assembly is active
extern int CPU_uses_pseudo_pc(void);


#endif
