// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2005 Marco Baye
// Have a look at "main.c" for further info
//
// stuff needed for both "AssBlaster 3.x" and "F8-AssBlaster"
//


// Includes
//
#include <stdio.h>
#include "ab.h"
#include "acme.h"
#include "io.h"
#include "scr2iso.h"


// Constants
//
#define SCREENCODE_UPARROW	(0x1e)
// replacement characters for problematic label names
#define AB_LABELSPECIAL_NUL	('O')	// AssBlaster uses only lower case
#define AB_LABELSPECIAL_LEFT	('L')	// characters for labels, so these
#define AB_LABELSPECIAL_BACK	('B')	// shouldn't cause any clashes.
#define AB_LABELSPECIAL_RIGHT	('R')
#define AB_LABELSPECIAL_UP	('A')
// meaning of input bytes
// 0x01-0x1f lower case screen codes (used for label names and comments)
#define AB_SPACE	0x20
// 0x20-0x3a special characters
#define AB_COMMENT	0x3b
// 0x3c-0x40 unused ?
// 0x41-0x5f upper case screen codes (used for comments)
// 0x60-0x7f unused ?
#define AB_FIRST_MNEMONIC	0x80
//	0x80-0xec differ between AssBlaster 3.x and F8-AssBlaster
// 0xed-0xfe unused ?
// 0xff end-of-line
#define AB_PSEUDOOFFSET_MACROCALL	7	// index in PO table is equal
#define AB_PSEUDOOFFSET_MACRODEF	5	// in AB3.x and F8-AB
// after mnemonic or pseudo opcode, numbers may follow:
#define AB_NUMVAL_FLAGBIT		0x80	// indicates packed number

// Pre- and postfixes for addressing modes
// Don't care whether argument is 8, 16 or 24 bits wide
const char*	ab_address_modes[][2]	= {
	{"",	""	},	// ($00=%.....) implicit
	{" ",	""	},	// ($01=%....1) absolute
	{" ",	",x"	},	// ($02=%...1.) absolute,x
	{" ",	",y"	},	// ($03=%...11) absolute,y
	{" #",	""	},	// ($04=%..1..) immediate
	{NULL,	NULL	},	// ($05=%..1.1) unused (indirect-y)
	{NULL,	NULL	},	// ($06=%..11.) unused (indirect-y)
	{NULL,	NULL	},	// ($07=%..111) unused (indirect-y)
	{" (",	"),y"	},	// ($08=%.1...) indirect-y
	{" (",	",x)"	},	// ($09=%.1..1) indirect-x
	{" ",	""	},	// ($0a=%.1.1.) relative (=absolute, actually)
	{" (",	")"	},	// ($0b=%.1.11) indirect
	// above: used by both AB3 and F8AB (except $0a, which is no longer
	// used by F8AB. But it's indistinguishable from $01 anyway).
	// FIXME - what does AB3 with the other unused addressing modes?
	// I think old AB3 sources may also use mode 0c!
	// below: used by F8AB only
	{NULL,	NULL	},	// ($0c=%.11..) unused (indirect-x)
	{" [",	"]"	},	// ($0d=%.11.1) indirect long
	{NULL,	NULL	},	// ($0e=%.111.) unused (absolute)
	{NULL,	NULL	},	// ($0f=%.1111) unused (absolute-x)
	{" ",	""	},	// ($10=%1....) MVP/MVN in F8AB: arg1.arg2
	{NULL,	NULL	},	// ($11=%1...1) unused (indirect)
	{NULL,	NULL	},	// ($12=%1..1.) unused (indirect long)
	{" [",	"],y"	},	// ($13=%1..11) indirect-y long
	{NULL,	NULL	},	// ($14=%1.1..) unused (absolute)
	{" ",	",s"	},	// ($15=%1.1.1) stack-relative
	{" (",	",s),y"	},	// ($16=%1.11.) stack-relative-indirect-y
	// from here on, unused (indirect-y)
	// addressing mode $10 (for MVP/MVN) is displayed and stored by F8AB
	// as "arg1.arg2" instead of "arg1,arg2". Therefore the following
	// constant is used to fix it on-the-fly.
};
#define AB_MVP_MVN_ADDRMODE	0x10


// Variables
//
struct ab_t*	conf;


// Functions
//


// Generate error/warning messages
//
const char	error_unknown_addressing[]	= "Conversion failed: AssBlaster file contains unknown addressing mode.\n";
const char	error_unknown_compression[]	= "Conversion failed: AssBlaster file contains unknown number compression.\n";
const char	warning_unknown_number_format[]	= "Warning: AssBlaster file uses unknown number format. Fallback to hexadecimal.\n";
void ab_generate_errors(int ErrBits) {
	if(ErrBits & AB_ERRBIT_UNKNOWN_ADDRMODE) {
		fputs(error_unknown_addressing, stderr);
		fprintf(global_output_stream, "; ToACME: %s", error_unknown_addressing);
	}
	if(ErrBits & AB_ERRBIT_UNKNOWN_NUMBER_COMPRESSION) {
		fputs(error_unknown_compression, stderr);
		fprintf(global_output_stream, "; ToACME: %s", error_unknown_compression);
	}
	if(ErrBits & AB_ERRBIT_UNKNOWN_NUMBER_FORMAT) {
		fputs(warning_unknown_number_format, stderr);
		fprintf(global_output_stream, "; ToACME: %s", warning_unknown_number_format);
	}
}

