// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// Flow control stuff (loops, conditional assembly etc.)
//
// Macros, conditional assembly, loops and sourcefile-includes are all based on
// parsing blocks of code. When defining macros or using loops or conditional
// assembly, the block starts with "{" and ends with "}". In the case of
// "!source", the given file is treated like a block - the outermost assembler
// function uses the same technique to parse the top level file.
//
// 24 Nov 2007	Added "!ifndef"
#include <string.h>
#include "acme.h"
#include "alu.h"
#include "config.h"
#include "dynabuf.h"
#include "global.h"	// FIXME - remove when no longer needed
#include "input.h"
#include "mnemo.h"
#include "pseudoopcodes.h"	// FIXME - remove when no longer needed
#include "symbol.h"
#include "tree.h"


// type definitions

struct loop_condition {
	int	line;	// original line number
	int	is_until;	// actually bool (0 for WHILE, 1 for UNTIL)
	char	*body;	// pointer to actual expression
};


// helper functions for "!for" and "!do"

// parse a loop body (could also be used for macro body)
static void parse_ram_block(int line_number, char *body)
{
	Input_now->line_number = line_number;	// set line number to loop start
	Input_now->src.ram_ptr = body;	// set RAM read pointer to loop
	// parse loop body
	Parse_until_eob_or_eof();
	if (GotByte != CHAR_EOB)
		Bug_found("IllegalBlockTerminator", GotByte);
}


// try to read a condition into DynaBuf and store copy pointer in
// given loop_condition structure.
// if no condition given, NULL is written to structure.
// call with GotByte = first interesting character
static void store_condition(struct loop_condition *condition, char terminator)
{
	// write line number
	condition->line = Input_now->line_number;
	// set defaults
	condition->is_until = FALSE;
	condition->body = NULL;
	// check for empty condition
	if (GotByte == terminator)
		return;

	// seems as if there really *is* a condition, so check for until/while
	if (Input_read_and_lower_keyword()) {
		if (strcmp(GlobalDynaBuf->buffer, "while") == 0) {
			//condition.is_until = FALSE;
		} else if (strcmp(GlobalDynaBuf->buffer, "until") == 0) {
			condition->is_until = TRUE;
		} else {
			Throw_error(exception_syntax);
			return;
		}
		// write given condition into buffer
		SKIPSPACE();
		DYNABUF_CLEAR(GlobalDynaBuf);
		Input_until_terminator(terminator);
		DynaBuf_append(GlobalDynaBuf, CHAR_EOS);	// ensure terminator
		condition->body = DynaBuf_get_copy(GlobalDynaBuf);
	}
}


// check a condition expression
static int check_condition(struct loop_condition *condition)
{
	intval_t	expression;

	// first, check whether there actually *is* a condition
	if (condition->body == NULL)
		return TRUE;	// non-existing conditions are always true

	// set up input for expression evaluation
	Input_now->line_number = condition->line;
	Input_now->src.ram_ptr = condition->body;
	GetByte();	// proceed with next char
	expression = ALU_defined_int();
	if (GotByte)
		Throw_serious_error(exception_syntax);
	return condition->is_until ? !expression : !!expression;
}


struct loop_total {
	struct loop_condition	head_cond;
	int			start;	// line number of loop pseudo opcode
	char			*body;
	struct loop_condition	tail_cond;
};

