// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// symbol stuff
//
// 22 Nov 2007	"warn on indented labels" is now a CLI switch
// 25 Sep 2011	Fixed bug in !sl (colons in filename could be interpreted as EOS)
#include <stdio.h>
#include "acme.h"
#include "alu.h"
#include "dynabuf.h"
#include "global.h"
#include "input.h"
#include "output.h"
#include "platform.h"
#include "section.h"
#include "symbol.h"
#include "tree.h"
#include "typesystem.h"


// constants
#define s_sl	(s_asl + 1)	// Yes, I know I'm sick


// variables
struct node_ra_t	*symbols_forest[256];	// ... (because of 8-bit hash)


// Dump symbol value and flags to dump file
static void dump_one_symbol(struct node_ra_t *node, FILE *fd)
{
	struct symbol	*symbol	= node->body;

	// output name
	if (warn_on_type_mismatch
	&& symbol->result.addr_refs == 1)
		fprintf(fd, "!addr\t");
	fprintf(fd, "%s", node->id_string);
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
		fprintf(fd, " ?");
	}
	if (symbol->result.flags & MVALUE_UNSURE)
		fprintf(fd, "\t; ?");
	if (symbol->usage == 0)
		fprintf(fd, "\t; unused");
	fprintf(fd, "\n");
}


// search for symbol. create if nonexistant. if created, give it flags "flags".
// the symbol name must be held in GlobalDynaBuf.
struct symbol *symbol_find(zone_t zone, int flags)
{
	struct node_ra_t	*node;
	struct symbol		*symbol;
	int			node_created,
				force_bits	= flags & MVALUE_FORCEBITS;

	node_created = Tree_hard_scan(&node, symbols_forest, zone, TRUE);
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
		|| ((oldflags & MVALUE_IS_FP)
		? (symbol->result.val.fpval != new_value->val.fpval)
		: (symbol->result.val.intval != new_value->val.intval)))
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


// (Re)set symbol
static enum eos PO_set(void)	// Now GotByte = illegal char
{
	struct result	result;
	int		force_bit;
	struct symbol	*symbol;
	zone_t		zone;

	if (Input_read_zone_and_keyword(&zone) == 0)	// skips spaces before
		// Now GotByte = illegal char
		return SKIP_REMAINDER;

	force_bit = Input_get_force_bit();	// skips spaces after
	symbol = symbol_find(zone, force_bit);
	if (GotByte != '=') {
		Throw_error(exception_syntax);
		return SKIP_REMAINDER;
	}

	// symbol = parsed value
	GetByte();	// proceed with next char
	ALU_any_result(&result);
	// clear symbol's force bits and set new ones
	symbol->result.flags &= ~(MVALUE_FORCEBITS | MVALUE_ISBYTE);
	if (force_bit) {
		symbol->result.flags |= force_bit;
		result.flags &= ~(MVALUE_FORCEBITS | MVALUE_ISBYTE);
	}
	symbol_set_value(symbol, &result, TRUE);
	return ENSURE_EOS;
}


// Select dump file
static enum eos PO_sl(void)
{
	// bugfix: first read filename, *then* check for first pass.
	// if skipping right away, quoted colons might be misinterpreted as EOS
	// FIXME - why not just fix the skipping code to handle quotes? :)
	// "!to" has been fixed as well

	// read filename to global dynamic buffer
	// if no file name given, exit (complaining will have been done)
	if (Input_read_filename(FALSE))
		return SKIP_REMAINDER;

	// only process this pseudo opcode in first pass
	if (pass_count)
		return SKIP_REMAINDER;

	// if symbol dump file already chosen, complain and exit
	if (symboldump_filename) {
		Throw_warning("Label dump file already chosen.");
		return SKIP_REMAINDER;
	}

	// get malloc'd copy of filename
	symboldump_filename = DynaBuf_get_copy(GlobalDynaBuf);
	// ensure there's no garbage at end of line
	return ENSURE_EOS;
}


// predefined stuff
static struct node_t	pseudo_opcodes[]	= {
	PREDEFNODE("set",	PO_set),
	PREDEFLAST(s_sl,	PO_sl),
	//    ^^^^ this marks the last element
};


// parse label definition (can be either global or local).
// name must be held in GlobalDynaBuf.
void symbol_set_label(zone_t zone, int stat_flags, int force_bit, int change_allowed)
{
	struct result	pc,
			result;
	struct symbol	*symbol;

	symbol = symbol_find(zone, force_bit);
	// label definition
	if ((stat_flags & SF_FOUND_BLANK) && warn_on_indented_labels)
		Throw_first_pass_warning("Label name not in leftmost column.");
	vcpu_read_pc(&pc);
	result.flags = pc.flags & MVALUE_DEFINED;
	result.val.intval = pc.val.intval;
	result.addr_refs = pc.addr_refs;
	symbol_set_value(symbol, &result, change_allowed);
}


// parse symbol definition (can be either global or local, may turn out to be a label).
// name must be held in GlobalDynaBuf.
void symbol_parse_definition(zone_t zone, int stat_flags)
{
	struct result	result;
	struct symbol	*symbol;
	int		force_bit	= Input_get_force_bit();	// skips spaces after
	// FIXME - force bit is allowed for label definitions?!

	if (GotByte == '=') {
		// explicit symbol definition (symbol = <something>)
		symbol = symbol_find(zone, force_bit);
		// symbol = parsed value
		GetByte();	// skip '='
		ALU_any_result(&result);
		// if wanted, mark as address reference
		if (typesystem_says_address())
			result.addr_refs = 1;
		symbol_set_value(symbol, &result, FALSE);
		Input_ensure_EOS();
	} else {
		symbol_set_label(zone, stat_flags, force_bit, FALSE);
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
	symbol = symbol_find(ZONE_GLOBAL, 0);
	symbol_set_value(symbol, &result, TRUE);
}


// dump global symbols to file
void symbols_dump_all(FILE *fd)
{
	Tree_dump_forest(symbols_forest, ZONE_GLOBAL, dump_one_symbol, fd);
	PLATFORM_SETFILETYPE_TEXT(symboldump_filename);
}


// clear symbols forest (is done early)
void symbols_clear_init(void)
{
	struct node_ra_t	**ptr;
	int			ii;

	// cut down all the trees (clear pointer table)
	ptr = symbols_forest;
	for (ii = 255; ii >= 0; ii--)
		*ptr++ = NULL;
}


// register pseudo opcodes (done later)
void symbols_register_init(void)
{
	Tree_add_table(&pseudo_opcode_tree, pseudo_opcodes);
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
	counter_symbol = symbol_find(Section_now->zone, 0);
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
