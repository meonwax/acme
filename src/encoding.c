// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// Character encoding stuff
#include "encoding.h"
#include <stdio.h>
#include <string.h>
#include "alu.h"
#include "acme.h"
#include "dynabuf.h"
#include "global.h"	// FIXME - remove when no longer needed
#include "output.h"
#include "input.h"
#include "tree.h"


// struct definition
struct encoder {
	char	(*fn)(char);
	// maybe add table pointer?
};


// variables
static char		outermost_table[256];	// space for encoding table...
const struct encoder	*encoder_current;	// gets set before each pass
char			*encoding_loaded_table	= outermost_table;	// ...loaded from file


// encoder functions:


// convert raw to raw (do not convert at all)
static char encoderfn_raw(char byte)
{
	return byte;
}
// convert raw to petscii
static char encoderfn_pet(char byte)
{
	if ((byte >= 'A') && (byte <= 'Z'))
		return (char) (byte | 0x80);	// FIXME - check why SAS-C
	if ((byte >= 'a') && (byte <= 'z'))	//	wants these casts.
		return (char) (byte - 32);	//	There are more below.
	return byte;
}
// convert raw to C64 screencode
static char encoderfn_scr(char byte)
{
	if ((byte >= 'a') && (byte <= 'z'))
		return (char) (byte - 96);	// shift uppercase down
	if ((byte >= '[') && (byte <= '_'))
		return (char) (byte - 64);	// shift [\]^_ down
	if (byte == '`')
		return 64;	// shift ` down
	if (byte == '@')
		return 0;	// shift @ down
	return byte;
}
// convert raw to whatever is defined in table
static char encoderfn_file(char byte)
{
	return encoding_loaded_table[(unsigned char) byte];
}


// predefined encoder structs:


const struct encoder	encoder_raw	= {
	encoderfn_raw
};
const struct encoder	encoder_pet	= {
	encoderfn_pet
};
const struct encoder	encoder_scr	= {
	encoderfn_scr
};
const struct encoder	encoder_file	= {
	encoderfn_file
};


// keywords for "!convtab" pseudo opcode
static struct ronode	*encoder_tree	= NULL;	// tree to hold encoders
static struct ronode	encoder_list[]	= {
//no!	PREDEFNODE("file",	&encoder_file),	"!ct file" is not needed; just use {} after initial loading of table!
	PREDEFNODE(s_pet,	&encoder_pet),
	PREDEFNODE(s_raw,	&encoder_raw),
	PREDEFLAST(s_scr,	&encoder_scr),
	//    ^^^^ this marks the last element
};


// exported functions


// convert character using current encoding (exported for use by alu.c and pseudoopcodes.c)
char encoding_encode_char(char byte)
{
	return encoder_current->fn(byte);
}

// set "raw" as default encoding
void encoding_passinit(void)
{
	encoder_current = &encoder_raw;
}

// try to load encoding table from given file
void encoding_load_from_file(char target[256], FILE *stream)
{
	if (fread(target, sizeof(char), 256, stream) != 256)
		Throw_error("Conversion table incomplete.");
}

// lookup encoder held in DynaBuf and return its struct pointer (or NULL on failure)
const struct encoder *encoding_find(void)
{
	void	*node_body;

	// make sure tree is initialised
	if (encoder_tree == NULL)
		Tree_add_table(&encoder_tree, encoder_list);
	// perform lookup
	if (!Tree_easy_scan(encoder_tree, &node_body, GlobalDynaBuf)) {
		Throw_error("Unknown encoding.");
		return NULL;
	}

	return node_body;
}
