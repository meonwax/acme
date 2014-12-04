// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// CPU type stuff
#include "config.h"
#include "alu.h"
#include "cpu.h"
#include "dynabuf.h"
#include "global.h"	// FIXME - remove when no longer needed
#include "input.h"
#include "mnemo.h"
#include "output.h"
#include "pseudoopcodes.h"	// FIXME - remove when no longer needed
#include "tree.h"


// constants
static struct cpu_type	cpu_type_6502	= {
	keyword_is_6502mnemo,
	CPUFLAG_INDIRECTJMPBUGGY,	// JMP ($xxFF) is buggy
	234			// !align fills with "NOP"
};
static struct cpu_type	cpu_type_6510	= {
	keyword_is_6510mnemo,
	CPUFLAG_INDIRECTJMPBUGGY |	// JMP ($xxFF) is buggy
		CPUFLAG_8B_AND_AB_NEED_0_ARG,	// ANE/LXA #$xx are unstable unless arg is $00
	234			// !align fills with "NOP"
};
static struct cpu_type	cpu_type_c64dtv2	= {
	keyword_is_c64dtv2mnemo,
	CPUFLAG_INDIRECTJMPBUGGY |	// JMP ($xxFF) is buggy
		CPUFLAG_8B_AND_AB_NEED_0_ARG,	// ANE/LXA #$xx are unstable unless arg is $00 (FIXME - correct?)
	234			// !align fills with "NOP"
};
static struct cpu_type	cpu_type_65c02	= {
	keyword_is_65c02mnemo,
	0,			// no flags
	234			// !align fills with "NOP"
};
/*
static struct cpu_type	cpu_type_Rockwell65c02	= {
	keyword_is_Rockwell65c02mnemo,
	0,			// no flags
	234			// !align fills with "NOP"
};
static struct cpu_type	cpu_type_WDC65c02	= {
	keyword_is_WDC65c02mnemo,
	0,			// no flags
	234			// !align fills with "NOP"
};
*/
static struct cpu_type	cpu_type_65816	= {
	keyword_is_65816mnemo,
	CPUFLAG_SUPPORTSLONGREGS,	// allows A and XY to be 16bits wide
	234			// !align fills with "NOP"
};
#define s_rl	(s_brl + 1)	// Yes, I know I'm sick


// variables

// predefined stuff
static struct ronode	*cputype_tree	= NULL;
static struct ronode	cputype_list[]	= {
//	PREDEFNODE("z80",		&cpu_type_Z80),
	PREDEFNODE("6502",		&cpu_type_6502),
	PREDEFNODE("6510",		&cpu_type_6510),
	PREDEFNODE("c64dtv2",		&cpu_type_c64dtv2),
	PREDEFNODE("65c02",		&cpu_type_65c02),
//	PREDEFNODE("Rockwell65c02",	&cpu_type_Rockwell65c02),
//	PREDEFNODE("WDC65c02",		&cpu_type_WDC65c02),
	PREDEFLAST(s_65816,		&cpu_type_65816),
	//    ^^^^ this marks the last element
};


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
static void check_and_set_reg_length(int *var, int make_long)
{
	if (((CPU_state.type->flags & CPUFLAG_SUPPORTSLONGREGS) == 0) && make_long)
		Throw_error("Chosen CPU does not support long registers.");
	else
		*var = make_long;
}


// set register length, block-wise if needed.
static enum eos set_register_length(int *var, int make_long)
{
	int	old_size	= *var;

	// set new register length (or complain - whichever is more fitting)
	check_and_set_reg_length(var, make_long);
	// if there's a block, parse that and then restore old value!
	if (Parse_optional_block())
		check_and_set_reg_length(var, old_size);	// restore old length
	return ENSURE_EOS;
}


// set default values for pass
void CPU_passinit(const struct cpu_type *cpu_type)
{
	// handle cpu type (default is 6502)
	CPU_state.type = cpu_type ? cpu_type : &cpu_type_6502;
}


// FIXME - move to pseudoopcodes.c:


// insert byte until PC fits condition
static enum eos po_align(void)
{
	// FIXME - read cpu state via function call!
	intval_t	and,
			equal,
			fill,
			test	= CPU_state.pc.val.intval;

