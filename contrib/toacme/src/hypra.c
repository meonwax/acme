// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// "HypraAss" stuff
//


// Includes
//
#include <stdio.h>
#include "config.h"
#include "acme.h"
#include "gighyp.h"
#include "io.h"
#include "pet2iso.h"


// Functions
//


// Process pseudo opcode
//
int hypra_PseudoOpcode(void) {// '.' was last read
	int	a,
		b	= '\0',
		flags	= FLAG_INSERT_SPACE;
	bool	fBlah	= TRUE;

	a = PET2ISO(GetByte());
	if((a != ' ') && (a != ';') && (a != '\0')) {
		b = PET2ISO(GetByte());
		if((b != ' ') && (b != ';') && (b != '\0')) {
			switch(a) {

				case '.':
				if(b == '.') {	// "..." = macro call
					fBlah = FALSE;
					PutString(PseudoOp_MacroCall);
					flags |= FLAG_SKIP_OPENING;
				}
				break;

				case 'a':
				if(b == 'p') {	// ".ap" = append source file
					fBlah = FALSE;
					PutString(PseudoOp_Source);
				}
				break;

				case 'b':
				if(b == 'a') {	// ".ba" = set base address
					fBlah = FALSE;
					PutString(PseudoOp_SetPC);
				}
				if(b == 'y') {	// ".by" = insert bytes
					fBlah = FALSE;
					PutString(PseudoOp_Byte);
				}
				break;

				case 'e':
				switch(b) {

					case 'i':	// ".ei" = endif
					fBlah = FALSE;
					PutString(PseudoOp_EndIf);
					break;

					case 'l':	// ".el" = else
					fBlah = FALSE;
					PutString(PseudoOp_Else);
					break;

					case 'n':	// ".en" = end
					fBlah = FALSE;
					PutString(PseudoOp_EOF);
					break;

					case 'q':	// ".eq" = label def
					fBlah = FALSE;
					flags &= ~FLAG_INSERT_SPACE;
					break;
				}
				break;

				case 'g':
				if(b == 'l')	// ".gl" = global label def
					fBlah = FALSE;
				flags &= ~FLAG_INSERT_SPACE;
				break;

				case 'i':
				if(b == 'f') {	// ".if" = conditional assembly
					fBlah = FALSE;
					PutString(PseudoOp_If);
					flags |= FLAG_ADD_LEFT_BRACE;
				}
				break;

				case 'm':
				if(b == 'a') {	// ".ma" = macro definition
					fBlah = FALSE;
					PutString(PseudoOp_MacroDef);
					flags |= FLAG_SKIP_OPENING | FLAG_ADD_LEFT_BRACE;
				}
				break;

				case 'o':
				if(b == 'b') {	// ".ob" = output to file
					fBlah = FALSE;
					PutString(PseudoOp_ToFile);
					flags |= FLAG_ADD_CBM;
				}
				break;

				case 'r':
				if(b == 't') {	// ".rt" = end of macro def
					fBlah = FALSE;
					PutString(PseudoOp_EndMacroDef);
				}
				break;

				case 's':
				if(b == 'y') {	// ".sy" = symbol dump
					fBlah = FALSE;
					PutString(PseudoOp_LabelDump);
					PutString("\"symboldump.txt\";");
				}
				break;

				case 't':
				if(b == 'x') {	// ".tx" = insert string
					fBlah = FALSE;
					PutString(PseudoOp_PetTxt);
				}
				if(b == 's') {	// ".ts" = screen code string
					fBlah = FALSE;
					PutString(PseudoOp_ScrTxt);
				}
				break;

				case 'w':
				if(b == 'o') {	// ".wo" = insert words
					fBlah = FALSE;
					PutString(PseudoOp_Word);
				}
				break;

			}
			GetByte();// exit with unused byte pre-read
		} else
			fBlah = TRUE;
	} else
		fBlah = TRUE;
	if(fBlah) {
		PutString("; ToACME: .");
		if(a)
			PutByte(a);
		if(b)
			PutByte(b);
		PutString(" cannot be converted\n");
	}
	return(flags);
}

// Process ocpode
//
void hypra_RealOpcode(void) {// character was last read

	PutByte(PET2ISO(GotByte));
	GetByte();
	if((GotByte == ' ') || (GotByte == ';') || (GotByte == '\0'))
		return;
	PutByte(PET2ISO(GotByte));
	GetByte();
	if((GotByte == ' ') || (GotByte == ';') || (GotByte == '\0'))
		return;
	PutByte(PET2ISO(GotByte));
	GetByte();// exit with unused byte pre-read
	return;
}

// Main routine for HypraAss conversion
//
void hypra_main(void) {
	int	indent;

	input_set_padding(0);
	io_process_load_address();
	acme_SwitchToPet();
	// loop: once for every line in the file
	while(!ReachedEOF) {
		// skip link pointer (if it's zero, report as end marker)
		if(GetLE16() == 0)
			PutString("; ToACME: Found BASIC end marker.\n");

		GetLE16();	// skip line number

		// process line
		GetByte();
		indent = 0;
		if((GotByte != ' ') && (GotByte != ';') && (GotByte != '\0'))
			indent = gigahypra_LabelDef();

		// skip spaces
		while(GotByte == ' ')
			GetByte();

		// if there is an opcode, process it
		if((GotByte != ';') && (GotByte != '\0')) {
			gigahypra_Indent(indent);
			// branch to relevant routine
			if(GotByte == '.')
				gigahypra_Opcode(hypra_PseudoOpcode());
			else {
				hypra_RealOpcode();
				gigahypra_Opcode(FLAG_INSERT_SPACE);
			}
		}

		// skip comment, if there is one
		if(GotByte == ';')
			gigahypra_ConvComment();

		// end of line
		PutByte('\n');
	}
}
