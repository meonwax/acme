// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// ACME syntax
//

// Includes
//
#include "acme.h"
#include "io.h"


// Constants
//

// Pseudo opcodes
const char	PseudoOp_Byte[]		= "!byte";
const char	PseudoOp_Else[]		= "} else {";
const char	PseudoOp_EndIf[]	= "}; (end of conditional assembly)\n";
const char	PseudoOp_EndMacroDef[]	= "}; (end of macro definition)\n";
const char	PseudoOp_EOF[]		= "!eof";
const char	PseudoOp_Fill[]		= "!fill";
const char	PseudoOp_If[]		= "!if";
const char	PseudoOp_LabelDump[]	= "!sl";
const char	PseudoOp_MacroCall[]	= "+";
const char	PseudoOp_MacroDef[]	= "!macro";
const char	PseudoOp_PetTxt[]	= "!pet";
const char	PseudoOp_ScrTxt[]	= "!scr";
const char	PseudoOp_SetPC[]	= "*=";
const char	PseudoOp_Source[]	= "!src";
const char	PseudoOp_ToFile[]	= "!to";
const char	PseudoOp_Word[]		= "!word";
// Pseudo opcodes for 65816 (used by F8-AssBlaster)
const char	PseudoOp_al[]		= "!al";
const char	PseudoOp_as[]		= "!as";
const char	PseudoOp_rl[]		= "!rl";
const char	PseudoOp_rs[]		= "!rs";


// Functions
//


// Output pseudo opcode to make ACME use PetSCII encoding
//
void acme_SwitchToPet(void) {

	fputs(
"; ToACME: Adding pseudo opcode to use PetSCII encoding by default:\n"
"!convtab pet\n"
	, global_output_stream);
}
