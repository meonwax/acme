// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// ALU stuff (the expression parser)
#ifndef alu_H
#define alu_H


#include "config.h"


// constants

// meaning of bits in "flags" of struct result:	TODO - this is only for future "number" result type!
#define MVALUE_IS_FP	(1u << 8)	// floating point value
#define MVALUE_INDIRECT	(1u << 7)	// needless parentheses indicate use of indirect addressing modes
#define MVALUE_EXISTS	(1u << 6)	// 0: expression was empty. 1: there was *something* to parse.	TODO - get rid of this, make "nothing" its own result type instead!
#define MVALUE_UNSURE	(1u << 5)	// value once was related to undefined
// expression. Needed for producing the same addresses in all passes; because in
// the first pass there will almost for sure be labels that are undefined, you
// can't simply get the addressing mode from looking at the parameter's value.
#define MVALUE_DEFINED	(1u << 4)	// 0: undefined expression (value will be zero). 1: known result
#define MVALUE_ISBYTE	(1u << 3)	// value is guaranteed to fit in one byte
#define MVALUE_FORCE24	(1u << 2)	// value usage forces 24-bit usage
#define MVALUE_FORCE16	(1u << 1)	// value usage forces 16-bit usage
#define MVALUE_FORCE08	(1u << 0)	// value usage forces 8-bit usage
#define MVALUE_FORCEBITS	(MVALUE_FORCE08|MVALUE_FORCE16|MVALUE_FORCE24)
#define MVALUE_GIVEN	(MVALUE_DEFINED | MVALUE_EXISTS)	// bit mask for fixed values (defined and existing)


// create dynamic buffer, operator/function trees and operator/operand stacks
extern void ALU_init(void);
// function pointer for "value undefined" error output.
// set to NULL to suppress those errors,
// set to Throw_error to show them.
extern void (*ALU_optional_notdef_handler)(const char *);


// FIXME - replace all the functions below with a single one using a "flags" arg!
/* its return value would then be:
enum expression_result {
	EXRE_ERROR,	// error (has been reported, so skip remainder of statement)
	EXRE_NOTHING,	// next char after space was comma or end-of-statement
	EXRE_NUMBER,	// int or float (what is returned by the current functions)
	EXRE_STRING,	// TODO
	EXRE_RESERVED,	// reserved cpu constant (register names), TODO
	EXRE_LIST	// TODO
};
// input flags:
#define ACCEPT_EMPTY		(1u << 0)	// if not given, throws error
#define ACCEPT_UNDEFINED	(1u << 1)	// if not given, undefined throws serious error
//#define ACCEPT_INT		(1u << )	needed when strings come along!
#define ACCEPT_FLOAT		(1u << 2)	// if not given, floats are converted to integer
#define ACCEPT_OPENPARENTHESIS	(1u << 3)	// if not given, throws syntax error
//#define ACCEPT_STRING
// do I need ACCEPT_INT and/or ACCEPT_ADDRESS?
*/

// stores int value if given. Returns whether stored. Throws error if undefined.
extern int ALU_optional_defined_int(intval_t *target);
// returns int value (0 if result was undefined)
extern intval_t ALU_any_int(void);
// stores int value and flags (floats are transformed to int)
extern void ALU_int_result(struct result *intresult);
// stores int value and flags (floats are transformed to int)
// if result was undefined, serious error is thrown
extern void ALU_defined_int(struct result *intresult);
// stores int value and flags, allowing for one '(' too many (x-indirect addr).
// returns number of additional '(' (1 or 0).
extern int ALU_liberal_int(struct result *intresult);
// stores value and flags (result may be either int or float)
extern void ALU_any_result(struct result *result);


#endif
