// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2015 Marco Baye
// Have a look at "main.c" for further info
//
// stuff needed for "VisAss", "AssBlaster 3.x" and/or "Flash8-AssBlaster"

#include <stdio.h>
#include "ab.h"
#include "acme.h"
#include "mnemo.h"
#include "io.h"
#include "scr2iso.h"


// comparison:
//	VisAss			AssBlaster 3.x		F8-AssBlaster
//	00-	Mnemonics?
//	48-	PseudoOps
//				80..86	Mnemonics	80..db	Mnemonics
//				87..95	Illegals
//				96..c7	Mnemonics
//				c8..d4	PseudoOps	dc..ec	PseudoOps
//				d5..fe	unused		ed..fe	unused
//	ff	line mark	ff	line mark	ff	line mark

// Mnemonic table in VisAss/AssBlaster 3.x order
const char	*visass_ab3_mnemonic_table[]	= {
	NULL,		// unused
	MnemonicCPX, MnemonicCPY,
	MnemonicLDX, MnemonicLDY,
	MnemonicSTX, MnemonicSTY,
//============================= start of illegals =============================
	MnemonicSAX,	// broken in VisAss/AB3, see docs (called AAX)
	MnemonicASR,	// broken in VisAss/AB3, see docs
	MnemonicARR,	// broken in VisAss/AB3, see docs
	MnemonicSBX,	// (called AXS)
	MnemonicDCP,
	MnemonicDOP,	// ACME uses a different opcode
	MnemonicISC,
	MnemonicJAM,	// ACME uses a different opcode (called KIL)
	"!error \"See the ToACME docs about the illegal opcode LAR.\";",
			// broken in VisAss/AB3? see docs
	MnemonicLAX,	// broken in VisAss/AB3, see docs
	MnemonicRLA, MnemonicRRA,
	MnemonicSLO, MnemonicSRE,
	MnemonicTOP,	// ACME uses a different opcode
//============================== end of illegals ==============================
	MnemonicADC, MnemonicAND, MnemonicASL,
	MnemonicBIT,
	MnemonicBCS, MnemonicBEQ, MnemonicBCC, MnemonicBMI,
	MnemonicBNE, MnemonicBPL, MnemonicBVS, MnemonicBVC,
	MnemonicBRK,
	MnemonicCLC, MnemonicCLD, MnemonicCLI, MnemonicCLV,
	MnemonicCMP,
	MnemonicDEC, MnemonicDEX, MnemonicDEY,
	MnemonicEOR,
	MnemonicINC, MnemonicINX, MnemonicINY,
	MnemonicJMP, MnemonicJSR,
	MnemonicLDA,
	MnemonicLSR,
	MnemonicNOP,
	MnemonicORA,
	MnemonicPHA, MnemonicPHP, MnemonicPLA, MnemonicPLP,
	MnemonicROL, MnemonicROR,
	MnemonicRTI, MnemonicRTS,
	MnemonicSBC,
	MnemonicSEC, MnemonicSED, MnemonicSEI,
	MnemonicSTA,
	MnemonicTAX, MnemonicTAY, MnemonicTSX,
	MnemonicTXA, MnemonicTXS, MnemonicTYA,
};


// PseudoOpcode table in VisAss/AssBlaster 3.x order
const char	*visass_ab3_pseudo_opcode_table[]	= {
	NULL,			// la	NULL because ACME does not need a pseudo opcode for label defs
	ACME_set_pc,		// ba
	ACME_po_byte,		// by
	ACME_po_fill,		// br
	ACME_po_pet,		// tx
	ACME_po_macro,		// md	see AB_PSEUDOOFFSET_MACRODEF
	ACME_endmacro,		// me
	ACME_macro_call,	// ma	see AB_PSEUDOOFFSET_MACROCALL
	ACME_po_eof,		// st
	ACME_po_scr,		// ts
	ACME_po_to,		// to	see AB_PSEUDOOFFSET_OUTFILE
	ACME_po_word,		// wo
	"; ToACME: Cannot convert \\kc.\n",	// kc
	nothing			// "nothing"
};
// constants
const char	nothing[]	= "doesnotmatter";	// rename to visass_nopseudoopcode


