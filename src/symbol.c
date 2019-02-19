// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2017 Marco Baye
// Have a look at "acme.c" for further info
//
// symbol stuff
//
// 22 Nov 2007	"warn on indented labels" is now a CLI switch
// 25 Sep 2011	Fixed bug in !sl (colons in filename could be interpreted as EOS)
// 23 Nov 2014	Added label output in VICE format
#include "symbol.h"
#include <stdio.h>
#include "acme.h"
#include "alu.h"
#include "dynabuf.h"
#include "global.h"	// FIXME - remove when no longer needed
#include "input.h"
#include "output.h"
#include "platform.h"
#include "section.h"
#include "tree.h"
#include "typesystem.h"


// variables
struct rwnode	*symbols_forest[256]	= { NULL };	// because of 8-bit hash - must be (at least partially) pre-defined so array will be zeroed!


// Dump symbol value and flags to dump file
static void dump_one_symbol(struct rwnode *node, FILE *fd)
{
	struct symbol	*symbol	= node->body;

	// output name
	if (config.warn_on_type_mismatch
	&& symbol->result.addr_refs == 1)
		fprintf(fd, "!addr");
	fprintf(fd, "\t%s", node->id_string);
	switch (symbol->result.flags & MVALUE_FORCEBITS) {
	case MVALUE_FORCE16:
		fprintf(fd, "+2\t= ");
		break;
	case MVALUE_FORCE16 | MVALUE_FORCE24:
		/*FALLTHROUGH*/
	case MVALUE_FORCE24:
		fprintf(fd, "+3\t= ");
		break;
	default:
		fprintf(fd, "\t= ");
	}
	if (symbol->result.flags & MVALUE_DEFINED) {
		if (symbol->result.flags & MVALUE_IS_FP)
			fprintf(fd, "%.30f", symbol->result.val.fpval);	//FIXME %g
		else
			fprintf(fd, "$%x", (unsigned) symbol->result.val.intval);
	} else {
		fprintf(fd, " ?");	// TODO - maybe write "UNDEFINED" instead? then the file could at least be parsed without errors
	}
	if (symbol->result.flags & MVALUE_UNSURE)
		fprintf(fd, "\t; ?");	// TODO - write "forward" instead?
	if (symbol->usage == 0)
		fprintf(fd, "\t; unused");
	fprintf(fd, "\n");
}


// output symbols in VICE format (example: "al C:09ae .nmi1")
static void dump_vice_address(struct rwnode *node, FILE *fd)
{
	struct symbol	*symbol	= node->body;

	// dump address symbols even if they are not used
	if ((symbol->result.flags & MVALUE_DEFINED)
	&& !(symbol->result.flags & MVALUE_IS_FP)
	&& (symbol->result.addr_refs == 1))
		fprintf(fd, "al C:%04x .%s\n", (unsigned) symbol->result.val.intval, node->id_string);
}
static void dump_vice_usednonaddress(struct rwnode *node, FILE *fd)
{
	struct symbol	*symbol	= node->body;

	// dump non-addresses that are used
	if (symbol->usage
	&& (symbol->result.flags & MVALUE_DEFINED)
	&& !(symbol->result.flags & MVALUE_IS_FP)
	&& (symbol->result.addr_refs != 1))
		fprintf(fd, "al C:%04x .%s\n", (unsigned) symbol->result.val.intval, node->id_string);
}
static void dump_vice_unusednonaddress(struct rwnode *node, FILE *fd)
{
	struct symbol	*symbol	= node->body;

	// dump non-addresses that are unused
	if (!symbol->usage
	&& (symbol->result.flags & MVALUE_DEFINED)
	&& !(symbol->result.flags & MVALUE_IS_FP)
	&& (symbol->result.addr_refs != 1))
		fprintf(fd, "al C:%04x .%s\n", (unsigned) symbol->result.val.intval, node->id_string);
}


// search for symbol. create if nonexistant. if created, give it flags "flags".
// the symbol name must be held in GlobalDynaBuf.
struct symbol *symbol_find(scope_t scope, int flags)
{
	struct rwnode	*node;
	struct symbol	*symbol;
	int		node_created,
			force_bits	= flags & MVALUE_FORCEBITS;

	node_created = Tree_hard_scan(&node, symbols_forest, scope, TRUE);
	// if node has just been created, create symbol as well
	if (node_created) {
		// create new symbol structure
		symbol = safe_malloc(sizeof(*symbol));
		// finish empty symbol item
		symbol->result.flags = flags;
		symbol->result.addr_refs = 0;
		if (flags & MVALUE_IS_FP)
			symbol->result.val.fpval = 0;
		else
			symbol->result.val.intval = 0;
		symbol->usage = 0;	// usage count
		symbol->pass = pass_count;
		node->body = symbol;
	} else {
		symbol = node->body;
	}
	// make sure the force bits don't clash
	if ((node_created == FALSE) && force_bits)
		if ((symbol->result.flags & MVALUE_FORCEBITS) != force_bits)
			Throw_error("Too late for postfix.");
	return symbol;
}


