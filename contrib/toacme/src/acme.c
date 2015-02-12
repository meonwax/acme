// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// ACME syntax

#include "acme.h"
#include "io.h"


// constants

// pseudo opcodes
const char	ACME_po_to[]		= "!to";
const char	ACME_cbmformat[]	= ", cbm";
const char	ACME_po_sl[]		= "!sl";
const char	ACME_set_pc[]		= "*=";
const char	ACME_po_source[]	= "!src";
const char	ACME_po_byte[]		= "!byte";
const char	ACME_po_word[]		= "!word";
const char	ACME_po_fill[]		= "!fill";
const char	ACME_po_pet[]		= "!pet";
const char	ACME_po_scr[]		= "!scr";
const char	ACME_po_macro[]		= "!macro";
const char	ACME_endmacro[]		= "}; (end of macro definition)\n";
const char	ACME_macro_call[]	= "+";
const char	ACME_po_if[]		= "!if";
const char	ACME_else[]		= "} else {";
const char	ACME_endif[]		= "}; (end of conditional assembly)\n";
const char	ACME_po_eof[]		= "!eof";
// pseudo opcodes for 65816 (used by Flash8-AssBlaster)
const char	ACME_po_al[]		= "!al";
const char	ACME_po_as[]		= "!as";
const char	ACME_po_rl[]		= "!rl";
const char	ACME_po_rs[]		= "!rs";


// functions

// output pseudo opcode to make ACME use PetSCII encoding
void ACME_switch_to_pet(void)
{
	IO_put_string(
"; ToACME: Adding pseudo opcode to use PetSCII encoding by default:\n"
"!convtab pet\n"
	);
}