// back end function for "!do" pseudo opcode
static void do_loop(struct loop_total *loop)
{
	struct input	loop_input;
	struct input	*outer_input;
	int		go_on;

	// set up new input
	loop_input = *Input_now;	// copy current input structure into new
	loop_input.source_is_ram = TRUE;	// set new byte source
	// remember old input
	outer_input = Input_now;
	// activate new input (not useable yet, as pointer and
	// line number are not yet set up)
	Input_now = &loop_input;
	do {
		// check head condition
		go_on = check_condition(&loop->head_cond);
		if (go_on) {
			parse_ram_block(loop->start, loop->body);
			// check tail condition
			go_on = check_condition(&loop->tail_cond);
		}
	} while (go_on);
	// free memory
	free(loop->head_cond.body);
	free(loop->body);
	free(loop->tail_cond.body);
	// restore previous input:
	Input_now = outer_input;
	GotByte = CHAR_EOS;	// CAUTION! Very ugly kluge.
	// But by switching input, we lost the outer input's GotByte. We know
	// it was CHAR_EOS. We could just call GetByte() to get real input, but
	// then the main loop could choke on unexpected bytes. So we pretend
	// that we got the outer input's GotByte value magically back.
}


// move to pseudoopcodes.c:

// looping assembly ("!do"). has to be re-entrant.
static enum eos po_do(void)	// now GotByte = illegal char
{
	struct loop_total	loop;

	// init
	loop.head_cond.is_until = FALSE;
	loop.head_cond.body = NULL;
	loop.tail_cond.is_until = FALSE;
	loop.tail_cond.body = NULL;
	
	// read head condition to buffer
	SKIPSPACE();
	store_condition(&loop.head_cond, CHAR_SOB);
	if (GotByte != CHAR_SOB)
		Throw_serious_error(exception_no_left_brace);
	// remember line number of loop body,
	// then read block and get copy
	loop.start = Input_now->line_number;
	loop.body = Input_skip_or_store_block(TRUE);	// changes line number!
	// now GotByte = '}'
	NEXTANDSKIPSPACE();	// now GotByte = first non-blank char after block
	// read tail condition to buffer
	store_condition(&loop.tail_cond, CHAR_EOS);
	// now GotByte = CHAR_EOS
	do_loop(&loop);
	return AT_EOS_ANYWAY;
}


// looping assembly ("!for"). has to be re-entrant.
// old syntax: !for VAR, END { BLOCK }		VAR counts from 1 to END
// new syntax: !for VAR, START, END { BLOCK }	VAR counts from START to END
static enum eos po_for(void)	// now GotByte = illegal char
{
	struct input	loop_input,
			*outer_input;
	struct result	loop_counter;
	intval_t	first_arg,
			counter_first,
			counter_last,
			counter_increment;
	int		old_algo;	// actually bool
	char		*loop_body;	// pointer to loop's body block
	struct symbol	*symbol;
	zone_t		zone;
	int		force_bit,
			loop_start;	// line number of "!for" pseudo opcode

	if (Input_read_zone_and_keyword(&zone) == 0)	// skips spaces before
		return SKIP_REMAINDER;

	// now GotByte = illegal char
	force_bit = Input_get_force_bit();	// skips spaces after
	symbol = symbol_find(zone, force_bit);
	if (!Input_accept_comma()) {
		Throw_error(exception_syntax);
		return SKIP_REMAINDER;
	}

