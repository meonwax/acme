// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2016 Marco Baye
// Have a look at "main.c" for further info
//
// "Professional Ass by Oliver Stiller" stuff (NOT "Profi-Ass" by Data Becker!)
// Kickoff:
// http://www.forum64.de/wbb4/index.php?thread/60047-toacme-demn%C3%A4chst-als-win32-release/

#include <stdio.h>
//#include "config.h"
//#include "acme.h"
//#include "mnemo.h"
#include "io.h"
#include "scr2iso.h"

// format of input files:
// load address, lo
// load address, hi
// for each line:
//	total line length, including this byte
//	amount of indentation
//	actual line as screen code (this part might be missing in case of empty line)
//	total line length, including this byte (yes, the same value again, for upscrolling)
// that's it!

// special directives:
// '.' is for pseudo opcodes?
//	.setpc is *=
//	.fill (length, data)	?
//	.b is !by, .w is !wo, .macro reads a macro argument?
//	.goto is used to skip macro definitions?!
//	what do .begin and .end do? local label scope?
//	.print str$(*-$1060)	wow... O_o
// '-' prefix means local label definition?
// '+' prefix means global label definition?
// '_' starts a macro definition? ".endmacro" ends definition?
// '*' prefix is a macro call?

#define TABSIZE		8	// anything else is heresy
#define REV_START	";-=#"
#define REV_END		"#=-"

// main
void prof_main(void)
{
	int	length1,
		indent,
		ii,
		byte,
		hibit,
		length2;

	IO_set_input_padding(0);
	IO_process_load_address();
	for (;;) {
		length1 = IO_get_byte();
		if (length1 == EOF)
			break;
		if (length1 < 3) {
			fprintf(stderr, "Error: Short line (%d bytes)\n", length1);
		}
		// read amount of indentation and output tabs/spaces
		indent = IO_get_byte();
		if (indent < 0) {
			fprintf(stderr, "Error: Negative indentation (%d)\n", indent);
		} else {
			for (ii = 0; ii + TABSIZE <= indent; ii += TABSIZE)
				IO_put_byte('\t');
			for (; ii < indent; ii++)
				IO_put_byte(' ');
		}
		// now convert line
		hibit = 0;
		for (ii = 0; ii < length1 - 3; ii++) {
			byte = IO_get_byte();
			if ((byte & 128) != hibit) {
				hibit = byte & 128;
				IO_put_string(hibit ? REV_START : REV_END);
			}
			IO_put_byte(SCR2ISO(byte & 127));
		}
		if (hibit)
			IO_put_string(REV_END);
		// and add newline
		IO_put_byte('\n');
		// now check second length byte
		length2 = IO_get_byte();
		if (length1 != length2)
			fprintf(stderr, "Error: Length bytes differ (%d != %d)\n", length1, length2);
	}
}
