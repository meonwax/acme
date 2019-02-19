// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2017 Marco Baye
// Have a look at "acme.c" for further info
//
// Macro stuff
#include "macro.h"
#include <string.h>	// needs strlen() + memcpy()
#include "config.h"
#include "platform.h"
#include "acme.h"
#include "alu.h"
#include "dynabuf.h"
#include "global.h"
#include "input.h"
#include "section.h"
#include "symbol.h"
#include "tree.h"


// Constants
#define MACRONAME_DYNABUF_INITIALSIZE	128
#define ARG_SEPARATOR	' '	// separates macro title from arg types
#define ARGTYPE_NUM_VAL	'v'
#define ARGTYPE_NUM_REF	'V'
//#define ARGTYPE_STR_VAL	's'
//#define ARGTYPE_STR_REF	'S'
#define REFERENCE_CHAR	'~'	// prefix for call-by-reference
#define HALF_INITIAL_ARG_TABLE_SIZE	4
static const char	exception_macro_twice[]	= "Macro already defined.";


// macro struct type definition
struct macro {
	int	def_line_number;	// line number of definition	for error msgs
	char	*def_filename,	// file name of definition	for error msgs
		*original_name,	// user-supplied name		for error msgs
		*parameter_list,	// parameters (whole line)
		*body;	// RAM block containing macro body
};
// there's no need to make this a struct and add a type component:
// when the macro has been found, accessing its parameter_list component
// gives us the possibility to find out which args are call-by-value and
// which ones are call-by-reference.
union macro_arg_t {
	struct result	result;	// value and flags (call by value)
	struct symbol	*symbol;	// pointer to symbol struct (call by reference)
};


// Variables
static struct dynabuf	*user_macro_name;	// original macro title
static struct dynabuf	*internal_name;		// plus param type chars
static struct rwnode	*macro_forest[256];	// trees (because of 8b hash)
// Dynamic argument table
static union macro_arg_t	*arg_table	= NULL;
static int			argtable_size	= HALF_INITIAL_ARG_TABLE_SIZE;


// Functions

// Enlarge the argument table
static void enlarge_arg_table(void)
{
	argtable_size *= 2;
	arg_table = realloc(arg_table, argtable_size * sizeof(*arg_table));
	if (arg_table == NULL)
		Throw_serious_error(exception_no_memory_left);
}

// create dynamic buffers and arg table
void Macro_init(void)
{
	user_macro_name = DynaBuf_create(MACRONAME_DYNABUF_INITIALSIZE);
	internal_name = DynaBuf_create(MACRONAME_DYNABUF_INITIALSIZE);
	enlarge_arg_table();
}

// Read macro scope and title. Title is read to GlobalDynaBuf and then copied
// over to internal_name DynaBuf, where ARG_SEPARATOR is added.
// In user_macro_name DynaBuf, the original name is reconstructed (even with
// LOCAL_PREFIX) so a copy can be linked to the resulting macro struct.
static scope_t get_scope_and_title(void)
{
	scope_t	macro_scope;

	Input_read_scope_and_keyword(&macro_scope);	// skips spaces before
	// now GotByte = illegal character after title
	// copy macro title to private dynabuf and add separator character
	DYNABUF_CLEAR(user_macro_name);
	DYNABUF_CLEAR(internal_name);
	if (macro_scope != SCOPE_GLOBAL) {
		// TODO - allow "cheap macros"?!
		DynaBuf_append(user_macro_name, LOCAL_PREFIX);
	}
	DynaBuf_add_string(user_macro_name, GLOBALDYNABUF_CURRENT);
	DynaBuf_add_string(internal_name, GLOBALDYNABUF_CURRENT);
	DynaBuf_append(user_macro_name, '\0');
	DynaBuf_append(internal_name, ARG_SEPARATOR);
	SKIPSPACE();	// done here once so it's not necessary at two callers
	return macro_scope;
}

// Check for comma. If there, append to GlobalDynaBuf.
static int pipe_comma(void)
{
	int	result;

	result = Input_accept_comma();
	if (result)
		DYNABUF_APPEND(GlobalDynaBuf, ',');
	return result;
}

// Return malloc'd copy of string
static char *get_string_copy(const char *original)
{
	size_t	size;
	char	*copy;

	size = strlen(original) + 1;
	copy = safe_malloc(size);
	memcpy(copy, original, size);
	return copy;
}

// This function is called from both macro definition and macro call.
// Terminate macro name and copy from internal_name to GlobalDynaBuf
// (because that's where Tree_hard_scan() looks for the search string).
// Then try to find macro and return whether it was created.
static int search_for_macro(struct rwnode **result, scope_t scope, int create)
{
	DynaBuf_append(internal_name, '\0');	// terminate macro name
	// now internal_name = macro_title SPC argument_specifiers NUL
	DYNABUF_CLEAR(GlobalDynaBuf);
	DynaBuf_add_string(GlobalDynaBuf, internal_name->buffer);
	DynaBuf_append(GlobalDynaBuf, '\0');
	return Tree_hard_scan(result, macro_forest, scope, create);
}