void visass_ab3_illegals(void)
{
	IO_put_string(
"; ToACME: Adding pseudo opcode to enable undocumented (\"illegal\") opcodes:\n"
"\t!cpu 6510\n"
"; ToACME: Support for illegal opcodes is somewhat broken in VisAss/AssBlaster.\n"
"; ToACME:   Make sure you read the ToACME docs to know what you'll have to\n"
"; ToACME:   look out for.\n"
"; ToACME:   Should work: DCP, DOP, ISC, JAM (called KIL in VisAss/AssBlaster),\n"
"; ToACME:     RLA, RRA, SBX (was called AXS in AssBlaster), SLO, SRE, TOP.\n"
"; ToACME:   Trouble: ARR, ASR, LAX, SAX (called AAX in VisAss/AssBlaster).\n"
	);
}


// constants

// generate error/warning messages
const char	error_unknown_addressing[]	= "Conversion failed: AssBlaster file contains unknown addressing mode.\n";
const char	error_unknown_compression[]	= "Conversion failed: AssBlaster file contains unknown number compression.\n";
const char	warning_unknown_number_format[]	= "Warning: AssBlaster file uses unknown number format. Fallback to hexadecimal.\n";

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
//	0x80-0xec differ between AssBlaster 3.x and Flash8-AssBlaster
// 0xed-0xfe unused ?
// 0xff end-of-line
#define AB_PSEUDOOFFSET_MACRODEF	 5	// in AB3 and F8AB
#define AB_PSEUDOOFFSET_MACROCALL	 7	// indices in PO table
#define AB_PSEUDOOFFSET_OUTFILE		10	// are equal
// after mnemonic or pseudo opcode, numbers may follow:
#define AB_NUMVAL_FLAGBIT		0x80	// indicates packed number

// pre- and postfixes for addressing modes
// don't care whether argument is 8, 16 or 24 bits wide
const char	*addressing_modes[][2]	= {
	{"",	""	},	// ($00=%.....) implied
	{" ",	""	},	// ($01=%....1) absolute
	{" ",	", x"	},	// ($02=%...1.) absolute,x
	{" ",	", y"	},	// ($03=%...11) absolute,y
	{" #",	""	},	// ($04=%..1..) immediate
	{NULL,	NULL	},	// ($05=%..1.1) unused (indirect-y)
	{NULL,	NULL	},	// ($06=%..11.) unused (indirect-y)
	{NULL,	NULL	},	// ($07=%..111) unused (indirect-y)
	{" (",	"), y"	},	// ($08=%.1...) indirect-y
	{" (",	", x)"	},	// ($09=%.1..1) indirect-x
	{" ",	""	},	// ($0a=%.1.1.) relative (=absolute, actually)
	{" (",	")"	},	// ($0b=%.1.11) indirect
	// above: used by both AB3 and F8AB (except $0a, which is no longer
	// used by F8AB. But it's indistinguishable from $01 anyway).
	// FIXME - what does AB3 do with the other unused addressing modes?
	// I think old AB3 sources may also use mode 0c!
	// below: used by F8AB only
	{NULL,	NULL	},	// ($0c=%.11..) unused (indirect-x)
	{" [",	"]"	},	// ($0d=%.11.1) indirect long
	{NULL,	NULL	},	// ($0e=%.111.) unused (absolute)
	{NULL,	NULL	},	// ($0f=%.1111) unused (absolute-x)
	{" ",	""	},	// ($10=%1....) MVP/MVN in F8AB: arg1.arg2
#define MVP_MVN_ADDRMODE	0x10
	{NULL,	NULL	},	// ($11=%1...1) unused (indirect)
	{NULL,	NULL	},	// ($12=%1..1.) unused (indirect long)
	{" [",	"], y"	},	// ($13=%1..11) indirect-y long
	{NULL,	NULL	},	// ($14=%1.1..) unused (absolute)
	{" ",	", s"	},	// ($15=%1.1.1) stack-relative
	{" (",	", s), y"	},	// ($16=%1.11.) stack-relative-indirect-y
	// from here on, unused (indirect-y)
	// addressing mode $10 (for MVP/MVN) is displayed and stored by F8AB
	// as "arg1.arg2" instead of "arg1,arg2". Therefore the following
	// constant is used to fix it on-the-fly.
};


