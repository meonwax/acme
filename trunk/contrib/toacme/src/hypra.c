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
	byte_t	a,
		b	= '\0';
	bool	fBlah	= TRUE;
	int	Flags	= 0;

	a = Pet2ISO_Table[GetByte()];
	if((a != ' ') && (a != ';') && (a != '\0')) {
		b = Pet2ISO_Table[GetByte()];
		if((b != ' ') && (b != ';') && (b != '\0')) {
			switch(a) {

				case '.':
				if(b == '.') {	// "..." = macro call
					fBlah = FALSE;
					fputs(PseudoOp_MacroCall, global_output_stream);
					Flags |= FLAGS_STRIPOPENING;
				}
				break;

				case 'a':
				if(b == 'p') {	// ".ap" = append source file
					fBlah = FALSE;
					fputs(PseudoOp_Source, global_output_stream);
				}
				break;

				case 'b':
				if(b == 'a') {	// ".ba" = set base address
					fBlah = FALSE;
					fputs(PseudoOp_SetPC, global_output_stream);
				}
				if(b == 'y') {	// ".by" = insert bytes
					fBlah = FALSE;
					fputs(PseudoOp_Byte, global_output_stream);
				}
				break;

				case 'e':
				switch(b) {

					case 'i':	// ".ei" = endif
					fBlah = FALSE;
					fputs(PseudoOp_EndIf, global_output_stream);
					break;

					case 'l':	// ".el" = else
					fBlah = FALSE;
					fputs(PseudoOp_Else, global_output_stream);
					break;

					case 'n':	// ".en" = end
					fBlah = FALSE;
					fputs(PseudoOp_EOF, global_output_stream);
					break;

					case 'q':	// ".eq" = label def
					fBlah = FALSE;
					Flags |= FLAGS_NOSPACE;
					break;
				}
				break;

				case 'g':
				if(b == 'l')	// ".gl" = global label def
					fBlah = FALSE;
				Flags |= FLAGS_NOSPACE;
				break;

				case 'i':
				if(b == 'f') {	// ".if" = conditional assembly
					fBlah = FALSE;
					fputs(PseudoOp_If, global_output_stream);
					Flags |= FLAGS_ADDLEFTBRACE;
				}
				break;

				case 'm':
				if(b == 'a') {	// ".ma" = macro definition
					fBlah = FALSE;
					fputs(PseudoOp_MacroDef, global_output_stream);
					Flags |= FLAGS_STRIPOPENING | FLAGS_ADDLEFTBRACE;
				}
				break;

				case 'o':
				if(b == 'b') {	// ".ob" = output to file
					fBlah = FALSE;
					fputs(PseudoOp_ToFile, global_output_stream);
				}
				break;

				case 'r':
				if(b == 't') {	// ".rt" = end of macro def
					fBlah = FALSE;
					fputs(PseudoOp_EndMacroDef, global_output_stream);
				}
				break;

				case 's':
				if(b == 'y') {	// ".sy" = symbol dump
					fBlah = FALSE;
					fputs(PseudoOp_LabelDump, global_output_stream);
					fputs("\"symboldump.txt\";", global_output_stream);
				}
				break;

				case 't':
				if(b == 'x') {	// ".tx" = insert string
					fBlah = FALSE;
					fputs(PseudoOp_PetTxt, global_output_stream);
				}
				if(b == 's') {	// ".ts" = screen code string
					fBlah = FALSE;
					fputs(PseudoOp_ScrTxt, global_output_stream);
				}
				break;

				case 'w':
				if(b == 'o') {	// ".wo" = insert words
					fBlah = FALSE;
					fputs(PseudoOp_Word, global_output_stream);
				}
				break;

			}
			GetByte();// exit with unused byte pre-read
		} else
			fBlah = TRUE;
	} else
		fBlah = TRUE;
	if(fBlah) {
		fputs("; ToACME: .", global_output_stream);
		if(a)
			PutByte(a);
		if(b)
			PutByte(b);
		fputs(" cannot be converted\n", global_output_stream);
	}
	return(Flags);
}

// Process ocpode
//
int hypra_RealOpcode(void) {// character was last read

	PutByte(Pet2ISO_Table[GotByte]);
	GetByte();
	if((GotByte == ' ') || (GotByte == ';') || (GotByte == 0))
		return(0);
	PutByte(Pet2ISO_Table[GotByte]);
	GetByte();
	if((GotByte == ' ') || (GotByte == ';') || (GotByte == 0))
		return(0);
	PutByte(Pet2ISO_Table[GotByte]);
	GetByte();// exit with unused byte pre-read
	return(0);
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
			fputs("; ToACME: Found BASIC end marker.\n", global_output_stream);

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
			else
				gigahypra_Opcode(hypra_RealOpcode());
		}

		// skip comment, if there is one
		if(GotByte == ';')
			gigahypra_ConvComment();

		// end of line
		PutByte('\n');
	}
}
