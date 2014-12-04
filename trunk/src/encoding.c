// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2014 Marco Baye
// Have a look at "acme.c" for further info
//
// Character encoding stuff
#include <stdio.h>
#include <string.h>
#include "alu.h"
#include "acme.h"
#include "dynabuf.h"
#include "encoding.h"
#include "global.h"	// FIXME - remove when no longer needed
#include "output.h"
#include "pseudoopcodes.h"	// FIXME - remove when no longer needed
#include "input.h"
#include "tree.h"


// struct definition
struct encoder {
	char	(*fn)(char);
	// maybe add table pointer?
};


// constants
static const char	s_pet[]		= "pet";
static const char	s_raw[]		= "raw";
static const char	s_scr[]		= "scr";


// variables
static char		outermost_table[256];	// space for encoding table...
static char		*loaded_table	= outermost_table;	// ...loaded from file
static struct encoder	*current_encoder;	// gets set before each pass


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
	return loaded_table[(unsigned char) byte];
}


// predefined encoder structs:


static struct encoder	encoder_raw	= {
	encoderfn_raw
};
static struct encoder	encoder_pet	= {
	encoderfn_pet
};
static struct encoder	encoder_scr	= {
	encoderfn_scr
};
static struct encoder	encoder_file	= {
	encoderfn_file
};


// functions

// convert character using current encoding (exported for use by alu.c)
char encoding_encode_char(char byte)
{
	return current_encoder->fn(byte);
}

// insert string(s)
static enum eos encode_string(struct encoder *inner_encoder, char xor)
{
	struct encoder	*outer_encoder	= current_encoder;	// buffer encoder

	// make given encoder the current one (for ALU-parsed values)
	current_encoder = inner_encoder;
	do {
		if (GotByte == '"') {
			// read initial character
			GetQuotedByte();
			// send characters until closing quote is reached
			while (GotByte && (GotByte != '"')) {
				output_8(xor ^ current_encoder->fn(GotByte));
				GetQuotedByte();
			}
			if (GotByte == CHAR_EOS)
				return AT_EOS_ANYWAY;

			// after closing quote, proceed with next char
			GetByte();
		} else {
			// Parse value. No problems with single characters
			// because the current encoding is
			// temporarily set to the given one.
			output_8(ALU_any_int());
		}
	} while (Input_accept_comma());
	current_encoder = outer_encoder;	// reactivate buffered encoder
	return ENSURE_EOS;
}

// read encoding table from file
static enum eos user_defined_encoding(void)
{
	FILE		*fd;
	char		local_table[256],
			*buffered_table		= loaded_table;
	struct encoder	*buffered_encoder	= current_encoder;

	fd = fopen(GLOBALDYNABUF_CURRENT, FILE_READBINARY);
	if (fd) {
		if (fread(local_table, sizeof(char), 256, fd) != 256)
			Throw_error("Conversion table incomplete.");
		fclose(fd);
	} else {
		Throw_error(exception_cannot_open_input_file);
	}
	current_encoder = &encoder_file;	// activate new encoding
	loaded_table = local_table;		// activate local table
	// If there's a block, parse that and then restore old values
	if (Parse_optional_block()) {
		current_encoder = buffered_encoder;
	} else {
		// if there's *no* block, the table must be used from now on.
		// copy the local table to the "outer" table
		memcpy(buffered_table, local_table, 256);
	}
	// re-activate "outer" table (it might have been changed by memcpy())
	loaded_table = buffered_table;
	return ENSURE_EOS;
}

// keywords for "!convtab" pseudo opcode
static struct ronode	*encoder_tree	= NULL;	// tree to hold encoders
static struct ronode	encoder_list[]	= {
//no!	PREDEFNODE("file",	&encoder_file),	"!ct file" is not needed; just use {} after initial loading of table!
	PREDEFNODE(s_pet,	&encoder_pet),
	PREDEFNODE(s_raw,	&encoder_raw),
	PREDEFLAST(s_scr,	&encoder_scr),
	//    ^^^^ this marks the last element
};


// use one of the pre-defined encodings (raw, pet, scr)
static enum eos predefined_encoding(void)
{
	void		*node_body;
	char		local_table[256],
			*buffered_table		= loaded_table;
	struct encoder	*buffered_encoder	= current_encoder;

	// use one of the pre-defined encodings
	if (Input_read_and_lower_keyword()) {
		// make sure tree is initialised
		if (encoder_tree == NULL)
			Tree_add_table(&encoder_tree, encoder_list);
		// perform lookup
		if (Tree_easy_scan(encoder_tree, &node_body, GlobalDynaBuf)) {
			current_encoder = (struct encoder *) node_body;	// activate new encoder
		} else {
			Throw_error("Unknown encoding.");
		}
	}
	loaded_table = local_table;	// activate local table
	// If there's a block, parse that and then restore old values
	if (Parse_optional_block())
		current_encoder = buffered_encoder;
	// re-activate "outer" table
	loaded_table = buffered_table;
	return ENSURE_EOS;
}


// exported functions

// set "raw" as default encoding
void encoding_passinit(void)
{
	current_encoder = &encoder_raw;
}



// FIXME - move this to pseudoopcodes.c:

// insert text string (default format)
static enum eos po_text(void)
{
	return encode_string(current_encoder, 0);
}

// insert raw string
static enum eos po_raw(void)
{
	return encode_string(&encoder_raw, 0);
}

// insert PetSCII string
static enum eos po_pet(void)
{
	return encode_string(&encoder_pet, 0);
}

// insert screencode string
static enum eos po_scr(void)
{
	return encode_string(&encoder_scr, 0);
}

// insert screencode string, XOR'd
static enum eos po_scrxor(void)
{
	intval_t	num	= ALU_any_int();

	if (Input_accept_comma() == FALSE) {
		Throw_error(exception_syntax);
		return SKIP_REMAINDER;
	}
	return encode_string(&encoder_scr, num);
}

// "!cbm" pseudo opcode (now obsolete)
static enum eos po_cbm(void)
{
	Throw_error("\"!cbm\" is obsolete; use \"!ct pet\" instead.");
	return ENSURE_EOS;
}

// Set current encoding ("!convtab" pseudo opcode)
static enum eos po_convtab(void)
{
	if ((GotByte == '<') || (GotByte == '"')) {
		// if file name is missing, don't bother continuing
		if (Input_read_filename(TRUE))
			return SKIP_REMAINDER;
		return user_defined_encoding();
	} else {
		return predefined_encoding();
	}
}

// pseudo opcode table
static struct ronode	pseudo_opcodes[]	= {
	PREDEFNODE(s_cbm,	po_cbm),
	PREDEFNODE("ct",	po_convtab),
	PREDEFNODE("convtab",	po_convtab),
	PREDEFNODE(s_pet,	po_pet),
	PREDEFNODE(s_raw,	po_raw),
	PREDEFNODE(s_scr,	po_scr),
	PREDEFNODE(s_scrxor,	po_scrxor),
	PREDEFNODE("text",	po_text),
	PREDEFLAST("tx",	po_text),
	//    ^^^^ this marks the last element
};

// register pseudo opcodes
void Encoding_init(void)
{
	Tree_add_table(&pseudo_opcode_tree, pseudo_opcodes);
}