	first_arg = ALU_defined_int();
	if (Input_accept_comma()) {
		old_algo = FALSE;	// new format - yay!
		if (!warn_on_old_for)
			Throw_first_pass_warning("Found new \"!for\" syntax.");
		counter_first = first_arg;	// use given argument
		counter_last = ALU_defined_int();	// read second argument
		counter_increment = (counter_last < counter_first) ? -1 : 1;
	} else {
		old_algo = TRUE;	// old format - booo!
		if (warn_on_old_for)
			Throw_first_pass_warning("Found old \"!for\" syntax.");
		if (first_arg < 0)
			Throw_serious_error("Loop count is negative.");
		counter_first = 0;	// CAUTION - old algo pre-increments and therefore starts with 1!
		counter_last = first_arg;	// use given argument
		counter_increment = 1;
	}
	if (GotByte != CHAR_SOB)
		Throw_serious_error(exception_no_left_brace);
	// remember line number of loop pseudo opcode
	loop_start = Input_now->line_number;
	// read loop body into DynaBuf and get copy
	loop_body = Input_skip_or_store_block(TRUE);	// changes line number!
	// switching input makes us lose GotByte. But we know it's '}' anyway!
	// set up new input
	loop_input = *Input_now;	// copy current input structure into new
	loop_input.source_is_ram = TRUE;	// set new byte source
	// remember old input
	outer_input = Input_now;
	// activate new input
	// (not yet useable; pointer and line number are still missing)
	Input_now = &loop_input;
	// init counter
	loop_counter.flags = MVALUE_DEFINED | MVALUE_EXISTS;
	loop_counter.val.intval = counter_first;
	symbol_set_value(symbol, &loop_counter, TRUE);
	if (old_algo) {
		// old algo for old syntax:
		// if count == 0, skip loop
		if (counter_last) {
			do {
				loop_counter.val.intval += counter_increment;
				symbol_set_value(symbol, &loop_counter, TRUE);
				parse_ram_block(loop_start, loop_body);
			} while (loop_counter.val.intval < counter_last);
		}
	} else {
		// new algo for new syntax:
		do {
			parse_ram_block(loop_start, loop_body);
			loop_counter.val.intval += counter_increment;
			symbol_set_value(symbol, &loop_counter, TRUE);
		} while (loop_counter.val.intval != (counter_last + counter_increment));
	}
	// free memory
	free(loop_body);
	// restore previous input:
	Input_now = outer_input;
	// GotByte of OuterInput would be '}' (if it would still exist)
	GetByte();	// fetch next byte
	return ENSURE_EOS;
}


// helper functions for "!if", "!ifdef" and "!ifndef"

// parse or skip a block. Returns whether block's '}' terminator was missing.
// afterwards: GotByte = '}'
static int skip_or_parse_block(int parse)
{
	if (!parse) {
		Input_skip_or_store_block(FALSE);
		return 0;
	}
	// if block was correctly terminated, return FALSE
	Parse_until_eob_or_eof();
	// if block isn't correctly terminated, complain and exit
	if (GotByte != CHAR_EOB)
		Throw_serious_error(exception_no_right_brace);
	return 0;
}


// parse {block} [else {block}]
void flow_parse_block_else_block(int parse_first)
{
	// Parse first block.
	// If it's not correctly terminated, return immediately (because
	// in that case, there's no use in checking for an "else" part).
	if (skip_or_parse_block(parse_first))
		return;

	// now GotByte = '}'. Check for "else" part.
	// If end of statement, return immediately.
	NEXTANDSKIPSPACE();
	if (GotByte == CHAR_EOS)
		return;

	// read keyword and check whether really "else"
	if (Input_read_and_lower_keyword()) {
		if (strcmp(GlobalDynaBuf->buffer, "else")) {
			Throw_error(exception_syntax);
		} else {
			SKIPSPACE();
			if (GotByte != CHAR_SOB)
				Throw_serious_error(exception_no_left_brace);
			skip_or_parse_block(!parse_first);
			// now GotByte = '}'
			GetByte();
		}
	}
	Input_ensure_EOS();
}


// parse a whole source code file
void Parse_and_close_file(FILE *fd, const char *filename)
{
	// be verbose
	if (Process_verbosity > 2)
		printf("Parsing source file '%s'\n", filename);
	// set up new input
	Input_new_file(filename, fd);
	// Parse block and check end reason
	Parse_until_eob_or_eof();
	if (GotByte != CHAR_EOF)
		Throw_error("Found '}' instead of end-of-file.");
	// close sublevel src
	fclose(Input_now->src.fd);
}


// pseudo opcode table
static struct ronode	pseudo_opcodes[]	= {
	PREDEFNODE("do",	po_do),
	PREDEFLAST("for",	po_for),
	//    ^^^^ this marks the last element
};


// register pseudo opcodes
void Flow_init(void)
{
	Tree_add_table(&pseudo_opcode_tree, pseudo_opcodes);
}
