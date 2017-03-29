// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// flow control stuff (loops, conditional assembly etc.)
#ifndef flow_H
#define flow_H


#include <stdio.h>
#include "config.h"


struct block {
	int	start;	// line number of start of block
	char	*body;
};

// struct to pass "!for" loop stuff from pseudoopcodes.c to flow.c
struct for_loop {
	struct symbol	*symbol;
	int		old_algo;	// actually bool
	struct {
		intval_t	first,
				last,
				increment;
		int		addr_refs;	// address reference count
	} counter;
	struct block	block;
};

// structs to pass "!do" loop stuff from pseudoopcodes.c to flow.c
struct loop_condition {
	int	line;	// original line number
	int	is_until;	// actually bool (0 for WHILE, 1 for UNTIL)
	char	*body;	// pointer to actual expression
};
struct do_loop {
	struct loop_condition	head_cond;
	struct block		block;
	struct loop_condition	tail_cond;
};


// back end function for "!for" pseudo opcode
extern void flow_forloop(struct for_loop *loop);
// try to read a condition into DynaBuf and store copy pointer in
// given loop_condition structure.
// if no condition given, NULL is written to structure.
// call with GotByte = first interesting character
extern void flow_store_doloop_condition(struct loop_condition *condition, char terminator);
// back end function for "!do" pseudo opcode
extern void flow_doloop(struct do_loop *loop);
// parse a whole source code file
extern void flow_parse_and_close_file(FILE *fd, const char *filename);
// parse {block} [else {block}]
extern void flow_parse_block_else_block(int parse_first);


#endif
