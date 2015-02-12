// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// ACME syntax
//
#ifndef acme_H
#define acme_H


// Constants
//

// pseudo opcodes
extern const char	PseudoOp_Byte[];
extern const char	PseudoOp_Else[];
extern const char	PseudoOp_EndIf[];
extern const char	PseudoOp_EndMacroDef[];
extern const char	PseudoOp_EOF[];
extern const char	PseudoOp_Fill[];
extern const char	PseudoOp_If[];
extern const char	PseudoOp_LabelDump[];
extern const char	PseudoOp_MacroCall[];
extern const char	PseudoOp_MacroDef[];
extern const char	PseudoOp_PetTxt[];
extern const char	PseudoOp_ScrTxt[];
extern const char	PseudoOp_SetPC[];
extern const char	PseudoOp_Source[];
extern const char	PseudoOp_ToFile[];
extern const char	PseudoTrail_ToFile[];
extern const char	PseudoOp_Word[];
// pseudo opcodes for 65816 cpu
extern const char	PseudoOp_al[];
extern const char	PseudoOp_as[];
extern const char	PseudoOp_rl[];
extern const char	PseudoOp_rs[];


// Prototypes
//
extern void	acme_SwitchToPet(void);


#endif