// Convert macro/label name character.
// AssBlaster allows '^', '[' and ']' in names, so replace these chars.
//
byte_t ab_conv_name_char(byte_t b) {
	b = Scr2ISO_Table[b];
	switch(b) {
		case 0x40:
		return(AB_LABELSPECIAL_NUL);
		case '[':
		return(AB_LABELSPECIAL_LEFT);
		case '\\':
		return(AB_LABELSPECIAL_BACK);
		case ']':
		return(AB_LABELSPECIAL_RIGHT);
		case '^':
		return(AB_LABELSPECIAL_UP);
		default:
		return(b);
	}
}

// Output binary representation of value
//
void ab_output_binary(unsigned long int v) {
	int	m	= 128;

	if(v > 0xff)
		ab_output_binary(v >> 8);
	v &= 0xff;
	while(m) {
		PutByte((v & m) ? '1' : '0');
		m >>= 1;
	}
}

// Output hex representation of value
//
void ab_output_hexadecimal(unsigned long int v) {
	if(v > 0xff)
		ab_output_hexadecimal(v >> 8);
	io_put_low_byte_hex(v);
}

// Convert and send macro/label name (until illegal character comes along)
//
void ab_pipe_global_name(void) {
	while((GotByte < 0x20) || ((GotByte >= '0') && (GotByte <= '9'))) {
		PutByte(ab_conv_name_char(GotByte));
		GetByte();
	}
}

// Convert and send label name (until illegal character comes along)
// Level 1
void ab_pipe_name(void) {
	// Dieser kleine Hack macht alle lokalen ABL-Labels
	// Auch unter ACME lokal.  nur mit '^' global markierte
	// Labels werden auch global übernommen ...
	if(GotByte == SCREENCODE_UPARROW)
		GetByte();	// global:	^witharrow => witharrow
	else
		PutByte('.');	// local:	allothers => .allothers
	ab_pipe_global_name();	// this does exactly what is needed
}

// Parse quoted strings
//
void ab_parse_quoted(void) {// now GotByte = unhandled opening quote

	PutByte('"');
	GetByte();
	while((GotByte != AB_ENDOFLINE) && (GotByte != '"')) {
		PutByte(Scr2ISO_Table[GotByte]);
		GetByte();
	}
	PutByte('"');

	// Closing quote is handled, but EndOfLine must remain unhandled
	if(GotByte == '"')
		GetByte();
}

// Parse label names, quoted strings, operators, literal values etc.
// Read until AB_ENDOFLINE or AB_COMMENT. Returns error bits.
// Level 1
// AB uses a full stop character ('.') in some inconvenient places, for example
// after macro names (at definitions and calls) and in the MVP/MVN addressing
// mode. The kluge variable "dot_replacement" is used to replace the '.'
// character with the correct character for ACME.
int ab_parse_unspecified(char dot_replacement) {
	int	ErrBits	= 0;

	while((GotByte != AB_ENDOFLINE) && (GotByte != AB_COMMENT)) {
		// kluge: replace '.' character with current replacement and
		// remember not to replace anymore from now on.
		if(GotByte == '.') {
			GotByte = dot_replacement;	// use replacement
			dot_replacement = '.';		// in future, keep
		}
		if(GotByte & AB_NUMVAL_FLAGBIT)
			ErrBits |= conf->number_parser();
		else {
			if(GotByte < 0x20)
				ab_pipe_name();
			else {
				if(GotByte == '"')
					ab_parse_quoted();
				else {
					PutByte(Scr2ISO_Table[GotByte]);
					GetByte();
				}
			}
		}
	}
	return(ErrBits);
}

// Parse macro call or start of definition (beware of full stops).
// Returns error bits.
//
int ab_parse_macro_stuff(void) {	// now GotByte = unhandled byte
	// I guess local macros are useless, so don't
	// do the scope fixing as for macro names!
	ab_pipe_global_name();
	return(ab_parse_unspecified(' '));	// output macro arguments
}