// variables
struct vab	*conf;


// functions


//
static void generate_errors(int err_bits)
{
	if (err_bits & AB_ERRBIT_UNKNOWN_ADDRMODE) {
		fputs(error_unknown_addressing, stderr);
		fprintf(global_output_stream, "; ToACME: %s", error_unknown_addressing);
	}
	if (err_bits & AB_ERRBIT_UNKNOWN_NUMBER_COMPRESSION) {
		fputs(error_unknown_compression, stderr);
		fprintf(global_output_stream, "; ToACME: %s", error_unknown_compression);
	}
	if (err_bits & AB_ERRBIT_UNKNOWN_NUMBER_FORMAT) {
		fputs(warning_unknown_number_format, stderr);
		fprintf(global_output_stream, "; ToACME: %s", warning_unknown_number_format);
	}
}


// convert macro/label name character.
// AssBlaster allows '^', '[' and ']' in names, so replace these chars.
static char conv_name_char(char byte)
{
	byte = SCR2ISO(byte);
	switch (byte) {
	case 0x40:
		return AB_LABELSPECIAL_NUL;
	case '[':
		return AB_LABELSPECIAL_LEFT;
	case '\\':
		return AB_LABELSPECIAL_BACK;
	case ']':
		return AB_LABELSPECIAL_RIGHT;
	case '^':
		return AB_LABELSPECIAL_UP;
	default:
		return byte;
	}
}


// output binary representation of value
void AB_output_binary(unsigned long int value)
{
	int	mask	= 128;

	if (value > 0xff)
		AB_output_binary(value >> 8);
	value &= 0xff;
	while (mask) {
		IO_put_byte((value & mask) ? '1' : '0');
		mask >>= 1;
	}
}


// output hex representation of value
void AB_output_hexadecimal(unsigned long int value)
{
	if (value > 0xff)
		AB_output_hexadecimal(value >> 8);
	IO_put_low_byte_hex(value);
}


// convert and send macro/label name (until illegal character comes along)
static void pipe_global_name(void)
{
	while ((GotByte < 0x20) || ((GotByte >= '0') && (GotByte <= '9'))) {
		IO_put_byte(conv_name_char(GotByte));
		IO_get_byte();
	}
}


// convert and send label name (until illegal character comes along)
// level 1
static void pipe_name(void)
{
	if (conf->flags & VABFLAG_ADD_DOT) {
		// Dieser kleine Hack macht alle lokalen ABL-Labels
		// Auch unter ACME lokal.  nur mit '^' global markierte
		// Labels werden auch global übernommen ...
		if (GotByte == SCREENCODE_UPARROW)
			IO_get_byte();	// global:	^witharrow => witharrow
		else
			IO_put_byte('.');	// local:	allothers => .allothers
	}
	pipe_global_name();	// this does exactly what is needed
}


// parse quoted strings
static void parse_quoted(void)	// now GotByte = unhandled opening quote
{
	IO_put_byte('"');
	IO_get_byte();
	while ((GotByte != AB_ENDOFLINE) && (GotByte != '"')) {
		IO_put_byte(SCR2ISO(GotByte));
		IO_get_byte();
	}
	IO_put_byte('"');
	// closing quote is handled, but EndOfLine must remain unhandled
	if (GotByte == '"')
		IO_get_byte();
}