// assign value to symbol. the function acts upon the symbol's flag bits and
// produces an error if needed.
void symbol_set_value(struct symbol *symbol, struct result *new_value, int change_allowed)
{
	int	oldflags	= symbol->result.flags;

	// value stuff
	if ((oldflags & MVALUE_DEFINED) && (change_allowed == FALSE)) {
		// symbol is already defined, so compare new and old values
		// if different type OR same type but different value, complain
		if (((oldflags ^ new_value->flags) & MVALUE_IS_FP)
		|| ((oldflags & MVALUE_IS_FP) ? (symbol->result.val.fpval != new_value->val.fpval) : (symbol->result.val.intval != new_value->val.intval)))
			Throw_error("Symbol already defined.");
	} else {
		// symbol is not defined yet OR redefinitions are allowed
		symbol->result = *new_value;
	}
	// flags stuff
	// Ensure that "unsure" symbols without "isByte" state don't get that
	if ((oldflags & (MVALUE_UNSURE | MVALUE_ISBYTE)) == MVALUE_UNSURE)
		new_value->flags &= ~MVALUE_ISBYTE;
	if (change_allowed) {
		oldflags = (oldflags & MVALUE_UNSURE) | new_value->flags;
	} else {
		if ((oldflags & MVALUE_FORCEBITS) == 0)
			if ((oldflags & (MVALUE_UNSURE | MVALUE_DEFINED)) == 0)
				oldflags |= new_value->flags & MVALUE_FORCEBITS;
		oldflags |= new_value->flags & ~MVALUE_FORCEBITS;
	}
	symbol->result.flags = oldflags;
}


// parse label definition (can be either global or local).
// name must be held in GlobalDynaBuf.
void symbol_set_label(scope_t scope, int stat_flags, int force_bit, int change_allowed)
{
	struct result	pc,
			result;
	struct symbol	*symbol;

	symbol = symbol_find(scope, force_bit);
	// label definition
	if ((stat_flags & SF_FOUND_BLANK) && config.warn_on_indented_labels)
		Throw_first_pass_warning("Label name not in leftmost column.");
	vcpu_read_pc(&pc);
	result.flags = pc.flags & MVALUE_DEFINED;
	result.val.intval = pc.val.intval;
	result.addr_refs = pc.addr_refs;
	symbol_set_value(symbol, &result, change_allowed);
	// global labels must open new scope for cheap locals
	if (scope == SCOPE_GLOBAL)
		section_new_cheap_scope(section_now);
}


// parse symbol definition (can be either global or local, may turn out to be a label).
// name must be held in GlobalDynaBuf.
void symbol_parse_definition(scope_t scope, int stat_flags)
{
	struct result	result;
	struct symbol	*symbol;
	int		force_bit	= Input_get_force_bit();	// skips spaces after
	// FIXME - force bit is allowed for label definitions?!

	if (GotByte == '=') {
		// explicit symbol definition (symbol = <something>)
		symbol = symbol_find(scope, force_bit);
		// symbol = parsed value
		GetByte();	// skip '='
		ALU_any_result(&result);
		// if wanted, mark as address reference
		if (typesystem_says_address())
			result.addr_refs = 1;
		symbol_set_value(symbol, &result, FALSE);
		Input_ensure_EOS();
	} else {
		symbol_set_label(scope, stat_flags, force_bit, FALSE);
	}
}


// set global symbol to value, no questions asked (for "-D" switch)
// Name must be held in GlobalDynaBuf.
void symbol_define(intval_t value)
{
	struct result	result;
	struct symbol	*symbol;

	result.flags = MVALUE_GIVEN;
	result.val.intval = value;
	symbol = symbol_find(SCOPE_GLOBAL, 0);
	symbol_set_value(symbol, &result, TRUE);
}


// dump global symbols to file
void symbols_list(FILE *fd)
{
	Tree_dump_forest(symbols_forest, SCOPE_GLOBAL, dump_one_symbol, fd);
}


void symbols_vicelabels(FILE *fd)
{
	// FIXME - if type checking is enabled, maybe only output addresses?
	// the order of dumped labels is important because VICE will prefer later defined labels
	// dump unused labels
	Tree_dump_forest(symbols_forest, SCOPE_GLOBAL, dump_vice_unusednonaddress, fd);
	fputc('\n', fd);
	// dump other used labels
	Tree_dump_forest(symbols_forest, SCOPE_GLOBAL, dump_vice_usednonaddress, fd);
	fputc('\n', fd);
	// dump address symbols
	Tree_dump_forest(symbols_forest, SCOPE_GLOBAL, dump_vice_address, fd);
}


// fix name of anonymous forward label (held in DynaBuf, NOT TERMINATED!) so it
// references the *next* anonymous forward label definition. The tricky bit is,
// each name length would need its own counter. But hey, ACME's real quick in
// finding symbols, so I'll just abuse the symbol system to store those counters.
void symbol_fix_forward_anon_name(int increment)
{
	struct symbol	*counter_symbol;
	unsigned long	number;

	// terminate name, find "counter" symbol and read value
	DynaBuf_append(GlobalDynaBuf, '\0');
	counter_symbol = symbol_find(section_now->local_scope, 0);
	// make sure it gets reset to zero in each new pass
	if (counter_symbol->pass != pass_count) {
		counter_symbol->pass = pass_count;
		counter_symbol->result.val.intval = 0;
	}
	number = (unsigned long) counter_symbol->result.val.intval;
	// now append to the name to make it unique
	GlobalDynaBuf->size--;	// forget terminator, we want to append
	do {
		DYNABUF_APPEND(GlobalDynaBuf, 'a' + (number & 15));
		number >>= 4;
	} while (number);
	DynaBuf_append(GlobalDynaBuf, '\0');
	if (increment)
		counter_symbol->result.val.intval++;
}
