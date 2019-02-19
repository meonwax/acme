// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2019 Marco Baye
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
#include "flow.h"
#include <string.h>
#include "acme.h"
#include "alu.h"
#include "config.h"
#include "dynabuf.h"
#include "global.h"	// FIXME - remove when no longer needed
#include "input.h"
#include "mnemo.h"
#include "symbol.h"
#include "tree.h"


// helper functions for "!for" and "!do"

// parse a loop body (TODO - also use for macro body?)
static void parse_ram_block(struct block *block)
{
	Input_now->line_number = block->start;	// set line number to loop start
	Input_now->src.ram_ptr = block->body;	// set RAM read pointer to loop
	// parse block
	Parse_until_eob_or_eof();
	if (GotByte != CHAR_EOB)
		Bug_found("IllegalBlockTerminator", GotByte);
}


// back end function for "!for" pseudo opcode
void flow_forloop(struct for_loop *loop)
{
	struct input	loop_input,
			*outer_input;
	struct result	loop_counter;

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
	loop_counter.val.intval = loop->counter.first;
	loop_counter.addr_refs = loop->counter.addr_refs;
	symbol_set_value(loop->symbol, &loop_counter, TRUE);
	if (loop->old_algo) {
		// old algo for old syntax:
		// if count == 0, skip loop
		if (loop->counter.last) {
			do {
				loop_counter.val.intval += loop->counter.increment;
				symbol_set_value(loop->symbol, &loop_counter, TRUE);
				parse_ram_block(&loop->block);
			} while (loop_counter.val.intval < loop->counter.last);
		}
	} else {
		// new algo for new syntax:
		do {
			parse_ram_block(&loop->block);
			loop_counter.val.intval += loop->counter.increment;
			symbol_set_value(loop->symbol, &loop_counter, TRUE);
		} while (loop_counter.val.intval != (loop->counter.last + loop->counter.increment));
	}
	// restore previous input:
	Input_now = outer_input;
}


// try to read a condition into DynaBuf and store copy pointer in
// given loop_condition structure.
// if no condition given, NULL is written to structure.
// call with GotByte = first interesting character
void flow_store_doloop_condition(struct loop_condition *condition, char terminator)
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
	struct result	intresult;

	// first, check whether there actually *is* a condition
	if (condition->body == NULL)
		return TRUE;	// non-existing conditions are always true

	// set up input for expression evaluation
	Input_now->line_number = condition->line;
	Input_now->src.ram_ptr = condition->body;
	GetByte();	// proceed with next char
	ALU_defined_int(&intresult);
	if (GotByte)
		Throw_serious_error(exception_syntax);
	return condition->is_until ? !intresult.val.intval : !!intresult.val.intval;
}


// back end function for "!do" pseudo opcode
void flow_doloop(struct do_loop *loop)
{
	struct input	loop_input;
	struct input	*outer_input;

	// set up new input
	loop_input = *Input_now;	// copy current input structure into new
	loop_input.source_is_ram = TRUE;	// set new byte source
	// remember old input
	outer_input = Input_now;
	// activate new input (not useable yet, as pointer and
	// line number are not yet set up)
	Input_now = &loop_input;
	for (;;) {
		// check head condition
		if (!check_condition(&loop->head_cond))
			break;
		parse_ram_block(&loop->block);
		// check tail condition
		if (!check_condition(&loop->tail_cond))
			break;
	}
	// restore previous input:
	Input_now = outer_input;
	GotByte = CHAR_EOS;	// CAUTION! Very ugly kluge.
	// But by switching input, we lost the outer input's GotByte. We know
	// it was CHAR_EOS. We could just call GetByte() to get real input, but
	// then the main loop could choke on unexpected bytes. So we pretend
	// that we got the outer input's GotByte value magically back.
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
void flow_parse_and_close_file(FILE *fd, const char *filename)
{
	//TODO - check for bogus/malformed BOM and ignore!
	// be verbose
	if (config.process_verbosity > 2)
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
