// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2005 Marco Baye
// Have a look at "main.c" for further info
//
// stuff needed for both "Hypra-Ass" and "Giga-Ass"
//


// Includes
//
#include "acme.h"
#include "gighyp.h"
#include "io.h"
#include "pet2iso.h"


// Process comment (GotByte == ';')
//
void gigahypra_ConvComment(void) {

	// check whether anything follows (empty comments => empty lines)
	if(GetByte()) {
		PutByte(';');
		do
			PutByte(PET2ISO(GotByte));
		while(GetByte());
	}
}

// Process operator
//
void gigahypra_Operator(void) {// '!' was last read
	char	Middle;

	Middle	= PET2ISO(GetByte());
	if((Middle != ';') && (Middle != 0)) {
		if(GetByte() == '!') {
			switch(Middle) {

				case 'n':
				PutByte('!');
				break;

				case 'o':
				PutByte('|');
				break;

				case 'a':
				PutByte('&');
				break;

				case '=':
				PutByte('=');
				break;

				case '<':
				PutString(" < ");
				break;

				case '>':
				PutString(" > ");
				break;

				default:
				PutByte('!');
				PutByte(Middle);
				PutByte('!');
			}
			GetByte();
		} else {
			PutByte('!');
			PutByte(Middle);
		}
	} else
		PutByte('!');
	// exit with unused byte pre-read
}

// output one or two TABs
//
void gigahypra_Indent(int indent) {
	if(indent < 8)
		PutByte('\t');
	PutByte('\t');
}

// Process opcode and arguments
//
void gigahypra_Opcode(int flags) {
	int	paren	= 0;	// number of open parentheses (to close)

	// if needed, add separating space between opcode and argument
	if((flags & FLAG_INSERT_SPACE) && (GotByte != ' ') && (GotByte != ';') && (GotByte != '\0'))
			PutByte(' ');
	// character loop
	while((GotByte != ';') && (GotByte != '\0')) {
		if(GotByte == '!')
			gigahypra_Operator();
		if(GotByte == '"') {
			// don't parse inside quotes
			PutByte(GotByte);
			GetByte();
			while((GotByte != '\0') && (GotByte != '"')) {
				if((GotByte == 0x5f)
				&& (flags & FLAG_CHANGE_LEFTARROW))
					PutString("\", 13,\"");
				else
					PutByte(PET2ISO(GotByte));
				GetByte();
			}
			PutByte('"');
			if(GotByte == '"') {
				GetByte();
				if((GotByte == '\0')
				&& (flags & FLAG_ADD_ZERO))
					PutString(", 0");
			}
		} else {
			// most characters go here
			switch(GotByte) {

				case '(':
				if(flags & FLAG_SKIP_OPENING) {
					flags &= ~FLAG_SKIP_OPENING;
					flags |= FLAG_SKIP_CLOSING;
				} else {
					paren++;
					PutByte(PET2ISO(GotByte));
				}
				break;

				case ')':
				if((flags & FLAG_SKIP_CLOSING) && (paren == 0))
					flags &= ~FLAG_SKIP_CLOSING;
				else {
					paren--;
					PutByte(PET2ISO(GotByte));
				}
				break;

				case ' ':	// shift-space
				PutByte(' ');	// space
				break;

				default:
				PutByte(PET2ISO(GotByte));

			}
			GetByte();
		}
	}
	if(flags & FLAG_ADD_CBM)
		PutString(PseudoTrail_ToFile);
	if(flags & FLAG_ADD_LEFT_BRACE)
		PutByte('{');
}

// Convert and send label name.
// Returns length (for proper indentation).
//
int gigahypra_LabelDef(void) {
	int	count	= 0;

	do {
		PutByte(PET2ISO(GotByte));
		count++;
		GetByte();
	} while((GotByte != ' ') && (GotByte != ';') && (GotByte != 0));
	return(count);
}