// Process mnemonics (real opcodes). Returns error bits.
// Level 1
int ab_parse_mnemo(int mnemonic_offset) {
	char		dot_replacement	= '.';
	int		ErrBits	= 0;
	byte_t		AddressingMode;
	const char	*mnemonic,
			*pre,
			*post;

	AddressingMode = GetByte();	// get addressing mode
	GetByte();	// and fetch next (not handled here)
	mnemonic = conf->mnemonics[mnemonic_offset];
	if(mnemonic == NULL) {
		fputs("Found unused mnemo code in input file.\n", stderr);
		mnemonic = "!error \"ToACME found unused mnemo code in input file\":";
	}
	fprintf(global_output_stream, "\t\t%s", mnemonic);
	// determine prefix and postfix of addressing mode
	if(AddressingMode < conf->address_mode_count) {
		pre = ab_address_modes[AddressingMode][0];
		post = ab_address_modes[AddressingMode][1];
		if(AddressingMode == AB_MVP_MVN_ADDRMODE)
			dot_replacement = ',';	// replace '.' with ','
	} else {
		pre = NULL;
		post = NULL;
	}
	// if addressing mode is invalid, set error bit
	// output prefix (or space if invalid)
	if((pre == NULL) || (post == NULL)) {
		ErrBits |= AB_ERRBIT_UNKNOWN_ADDRMODE;
		fprintf(stderr, "Found an unknown addressing mode bit pattern ($%x). Please tell my programmer.\n", AddressingMode);
	}
	if(pre)
		fputs(pre, global_output_stream);
	else
		PutByte(' ');
	ErrBits |= ab_parse_unspecified(dot_replacement);	// output arg
	if(post) {
		fputs(post, global_output_stream);
	}
	return(ErrBits);
}

// Process pseudo opcodes. Returns error bits.
// Level 1
int ab_parse_pseudo_opcode(int pseudo_offset) {
	const char*	String;
	int		ErrBits	= 0;

	GetByte();	// and fetch next (not handled here)
	fputs("\t\t", global_output_stream);
	String = conf->pseudo_opcodes[pseudo_offset];
	if(String)
		fputs(String, global_output_stream);
	// check for macro call/definition (need special handlers)
	switch(pseudo_offset) {

		case AB_PSEUDOOFFSET_MACROCALL:	// (in ACME: '+')
		// ACME does not like spaces after the macro call char
		ErrBits |= ab_parse_macro_stuff();
		break;

		case AB_PSEUDOOFFSET_MACRODEF:	// macro definition
		if(String)
			PutByte(' ');// but here a space looks good :)
		ErrBits |= ab_parse_macro_stuff();
		fputs(" {", global_output_stream);
		break;

		default:	// all other pseudo opcodes
		if((String)
		&& (GotByte != AB_ENDOFLINE)
		&& (GotByte != AB_COMMENT))
			PutByte(' ');// but here a space looks good :)
		ErrBits |= ab_parse_unspecified('.');	// output pseudo opcode's arg(s)
	}
	return(ErrBits);
}

// Main routine for AssBlaster conversion (works for both AB3.x and F8-AB).
// Call with first byte of first line pre-read (in GotByte)!
//
void ab_main(struct ab_t* client_config) {
	int		ErrBits;
	const char	*comment_indent;

	conf = client_config;
	acme_SwitchToPet();
	// convert lines until EndOfFile
	while(!ReachedEOF) {
		ErrBits = 0;	// no errors yet (in this line)
		comment_indent = "\t";
		if(GotByte < AB_FIRST_MNEMONIC) {
			switch(GotByte) {

				case 0:	// empty line
				GetByte();	// skip this byte
				break;

				case AB_COMMENT:	// early comment
				comment_indent = "";
				break;	// ignore now, act later

				case AB_SPACE:	// empty line or late comment
				comment_indent = "\t\t\t\t";
				GetByte();	// skip this space
				// output whatever found
				ErrBits |= ab_parse_unspecified('.');
				break;

				default:	// implicit label definition
				ab_pipe_name();
			}
		} else if(GotByte < conf->first_pseudo_opcode) {
			ErrBits |= ab_parse_mnemo(GotByte - AB_FIRST_MNEMONIC);
		} else if(GotByte < conf->first_unused_byte_value) {
			ErrBits |= ab_parse_pseudo_opcode(GotByte - conf->first_pseudo_opcode);
		} else if(GotByte != AB_ENDOFLINE) {
			fprintf(global_output_stream, "; ToACME: AssBlaster file used unknown code ($%x). ", GotByte);
			GetByte();	// fetch next
			ErrBits |= ab_parse_unspecified('.');	// output remainder
		}

		// everything might be followed by a comment, so check
		if(GotByte == AB_COMMENT) {
			// skip empty comments by checking next byte
			if(GetByte() != AB_ENDOFLINE) {
				// something's there, so pipe until end of line
				fputs(comment_indent, global_output_stream);
				PutByte(';');
				do
					PutByte(Scr2ISO_Table[GotByte]);
				while(GetByte() != AB_ENDOFLINE);
			}
		}

		// now check whether line generated any errors
		if(ErrBits)
			ab_generate_errors(ErrBits);

		// if not at end-of-line, something's fishy
		if(GotByte != AB_ENDOFLINE) {
			if(GotByte == '\0')
				fputs("; ToACME: found $00 - looks like end-of-file marker.", global_output_stream);
			else {
				fputs("Found data instead of end-of-line indicator!?.\n", stderr);
				fputs("; ToACME: Garbage at end-of-line:", global_output_stream);
				do
					PutByte(Scr2ISO_Table[GotByte]);
				while(GetByte() != AB_ENDOFLINE);
			}
		}
		PutByte('\n');
		// read first byte of next line
		GetByte();
	}
}