// This function is called when an already existing macro is re-defined.
// It first outputs a warning and then a serious error, stopping assembly.
// Showing the first message as a warning guarantees that ACME does not reach
// the maximum error limit inbetween.
static void report_redefinition(struct rwnode *macro_node)
{
	struct macro	*original_macro	= macro_node->body;

	// show warning with location of current definition
	Throw_warning(exception_macro_twice);
	// CAUTION, ugly kluge: fiddle with Input_now and section_now
	// data to generate helpful error messages
	Input_now->original_filename = original_macro->def_filename;
	Input_now->line_number = original_macro->def_line_number;
	section_now->type = "original";
	section_now->title = "definition";
	// show serious error with location of original definition
	Throw_serious_error(exception_macro_twice);
}

// This function is only called during the first pass, so there's no need to
// check whether to skip the definition or not.
// Return with GotByte = '}'
void Macro_parse_definition(void)	// Now GotByte = illegal char after "!macro"
{
	char		*formal_parameters;
	struct rwnode	*macro_node;
	struct macro	*new_macro;
	scope_t		macro_scope	= get_scope_and_title();

	// now GotByte = first non-space after title
	DYNABUF_CLEAR(GlobalDynaBuf);	// prepare to hold formal parameters
	// GlobalDynaBuf = "" (will hold formal parameter list)
	// user_macro_name = [LOCAL_PREFIX] MacroTitle NUL
	// internal_name = MacroTitle ARG_SEPARATOR (grows to signature)
	// Accept n>=0 comma-separated formal parameters before CHAR_SOB ('{').
	// Valid argument formats are:
	// .LOCAL_LABEL_BY_VALUE
	// ~.LOCAL_LABEL_BY_REFERENCE
	// @CHEAP_LOCAL_LABEL_BY_VALUE
	// ~@CHEAP_LOCAL_LABEL_BY_REFERENCE
	// GLOBAL_LABEL_BY_VALUE	global args are very uncommon,
	// ~GLOBAL_LABEL_BY_REFERENCE	but not forbidden
	// now GotByte = non-space
	if (GotByte != CHAR_SOB) {	// any at all?
		do {
			// handle call-by-reference character ('~')
			if (GotByte != REFERENCE_CHAR) {
				DynaBuf_append(internal_name, ARGTYPE_NUM_VAL);
			} else {
				DynaBuf_append(internal_name, ARGTYPE_NUM_REF);
				DynaBuf_append(GlobalDynaBuf, REFERENCE_CHAR);
				GetByte();
			}
			// handle prefix for (cheap) local symbols ('.'/'@')
			if ((GotByte == LOCAL_PREFIX)
			|| (GotByte == CHEAP_PREFIX)) {
				DynaBuf_append(GlobalDynaBuf, GotByte);
				GetByte();
			}
			// handle symbol name
			Input_append_keyword_to_global_dynabuf();
		} while (pipe_comma());
		// ensure CHAR_SOB ('{')
		if (GotByte != CHAR_SOB)
			Throw_serious_error(exception_no_left_brace);
	}
	DynaBuf_append(GlobalDynaBuf, CHAR_EOS);	// terminate param list
	// now GlobalDynaBuf = comma-separated parameter list without spaces,
	// but terminated with CHAR_EOS.
	formal_parameters = DynaBuf_get_copy(GlobalDynaBuf);
	// now GlobalDynaBuf = unused
	// Reading the macro body would change the line number. To have correct
	// error messages, we're checking for "macro twice" *now*.
	// Search for macro. Create if not found.
	// But if found, complain (macro twice).
	if (search_for_macro(&macro_node, macro_scope, TRUE) == FALSE)
		report_redefinition(macro_node);	// quits with serious error
	// Create new macro struct and set it up. Finally we'll read the body.
	new_macro = safe_malloc(sizeof(*new_macro));
	new_macro->def_line_number = Input_now->line_number;
	new_macro->def_filename = get_string_copy(Input_now->original_filename);
	new_macro->original_name = get_string_copy(user_macro_name->buffer);
	new_macro->parameter_list = formal_parameters;
	new_macro->body = Input_skip_or_store_block(TRUE);	// changes LineNumber
	macro_node->body = new_macro;	// link macro struct to tree node
	// and that about sums it up
}

