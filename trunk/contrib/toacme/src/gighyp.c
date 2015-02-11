// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2005 Marco Baye
// Have a look at "main.c" for further info
//
// stuff needed for both "Hypra-Ass" and "Giga-Ass"
//


// Includes
//
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
			PutByte(Pet2ISO_Table[GotByte]);
		while(GetByte());
	}
}

// Process operator
//
void gigahypra_Operator(void) {// '!' was last read
	byte_t	Middle;

	Middle	= Pet2ISO_Table[GetByte()];
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
				fputs(" < ", global_output_stream);
				break;

				case '>':
				fputs(" > ", global_output_stream);
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
void gigahypra_Opcode(int Flags) {
	int	NowOpen	= 0;// number of parentheses to close

	// if needed, add separating space between opcode and argument
	if(((Flags & FLAGS_NOSPACE) == 0) && (GotByte != ' ') && (GotByte != ';') && (GotByte != '\0'))
			PutByte(' ');
	// character loop
	while((GotByte != ';') && (GotByte != 0)) {
		if(GotByte == '!')
			gigahypra_Operator();
		if(GotByte == '"') {
			// don't parse inside quotes
			PutByte(GotByte);
			GetByte();
			while((GotByte != 0) && (GotByte != '"')) {
				PutByte(Pet2ISO_Table[GotByte]);
				GetByte();
			}
			PutByte('"');
			if(GotByte == '"') GetByte();
		} else {
			// most characters go here
			switch(GotByte) {

				case '(':
				if(Flags & FLAGS_STRIPOPENING) {
					Flags &= ~FLAGS_STRIPOPENING;
					Flags |= FLAGS_STRIPCLOSING;
				} else {
					NowOpen++;
					PutByte(Pet2ISO_Table[GotByte]);
				}
				break;

				case ')':
				if((Flags&FLAGS_STRIPCLOSING) && (NowOpen==0))
					Flags &= ~FLAGS_STRIPCLOSING;
				else {
					NowOpen--;
					PutByte(Pet2ISO_Table[GotByte]);
				}
				break;

				default:
				PutByte(Pet2ISO_Table[GotByte]);

			}
			GetByte();
		}
	}
	if(Flags & FLAGS_ADDLEFTBRACE)
		PutByte('{');
}

// Convert and send label name.
// Returns length (for proper indentation).
//
int gigahypra_LabelDef(void) {
	int	count	= 0;

	do {
		PutByte(Pet2ISO_Table[GotByte]);
		count++;
		GetByte();
	} while((GotByte != ' ') && (GotByte != ';') && (GotByte != 0));
	return(count);
}
