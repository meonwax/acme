// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// ACME syntax
#ifndef acme_H
#define acme_H


// constants

// pseudo opcodes and related keywords
extern const char	ACME_po_to[];
extern const char	ACME_cbmformat[];
extern const char	ACME_po_sl[];
extern const char	ACME_set_pc[];
extern const char	ACME_po_source[];
extern const char	ACME_po_byte[];
extern const char	ACME_po_word[];
extern const char	ACME_po_fill[];
extern const char	ACME_po_pet[];
extern const char	ACME_po_scr[];
extern const char	ACME_po_macro[];
extern const char	ACME_endmacro[];
extern const char	ACME_macro_call[];
extern const char	ACME_po_if[];
extern const char	ACME_else[];
extern const char	ACME_endif[];
extern const char	ACME_po_eof[];
// pseudo opcodes for 65816 cpu
extern const char	ACME_po_al[];
extern const char	ACME_po_as[];
extern const char	ACME_po_rl[];
extern const char	ACME_po_rs[];


// prototypes
extern void ACME_switch_to_pet(void);


#endif