// parse label names, quoted strings, operators, literal values etc.
// read until AB_ENDOFLINE or AB_COMMENT. Returns error bits.
// level 1
// AB uses a full stop character ('.') in some inconvenient places, for example
// after macro names (at definitions and calls) and in the MVP/MVN addressing
// mode. The kluge variable "dot_replacement" is used to replace the '.'
// character with the correct character for ACME.
static int parse_unspecified(char dot_replacement)
{
	int	err_bits	= 0;

	while ((GotByte != AB_ENDOFLINE) && (GotByte != AB_COMMENT)) {
		// kluge: replace '.' character with current replacement and
		// remember not to replace anymore from now on.
		if (GotByte == '.') {
			GotByte = dot_replacement;	// use replacement
			dot_replacement = '.';		// in future, keep
		}
		if (GotByte & AB_NUMVAL_FLAGBIT) {
			err_bits |= conf->number_parser();
			continue;
		}
		if (GotByte < 0x20) {
			pipe_name();
			continue;
		}
		if (GotByte == '"') {
			parse_quoted();
			continue;
		}
		IO_put_byte(SCR2ISO(GotByte));
		IO_get_byte();
	}
	return err_bits;
}


// parse macro call or start of definition (beware of full stops).
// returns error bits.
static int parse_macro_stuff(void)	// now GotByte = unhandled byte
{
	// I guess local macros are useless, so don't
	// do the scope fixing as for macro names!
	pipe_global_name();
	return parse_unspecified(SPACE);	// output macro arguments
}


// process mnemonics (real opcodes). returns error bits.
// level 1
static int parse_mnemo(int mnemonic_offset)
{
	const char	*mnemonic,
			*pre,
			*post;
	int		addressing_mode,
			dot_replacement	= '.',
			err_bits	= 0;

	if (conf->address_mode_count > 2) {
		addressing_mode = IO_get_byte();	// get addressing mode
	} else {
		addressing_mode = 1;	// dummy mode for VisAss
	}
	IO_get_byte();	// and fetch next (not handled here)
	mnemonic = conf->mnemonics[mnemonic_offset];
	if (mnemonic == NULL) {
		fputs("Found unused mnemo code in input file.\n", stderr);
		mnemonic = "!error \"ToACME found unused mnemo code in input file\":";
	}
	fprintf(global_output_stream, "\t\t%s", mnemonic);
	// determine prefix and postfix of addressing mode
	if (addressing_mode < conf->address_mode_count) {
		pre = addressing_modes[addressing_mode][0];
		post = addressing_modes[addressing_mode][1];
		if (addressing_mode == MVP_MVN_ADDRMODE)
			dot_replacement = ',';	// replace '.' with ','
	} else {
		pre = NULL;
		post = NULL;
	}
	// if addressing mode is invalid, set error bit
	// output prefix (or space if invalid)
	if ((pre == NULL) || (post == NULL)) {
		err_bits |= AB_ERRBIT_UNKNOWN_ADDRMODE;
		fprintf(stderr, "Found an unknown addressing mode bit pattern ($%x). Please tell my programmer.\n", addressing_mode);
	}
	if (pre)
		IO_put_string(pre);
	else
		IO_put_byte(SPACE);
	err_bits |= parse_unspecified(dot_replacement);	// output arg
	if (post)
		IO_put_string(post);
	return err_bits;
}


// process pseudo opcodes. returns error bits.
// level 1
static int parse_pseudo_opcode(int pseudo_offset)
{
	const char	*pseudo_opcode;
	int		err_bits	= 0;

	IO_get_byte();	// and fetch next (not handled here)
	pseudo_opcode = conf->pseudo_opcodes[pseudo_offset];
	if (pseudo_opcode != nothing)
		IO_put_string("\t\t");
	else
		pseudo_opcode = NULL;
	if (pseudo_opcode) {
		IO_put_string(pseudo_opcode);
	}
	// check for special cases
	switch (pseudo_offset) {
	case AB_PSEUDOOFFSET_MACROCALL:	// (in ACME: '+')
		// ACME does not like spaces after the macro call char
		err_bits |= parse_macro_stuff();
		break;
	case AB_PSEUDOOFFSET_MACRODEF:	// macro definition
		IO_put_byte(SPACE);	// but here a space looks good :)
		err_bits |= parse_macro_stuff();
		IO_put_string(" {");
		break;
	case AB_PSEUDOOFFSET_OUTFILE:	// outfile selection
		IO_put_byte(SPACE);	// but here a space looks good :)
		err_bits |= parse_unspecified('.');	// output arg(s)
		IO_put_string(ACME_cbmformat);
		break;
	default:	// all other pseudo opcodes
		if ((pseudo_opcode)
		&& (GotByte != AB_ENDOFLINE)
		&& (GotByte != AB_COMMENT))
			IO_put_byte(SPACE);	// but here a space looks good :)
		err_bits |= parse_unspecified('.');	// output pseudo opcode's arg(s)
	}
	return err_bits;
}


