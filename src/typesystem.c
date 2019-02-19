// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2017 Marco Baye
// Have a look at "acme.c" for further info
//
// type system stuff
#include "typesystem.h"
#include "config.h"
#include "alu.h"
#include "global.h"


static int	in_address_block	= FALSE;
static int	in_address_statement	= FALSE;

// Functions

int typesystem_says_address(void)
{
	return in_address_block | in_address_statement;
}

void typesystem_force_address_block(void)
{
	int	buffer	= in_address_block;

	in_address_block = TRUE;
	Parse_optional_block();
	in_address_block = buffer;
}

void typesystem_force_address_statement(int value)
{
	in_address_statement = value;
}

void typesystem_want_imm(struct result *result)
{
	if (!config.warn_on_type_mismatch)
		return;
	if (!(result->flags & MVALUE_DEFINED))
		return;
	if (result->addr_refs != 0) {
		Throw_warning("Wrong type - expected integer.");
		//printf("refcount should be 0, but is %d\n", result->addr_refs);
	}
}
void typesystem_want_addr(struct result *result)
{
	if (!config.warn_on_type_mismatch)
		return;
	if (!(result->flags & MVALUE_DEFINED))
		return;
	if (result->addr_refs != 1) {
		Throw_warning("Wrong type - expected address.");
		//printf("refcount should be 1, but is %d\n", result->addr_refs);
	}
}