	// make sure PC is defined.
	if ((CPU_state.pc.flags & MVALUE_DEFINED) == 0) {
		Throw_error(exception_pc_undefined);
		CPU_state.pc.flags |= MVALUE_DEFINED;	// do not complain again
		return SKIP_REMAINDER;
	}

	and = ALU_defined_int();
	if (!Input_accept_comma())
		Throw_error(exception_syntax);
	equal = ALU_defined_int();
	if (Input_accept_comma())
		fill = ALU_any_int();
	else
		fill = CPU_state.type->default_align_value;
	while ((test++ & and) != equal)
		output_8(fill);
	return ENSURE_EOS;
}


// select CPU ("!cpu" pseudo opcode)
static enum eos po_cpu(void)
{
	const struct cpu_type	*cpu_buffer	= CPU_state.type;	// remember current cpu
	const struct cpu_type	*new_cpu_type;

	if (Input_read_and_lower_keyword()) {
		new_cpu_type = cputype_find();
		if (new_cpu_type)
			CPU_state.type = new_cpu_type;
		else
			Throw_error("Unknown processor.");
	}
	// if there's a block, parse that and then restore old value!
	if (Parse_optional_block())
		CPU_state.type = cpu_buffer;
	return ENSURE_EOS;
}


static const char	Error_old_offset_assembly[]	=
	"\"!pseudopc/!realpc\" is obsolete; use \"!pseudopc {}\" instead.";


// "!realpc" pseudo opcode (now obsolete)
static enum eos po_realpc(void)
{
	Throw_error(Error_old_offset_assembly);
	return ENSURE_EOS;
}


// start offset assembly
// FIXME - split into PO (move to pseudoopcodes.c) and backend (move to output.c?)
// TODO - maybe add a label argument to assign the block size afterwards (for assemble-to-end-address) (or add another pseudo opcode)
static enum eos po_pseudopc(void)
{
	// FIXME - read pc using a function call!
	intval_t	new_pc,
			new_offset;
	int		outer_flags	= CPU_state.pc.flags;

	// set new
	new_pc = ALU_defined_int();	// FIXME - allow for undefined!
	new_offset = (new_pc - CPU_state.pc.val.intval) & 0xffff;
	CPU_state.pc.val.intval = new_pc;
	CPU_state.pc.flags |= MVALUE_DEFINED;	// FIXME - remove when allowing undefined!
	// if there's a block, parse that and then restore old value!
	if (Parse_optional_block()) {
		// restore old
		CPU_state.pc.val.intval = (CPU_state.pc.val.intval - new_offset) & 0xffff;
		CPU_state.pc.flags = outer_flags;
	} else {
		// not using a block is no longer allowed
		Throw_error(Error_old_offset_assembly);
	}
	return ENSURE_EOS;
}


// switch to long accumulator ("!al" pseudo opcode)
static enum eos po_al(void)
{
	return set_register_length(&CPU_state.a_is_long, TRUE);
}


// switch to short accumulator ("!as" pseudo opcode)
static enum eos po_as(void)
{
	return set_register_length(&CPU_state.a_is_long, FALSE);
}


// switch to long index registers ("!rl" pseudo opcode)
static enum eos po_rl(void)
{
	return set_register_length(&CPU_state.xy_are_long, TRUE);
}


// switch to short index registers ("!rs" pseudo opcode)
static enum eos po_rs(void)
{
	return set_register_length(&CPU_state.xy_are_long, FALSE);
}


// pseudo opcode table
static struct ronode	pseudo_opcodes[]	= {
	PREDEFNODE("align",	po_align),
	PREDEFNODE("cpu",	po_cpu),
	PREDEFNODE("pseudopc",	po_pseudopc),
	PREDEFNODE("realpc",	po_realpc),	// obsolete
	PREDEFNODE("al",	po_al),
	PREDEFNODE("as",	po_as),
	PREDEFNODE(s_rl,	po_rl),
	PREDEFLAST("rs",	po_rs),
	//    ^^^^ this marks the last element
};


// register pseudo opcodes (done later)
void CPU_init(void)
{
	Tree_add_table(&pseudo_opcode_tree, pseudo_opcodes);
}