// main routine for AssBlaster conversion (works for both AB3 and F8AB).
// call with first byte of first line pre-read (in GotByte)!
void AB_main(struct vab *client_config)
{
	int		err_bits;
	const char	*comment_indent;
	int		length_byte;
	int		handled;

	conf = client_config;
	ACME_switch_to_pet();
	// convert lines until EndOfFile
	while (!IO_reached_eof) {
		err_bits = 0;	// no errors yet (in this line)
		handled = 0;
		if (conf->flags & VABFLAG_HASLENGTHBYTE) {
			length_byte = GotByte;
			IO_get_byte();
		}
		comment_indent = "\t";
		if (GotByte >= conf->first_mnemonic
		&& (GotByte < conf->first_mnemonic + conf->mnemonic_count)) {
			handled = 1;
			err_bits |= parse_mnemo(GotByte - conf->first_mnemonic);
		}
		if (GotByte >= conf->first_pseudo_opcode
		&& (GotByte < conf->first_pseudo_opcode + conf->pseudo_opcode_count)) {
			handled = 1;
			err_bits |= parse_pseudo_opcode(GotByte - conf->first_pseudo_opcode);
		}
		if (GotByte >= conf->first_unused_byte_value
		&& (GotByte != AB_ENDOFLINE)) {
			handled = 1;
			fprintf(global_output_stream, "; ToACME: AssBlaster file used unknown code ($%x). ", GotByte);
			IO_get_byte();	// fetch next
			err_bits |= parse_unspecified('.');	// output remainder
		}
		if (handled == 0) {
			switch (GotByte) {
			case 0:	// empty line
				IO_get_byte();	// skip this byte
				break;
			case AB_COMMENT:	// early comment
				comment_indent = "";
				break;	// ignore now, act later
			case AB_SPACE:	// empty line or late comment
				comment_indent = "\t\t\t\t";
				IO_get_byte();	// skip this space
				// output whatever found
				err_bits |= parse_unspecified('.');
				break;
			default:	// implied label definition
				pipe_name();
			}
		}

		// everything might be followed by a comment, so check
		if (GotByte == AB_COMMENT) {
			// skip empty comments by checking next byte
			if (IO_get_byte() != AB_ENDOFLINE) {
				// something's there, so pipe until end of line
				IO_put_string(comment_indent);
				IO_put_byte(';');
				do
					IO_put_byte(SCR2ISO(GotByte));
				while (IO_get_byte() != AB_ENDOFLINE);
			}
		}

		// now check whether line generated any errors
		if (err_bits)
			generate_errors(err_bits);

		// if not at end-of-line, something's fishy
		if (GotByte != AB_ENDOFLINE) {
			if (GotByte == '\0') {
				IO_put_string("; ToACME: found $00 - looks like end-of-file marker.");
			} else {
				fputs("Found data instead of end-of-line indicator!?.\n", stderr);
				IO_put_string("; ToACME: Garbage at end-of-line:");
				do
					IO_put_byte(SCR2ISO(GotByte));
				while (IO_get_byte() != AB_ENDOFLINE);
			}
		}
		IO_put_byte('\n');
		// read first byte of next line
		IO_get_byte();
	}
}
