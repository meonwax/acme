// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// symbol stuff
#ifndef symbol_H
#define symbol_H


#include <stdio.h>
#include "config.h"


struct symbol {
	struct result	result;	// expression flags and value
	int		usage;	// usage count
	int		pass;	// pass of creation (for anon counters)
};


// variables
extern struct rwnode	*symbols_forest[];	// trees (because of 8-bit hash)


// clear symbol forest (is done early)
extern void symbols_clear_init(void);
// register pseudo opcodes (done later)
extern void symbols_register_init(void);
// function acts upon the symbol's flag bits and produces an error if needed.
extern void symbol_set_value(struct symbol *symbol, struct result *new_value, int change_allowed);
// parse label definition (can be either global or local).
// name must be held in GlobalDynaBuf.
extern void symbol_set_label(zone_t zone, int stat_flags, int force_bit, int change_allowed);
// parse symbol definition (can be either global or local, may turn out to be a label).
// name must be held in GlobalDynaBuf.
extern void symbol_parse_definition(zone_t zone, int stat_flags);
// search for symbol. create if nonexistant. if created, assign flags.
// name must be held in GlobalDynaBuf.
extern struct symbol *symbol_find(zone_t, int flags);
// set global symbol to value, no questions asked (for "-D" switch)
// name must be held in GlobalDynaBuf.
extern void symbol_define(intval_t value);
// dump global symbols to file
extern void symbols_list(FILE *fd);
// dump global labels to file in VICE format
extern void symbols_vicelabels(FILE *fd);
// fix name of anonymous forward label (held in GlobalDynaBuf, NOT TERMINATED!)
// so it references the *next* anonymous forward label definition.
extern void symbol_fix_forward_anon_name(int increment);


#endif