// Parse macro call ("+MACROTITLE"). Has to be re-entrant.
void Macro_parse_call(void)	// Now GotByte = dot or first char of macro name
{
	char		local_gotbyte;
	struct symbol	*symbol;
	struct section	new_section,
			*outer_section;
	struct input	new_input,
			*outer_input;
	struct macro	*actual_macro;
	struct rwnode	*macro_node,
			*symbol_node;
	scope_t		macro_scope,
			symbol_scope;
	int		arg_count	= 0;
	int		outer_err_count;

	// Enter deeper nesting level
	// Quit program if recursion too deep.
	if (--macro_recursions_left < 0)
		Throw_serious_error("Too deeply nested. Recursive macro calls?");
	macro_scope = get_scope_and_title();
	// now GotByte = first non-space after title
	// internal_name = MacroTitle ARG_SEPARATOR (grows to signature)
	// Accept n>=0 comma-separated arguments before CHAR_EOS.
	// Valid argument formats are:
	// EXPRESSION (everything that does NOT start with '~'
	// ~.LOCAL_LABEL_BY_REFERENCE
	// ~GLOBAL_LABEL_BY_REFERENCE
	// now GotByte = non-space
	if (GotByte != CHAR_EOS) {	// any at all?
		do {
			// if arg table cannot take another element, enlarge
			if (argtable_size <= arg_count)
				enlarge_arg_table();
			// Decide whether call-by-reference or call-by-value
			// In both cases, GlobalDynaBuf may be used.
			if (GotByte == REFERENCE_CHAR) {
				// read call-by-reference arg
				DynaBuf_append(internal_name, ARGTYPE_NUM_REF);
				GetByte();	// skip '~' character
				Input_read_scope_and_keyword(&symbol_scope);
				// GotByte = illegal char
				arg_table[arg_count].symbol = symbol_find(symbol_scope, 0);
			} else {
				// read call-by-value arg
				DynaBuf_append(internal_name, ARGTYPE_NUM_VAL);
				ALU_any_result(&(arg_table[arg_count].result));
			}
			++arg_count;
		} while (Input_accept_comma());
	}
	// now arg_table contains the arguments
	// now GlobalDynaBuf = unused
	// check for "unknown macro"
	// Search for macro. Do not create if not found.
	search_for_macro(&macro_node, macro_scope, FALSE);
	if (macro_node == NULL) {
		Throw_error("Macro not defined (or wrong signature).");
		Input_skip_remainder();
	} else {
		// make macro_node point to the macro struct
		actual_macro = macro_node->body;
		local_gotbyte = GotByte;	// CAUTION - ugly kluge

		// set up new input
		new_input.original_filename = actual_macro->def_filename;
		new_input.line_number = actual_macro->def_line_number;
		new_input.source_is_ram = TRUE;
		new_input.state = INPUTSTATE_NORMAL;	// FIXME - fix others!
		new_input.src.ram_ptr = actual_macro->parameter_list;
		// remember old input
		outer_input = Input_now;
		// activate new input
		Input_now = &new_input;

		outer_err_count = Throw_get_counter();	// remember error count (for call stack decision)

		// remember old section
		outer_section = section_now;
		// start new section (with new scope)
		// FALSE = title mustn't be freed
		section_new(&new_section, "Macro", actual_macro->original_name, FALSE);
		section_new_cheap_scope(&new_section);
		GetByte();	// fetch first byte of parameter list
		// assign arguments
		if (GotByte != CHAR_EOS) {	// any at all?
			arg_count = 0;
			do {
				// Decide whether call-by-reference
				// or call-by-value
				// In both cases, GlobalDynaBuf may be used.
				if (GotByte == REFERENCE_CHAR) {
					// assign call-by-reference arg
					GetByte();	// skip '~' character
					Input_read_scope_and_keyword(&symbol_scope);
					if ((Tree_hard_scan(&symbol_node, symbols_forest, symbol_scope, TRUE) == FALSE)
					&& (pass_count == 0))
						Throw_error("Macro parameter twice.");
					symbol_node->body = arg_table[arg_count].symbol;
				} else {
					// assign call-by-value arg
					Input_read_scope_and_keyword(&symbol_scope);
					symbol = symbol_find(symbol_scope, 0);
// FIXME - add a possibility to symbol_find to make it possible to find out
// whether symbol was just created. Then check for the same error message here
// as above ("Macro parameter twice.").
					symbol->result = arg_table[arg_count].result;
				}
				++arg_count;
			} while (Input_accept_comma());
		}
		// and now, finally, parse the actual macro body
		Input_now->state = INPUTSTATE_NORMAL;	// FIXME - fix others!
// maybe call parse_ram_block(actual_macro->def_line_number, actual_macro->body)
		Input_now->src.ram_ptr = actual_macro->body;
		Parse_until_eob_or_eof();
		if (GotByte != CHAR_EOB)
			Bug_found("IllegalBlockTerminator", GotByte);
		// end section (free title memory, if needed)
		section_finalize(&new_section);
		// restore previous section
		section_now = outer_section;
		// restore previous input:
		Input_now = outer_input;
		// restore old Gotbyte context
		GotByte = local_gotbyte;	// CAUTION - ugly kluge

		// if needed, output call stack
		if (Throw_get_counter() != outer_err_count)
			Throw_warning("...called from here.");

		Input_ensure_EOS();
	}
	++macro_recursions_left;	// leave this nesting level
}
