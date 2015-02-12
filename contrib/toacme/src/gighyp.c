// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// stuff needed for both "Hypra-Ass" and "Giga-Ass"

#include "acme.h"
#include "gighyp.h"
#include "io.h"
#include "pet2iso.h"


// called with GotByte == ';'
void GigaHypra_comment(void)
{
	// check whether anything follows (empty comments => empty lines)
	if (IO_get_byte()) {
		IO_put_byte(';');
		do
			IO_put_byte(PET2ISO(GotByte));
		while (IO_get_byte());
	}
}


// process operator
void GigaHypra_operator(void)	// '!' was last read
{
	char	middle	= PET2ISO(IO_get_byte());

	if ((middle != ';') && (middle != '\0')) {
		if (IO_get_byte() == '!') {
			switch (middle) {
			case 'n':
				IO_put_byte('!');
				break;
			case 'o':
				IO_put_byte('|');
				break;
			case 'a':
				IO_put_byte('&');
				break;
			case '=':
				IO_put_byte('=');
				break;
			case '<':
				IO_put_string(" < ");
				break;
			case '>':
				IO_put_string(" > ");
				break;
			default:
				IO_put_byte('!');
				IO_put_byte(middle);
				IO_put_byte('!');
			}
			IO_get_byte();
		} else {
			IO_put_byte('!');
			IO_put_byte(middle);
		}
	} else {
		IO_put_byte('!');
	}
	// exit with unused byte pre-read
}


// output one or two TABs
void GigaHypra_indent(int indent)
{
	if (indent < 8)
		IO_put_byte('\t');
	IO_put_byte('\t');
}


// Process opcode and arguments
void GigaHypra_argument(int flags)
{
	int	paren	= 0;	// number of open parentheses (to close)

	// if needed, add separating space between opcode and argument
	if ((flags & FLAG_INSERT_SPACE) && (GotByte != SPACE)
	&& (GotByte != ';') && (GotByte != '\0'))
			IO_put_byte(SPACE);
	// character loop
	while ((GotByte != ';') && (GotByte != '\0')) {
		if (GotByte == '!')
			GigaHypra_operator();
		if (GotByte == '"') {
			// don't parse inside quotes
			IO_put_byte(GotByte);
			IO_get_byte();
			while ((GotByte != '\0') && (GotByte != '"')) {
				if ((GotByte == 0x5f)
				&& (flags & FLAG_CHANGE_LEFTARROW))
					IO_put_string("\", 13,\"");
				else
					IO_put_byte(PET2ISO(GotByte));
				IO_get_byte();
			}
			IO_put_byte('"');
			if (GotByte == '"') {
				IO_get_byte();
				if ((GotByte == '\0')
				&& (flags & FLAG_ADD_ZERO))
					IO_put_string(", 0");
			}
		} else {
			// most characters go here
			switch (GotByte) {
			case '(':
				if (flags & FLAG_SKIP_OPENING) {
					flags &= ~FLAG_SKIP_OPENING;
					flags |= FLAG_SKIP_CLOSING;
				} else {
					paren++;
					IO_put_byte(PET2ISO(GotByte));
				}
				break;
			case ')':
				if ((flags & FLAG_SKIP_CLOSING) && (paren == 0)) {
					flags &= ~FLAG_SKIP_CLOSING;
				} else {
					paren--;
					IO_put_byte(PET2ISO(GotByte));
				}
				break;
			case SHIFTSPACE:
				IO_put_byte(SPACE);
				break;
			default:
				IO_put_byte(PET2ISO(GotByte));
			}
			IO_get_byte();
		}
	}
	if (flags & FLAG_ADD_CBM)
		IO_put_string(ACME_cbmformat);
	if (flags & FLAG_ADD_LEFT_BRACE)
		IO_put_byte('{');
}


// convert and send label name.
// returns length (for proper indentation).
int GigaHypra_label_definition(void)
{
	int	count	= 0;

	do {
		IO_put_byte(PET2ISO(GotByte));
		count++;
		IO_get_byte();
	} while ((GotByte != SPACE) && (GotByte != ';') && (GotByte != '\0'));
	return count;
}
