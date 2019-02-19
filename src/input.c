// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2017 Marco Baye
// Have a look at "acme.c" for further info
//
// Input stuff
// 19 Nov 2014	Merged Johann Klasek's report listing generator patch
#include "input.h"
#include "config.h"
#include "alu.h"
#include "dynabuf.h"
#include "global.h"	// FIXME - remove when no longer needed
#include "platform.h"
#include "section.h"
#include "symbol.h"
#include "tree.h"


// Constants
const char	FILE_READBINARY[]	= "rb";
#define CHAR_TAB	(9)	// Tab character
#define CHAR_LF		(10)	// line feed		(in file)
		//	(10)	// start of line	(in high-level format)
#define CHAR_CR		(13)	// carriage return	(in file)
		//	(13)	// end of file		(in high-level format)
#define CHAR_STATEMENT_DELIMITER	':'
#define	CHAR_COMMENT_SEPARATOR		';'
// if the characters above are changed, don't forget to adjust ByteFlags[]!

// fake input structure (for error msgs before any real input is established)
static struct input	outermost	= {
	"<none>",	// file name
	0,		// line number
	FALSE,		// Faked file access, so no RAM read
	INPUTSTATE_EOF,	// state of input
	{
		NULL	// RAM read pointer or file handle
	}
};


// variables
struct input	*Input_now	= &outermost;	// current input structure


// functions

// let current input point to start of file
void Input_new_file(const char *filename, FILE *fd)
{
	Input_now->original_filename	= filename;
	Input_now->line_number		= 1;
	Input_now->source_is_ram	= FALSE;
	Input_now->state		= INPUTSTATE_NORMAL;
	Input_now->src.fd		= fd;
}


// remember source code character for report generator
#define HEXBUFSIZE	9	// actually, 4+1 is enough, but for systems without snprintf(), let's be extra-safe.
#define IF_WANTED_REPORT_SRCCHAR(c)	do { if (report->fd) report_srcchar(c); } while(0)
static void report_srcchar(char new_char)
{
	static char	prev_char	= '\0';
	int		ii;
	char		hex_address[HEXBUFSIZE];
	char		hexdump[2 * REPORT_BINBUFSIZE + 2];	// +2 for '.' and terminator

	// if input has changed, insert explanation
	if (Input_now != report->last_input) {
		fprintf(report->fd, "\n; ******** Source: %s\n", Input_now->original_filename);
		report->last_input = Input_now;
		report->asc_used = 0;	// clear buffer
		prev_char = '\0';
	}
	if (prev_char == '\n') {
		// line start after line break detected and EOS processed,
		// build report line:
		// show line number...
		fprintf(report->fd, "%6d  ", Input_now->line_number - 1);
		// prepare outbytes' start address
		if (report->bin_used)
#if _BSD_SOURCE || _XOPEN_SOURCE >= 500 || _ISOC99_SOURCE || _POSIX_C_SOURCE >= 200112L
			snprintf(hex_address, HEXBUFSIZE, "%04x", report->bin_address);
#else
			sprintf(hex_address, "%04x", report->bin_address);
#endif
		else
			hex_address[0] = '\0';
		// prepare outbytes
		hexdump[0] = '\0';
		for (ii = 0; ii < report->bin_used; ++ii)
			sprintf(hexdump + 2 * ii, "%02x", (unsigned int) (unsigned char) (report->bin_buf[ii]));
		// if binary buffer is full, overwrite last byte with "..."
		if (report->bin_used == REPORT_BINBUFSIZE)
			sprintf(hexdump + 2 * (REPORT_BINBUFSIZE - 1), "...");
		// show address and bytes
		fprintf(report->fd, "%-4s %-19s", hex_address, hexdump);
		// at this point the output should be a multiple of 8 characters
		// so far to preserve tabs of the source...
		if (report->asc_used == REPORT_ASCBUFSIZE)
			--report->asc_used;
		report->asc_buf[report->asc_used] = '\0';
		fprintf(report->fd, "%s\n", report->asc_buf);	// show source line
		report->asc_used = 0;	// reset buffers
		report->bin_used = 0;
	}
	if (new_char != '\n' && new_char != '\r') {	// detect line break
		if (report->asc_used < REPORT_ASCBUFSIZE)
			report->asc_buf[report->asc_used++] = new_char;
	}
	prev_char = new_char;
}


// Deliver source code from current file (!) in shortened high-level format
static char get_processed_from_file(void)
{
	int	from_file = 0;

	for (;;) {
		switch (Input_now->state) {
		case INPUTSTATE_NORMAL:
			// fetch a fresh byte from the current source file
			from_file = getc(Input_now->src.fd);
			IF_WANTED_REPORT_SRCCHAR(from_file);
			// now process it
			/*FALLTHROUGH*/
		case INPUTSTATE_AGAIN:
			// Process the latest byte again. Of course, this only
			// makes sense if the loop has executed at least once,
			// otherwise the contents of from_file are undefined.
			// If the source is changed so there is a possibility
			// to enter INPUTSTATE_AGAIN mode without first having
			// defined "from_file", trouble may arise...
			Input_now->state = INPUTSTATE_NORMAL;
			// EOF must be checked first because it cannot be used
			// as an index into Byte_flags[]
			if (from_file == EOF) {
				// remember to send an end-of-file
				Input_now->state = INPUTSTATE_EOF;
				return CHAR_EOS;	// end of statement
			}

			// check whether character is special one
			// if not, everything's cool and froody, so return it
			if ((BYTEFLAGS(from_file) & BYTEIS_SYNTAX) == 0)
				return (char) from_file;

			// check special characters ("0x00 TAB LF CR SPC :;}")
			switch (from_file) {
			case CHAR_TAB:	// TAB character
			case ' ':
				// remember to skip all following blanks
				Input_now->state = INPUTSTATE_SKIPBLANKS;
				return ' ';

			case CHAR_LF:	// LF character
				// remember to send a start-of-line
				Input_now->state = INPUTSTATE_LF;
				return CHAR_EOS;	// end of statement

			case CHAR_CR:	// CR character
				// remember to check CRLF + send start-of-line
				Input_now->state = INPUTSTATE_CR;
				return CHAR_EOS;	// end of statement

			case CHAR_EOB:
				// remember to send an end-of-block
				Input_now->state = INPUTSTATE_EOB;
				return CHAR_EOS;	// end of statement

			case CHAR_STATEMENT_DELIMITER:
				// just deliver an EOS instead
				return CHAR_EOS;	// end of statement

			case CHAR_COMMENT_SEPARATOR:
				// remember to skip remainder of line
				Input_now->state = INPUTSTATE_COMMENT;
				return CHAR_EOS;	// end of statement

			default:
				// complain if byte is 0
				Throw_error("Source file contains illegal character.");
				return (char) from_file;
			}
		case INPUTSTATE_SKIPBLANKS:
			// read until non-blank, then deliver that
			do {
				from_file = getc(Input_now->src.fd);
				IF_WANTED_REPORT_SRCCHAR(from_file);
			} while ((from_file == CHAR_TAB) || (from_file == ' '));
			// re-process last byte
			Input_now->state = INPUTSTATE_AGAIN;
			break;
		case INPUTSTATE_LF:
			// return start-of-line, then continue in normal mode
			Input_now->state = INPUTSTATE_NORMAL;
			return CHAR_SOL;	// new line

		case INPUTSTATE_CR:
			// return start-of-line, remember to check for LF
			Input_now->state = INPUTSTATE_SKIPLF;
			return CHAR_SOL;	// new line

		case INPUTSTATE_SKIPLF:
			from_file = getc(Input_now->src.fd);
			IF_WANTED_REPORT_SRCCHAR(from_file);
			// if LF, ignore it and fetch another byte
			// otherwise, process current byte
			if (from_file == CHAR_LF)
				Input_now->state = INPUTSTATE_NORMAL;
			else
				Input_now->state = INPUTSTATE_AGAIN;
			break;
		case INPUTSTATE_COMMENT:
			// read until end-of-line or end-of-file
			do {
				from_file = getc(Input_now->src.fd);
				IF_WANTED_REPORT_SRCCHAR(from_file);
			} while ((from_file != EOF) && (from_file != CHAR_CR) && (from_file != CHAR_LF));
			// re-process last byte
			Input_now->state = INPUTSTATE_AGAIN;
			break;
		case INPUTSTATE_EOB:
			// deliver EOB
			Input_now->state = INPUTSTATE_NORMAL;
			return CHAR_EOB;	// end of block

		case INPUTSTATE_EOF:
			// deliver EOF
			Input_now->state = INPUTSTATE_NORMAL;
			return CHAR_EOF;	// end of file

		default:
			Bug_found("StrangeInputMode", Input_now->state);
		}
	}
}

// This function delivers the next byte from the currently active byte source
// in shortened high-level format. FIXME - use fn ptr?
// When inside quotes, use GetQuotedByte() instead!
char GetByte(void)
{
//	for (;;) {
		// If byte source is RAM, then no conversions are
		// necessary, because in RAM the source already has
		// high-level format
		// Otherwise, the source is a file. This means we will call
		// GetFormatted() which will do a shit load of conversions.
		if (Input_now->source_is_ram)
			GotByte = *(Input_now->src.ram_ptr++);
		else
			GotByte = get_processed_from_file();
//		// if start-of-line was read, increment line counter and repeat
//		if (GotByte != CHAR_SOL)
//			return GotByte;
//		Input_now->line_number++;
//	}
		if (GotByte == CHAR_SOL)
			Input_now->line_number++;
		return GotByte;
}

// This function delivers the next byte from the currently active byte source
// in un-shortened high-level format.
// This function complains if CHAR_EOS (end of statement) is read.
char GetQuotedByte(void)
{
	int	from_file;	// must be an int to catch EOF

	// if byte source is RAM, then no conversion is necessary,
	// because in RAM the source already has high-level format
	if (Input_now->source_is_ram) {
		GotByte = *(Input_now->src.ram_ptr++);
	// Otherwise, the source is a file.
	} else {
		// fetch a fresh byte from the current source file
		from_file = getc(Input_now->src.fd);
		IF_WANTED_REPORT_SRCCHAR(from_file);
		switch (from_file) {
		case EOF:
			// remember to send an end-of-file
			Input_now->state = INPUTSTATE_EOF;
			GotByte = CHAR_EOS;	// end of statement
			break;
		case CHAR_LF:	// LF character
			// remember to send a start-of-line
			Input_now->state = INPUTSTATE_LF;
			GotByte = CHAR_EOS;	// end of statement
			break;
		case CHAR_CR:	// CR character
			// remember to check for CRLF + send a start-of-line
			Input_now->state = INPUTSTATE_CR;
			GotByte = CHAR_EOS;	// end of statement
			break;
		default:
			GotByte = from_file;
		}

	}
	// now check for end of statement
	if (GotByte == CHAR_EOS)
		Throw_error("Quotes still open at end of line.");
	return GotByte;
}

// Skip remainder of statement, for example on error
void Input_skip_remainder(void)
{
	while (GotByte)
		GetByte();	// Read characters until end-of-statement
}

// Ensure that the remainder of the current statement is empty, for example
// after mnemonics using implied addressing.
void Input_ensure_EOS(void)	// Now GotByte = first char to test
{
	SKIPSPACE();
	if (GotByte) {
		Throw_error("Garbage data at end of statement.");
		Input_skip_remainder();
	}
}

// Skip or store block (starting with next byte, so call directly after
// reading opening brace).
// If "Store" is TRUE, the block is read into GlobalDynaBuf, then a copy
// is made and a pointer to that is returned.
// If "Store" is FALSE, NULL is returned.
// After calling this function, GotByte holds '}'. Unless EOF was found first,
// but then a serious error would have been thrown.
// FIXME - use a struct block *ptr argument!
char *Input_skip_or_store_block(int store)
{
	char	byte;
	int	depth	= 1;	// to find matching block end

	// prepare global dynamic buffer
	DYNABUF_CLEAR(GlobalDynaBuf);
	do {
		byte = GetByte();
		// if wanted, store
		if (store)
			DYNABUF_APPEND(GlobalDynaBuf, byte);
		// now check for some special characters
		switch (byte) {
		case CHAR_EOF:	// End-of-file in block? Sorry, no way.
			Throw_serious_error(exception_no_right_brace);

		case '"':	// Quotes? Okay, read quoted stuff.
		case '\'':
			do {
				GetQuotedByte();
				// if wanted, store
				if (store)
					DYNABUF_APPEND(GlobalDynaBuf, GotByte);
			} while ((GotByte != CHAR_EOS) && (GotByte != byte));
			break;
		case CHAR_SOB:
			++depth;
			break;
		case CHAR_EOB:
			--depth;
			break;
		}
	} while (depth);
	// in case of skip, return now
	if (!store)
		return NULL;
	// otherwise, prepare to return copy of block
	// add EOF, just to make sure block is never read too far
	DynaBuf_append(GlobalDynaBuf, CHAR_EOS);
	DynaBuf_append(GlobalDynaBuf, CHAR_EOF);
	// return pointer to copy
	return DynaBuf_get_copy(GlobalDynaBuf);
}

// Read bytes and add to GlobalDynaBuf until the given terminator (or CHAR_EOS)
// is found. Act upon single and double quotes by entering (and leaving) quote
// mode as needed (So the terminator does not terminate when inside quotes).
void Input_until_terminator(char terminator)
{
	char	byte	= GotByte;

	for (;;) {
		// Terminator? Exit. EndOfStatement? Exit.
		if ((byte == terminator) || (byte == CHAR_EOS))
			return;
		// otherwise, append to GlobalDynaBuf and check for quotes
		DYNABUF_APPEND(GlobalDynaBuf, byte);
		if ((byte == '"') || (byte == '\'')) {
			do {
				// Okay, read quoted stuff.
				GetQuotedByte();	// throws error on EOS
				DYNABUF_APPEND(GlobalDynaBuf, GotByte);
			} while ((GotByte != CHAR_EOS) && (GotByte != byte));
			// on error, exit now, before calling GetByte()
			if (GotByte != byte)
				return;
		}
		byte = GetByte();
	}
}

// Append to GlobalDynaBuf while characters are legal for keywords.
// Throws "missing string" error if none.
// Returns number of characters added.
int Input_append_keyword_to_global_dynabuf(void)
{
	int	length	= 0;

	// add characters to buffer until an illegal one comes along
	while (BYTEFLAGS(GotByte) & CONTS_KEYWORD) {
		DYNABUF_APPEND(GlobalDynaBuf, GotByte);
		++length;
		GetByte();
	}
	if (length == 0)
		Throw_error(exception_missing_string);
	return length;
}

// Check GotByte.
// If LOCAL_PREFIX ('.'), store current local scope value and read next byte.
// If CHEAP_PREFIX ('@'), store current cheap scope value and read next byte.
// Otherwise, store global scope value.
// Then jump to Input_read_keyword(), which returns length of keyword.
int Input_read_scope_and_keyword(scope_t *scope)
{
	SKIPSPACE();
	if (GotByte == LOCAL_PREFIX) {
		GetByte();
		*scope = section_now->local_scope;
	} else if (GotByte == CHEAP_PREFIX) {
		GetByte();
		*scope = section_now->cheap_scope;
	} else {
		*scope = SCOPE_GLOBAL;
	}
	return Input_read_keyword();
}

// Clear dynamic buffer, then append to it until an illegal (for a keyword)
// character is read. Zero-terminate the string. Return its length (without
// terminator).
// Zero lengths will produce a "missing string" error.
int Input_read_keyword(void)
{
	int	length;

	DYNABUF_CLEAR(GlobalDynaBuf);
	length = Input_append_keyword_to_global_dynabuf();
	// add terminator to buffer (increments buffer's length counter)
	DynaBuf_append(GlobalDynaBuf, '\0');
	return length;
}

// Clear dynamic buffer, then append to it until an illegal (for a keyword)
// character is read. Zero-terminate the string, then convert to lower case.
// Return its length (without terminator).
// Zero lengths will produce a "missing string" error.
int Input_read_and_lower_keyword(void)
{
	int	length;

	DYNABUF_CLEAR(GlobalDynaBuf);
	length = Input_append_keyword_to_global_dynabuf();
	// add terminator to buffer (increments buffer's length counter)
	DynaBuf_append(GlobalDynaBuf, '\0');
	DynaBuf_to_lower(GlobalDynaBuf, GlobalDynaBuf);	// convert to lower case
	return length;
}

// Try to read a file name.
// If "allow_library" is TRUE, library access by using <...> quoting
// is possible as well. If "uses_lib" is non-NULL, info about library
// usage is stored there.
// The file name given in the assembler source code is converted from
// UNIX style to platform style.
// Returns whether error occurred (TRUE on error). Filename in GlobalDynaBuf.
// Errors are handled and reported, but caller should call
// Input_skip_remainder() then.
int Input_read_filename(int allow_library, int *uses_lib)
{
	char	*lib_prefix,
		end_quote;

	DYNABUF_CLEAR(GlobalDynaBuf);
	SKIPSPACE();
	// check for library access
	if (GotByte == '<') {
		if (uses_lib)
			*uses_lib = 1;
		// if library access forbidden, complain
		if (allow_library == FALSE) {
			Throw_error("Writing to library not supported.");
			return TRUE;
		}

		// read platform's lib prefix
		lib_prefix = PLATFORM_LIBPREFIX;
#ifndef NO_NEED_FOR_ENV_VAR
		// if lib prefix not set, complain
		if (lib_prefix == NULL) {
			Throw_error("\"ACME\" environment variable not found.");
			return TRUE;
		}
#endif
		// copy lib path and set quoting char
		DynaBuf_add_string(GlobalDynaBuf, lib_prefix);
		end_quote = '>';
	} else {
		if (uses_lib)
			*uses_lib = 0;
		if (GotByte == '"') {
			end_quote = '"';
		} else {
			Throw_error("File name quotes not found (\"\" or <>).");
			return TRUE;
		}
	}
	// read first character, complain if closing quote
	if (GetQuotedByte() == end_quote) {
		Throw_error("No file name given.");
		return TRUE;
	}

	// read characters until closing quote (or EOS) is reached
	// append platform-converted characters to current string
	while ((GotByte != CHAR_EOS) && (GotByte != end_quote)) {
		DYNABUF_APPEND(GlobalDynaBuf, PLATFORM_CONVERTPATHCHAR(GotByte));
		GetQuotedByte();
	}
	// on error, return
	if (GotByte == CHAR_EOS)
		return TRUE;

	GetByte();	// fetch next to forget closing quote
	// terminate string
	DynaBuf_append(GlobalDynaBuf, '\0');	// add terminator
	return FALSE;	// no error
}

// Try to read a comma, skipping spaces before and after. Return TRUE if comma
// found, otherwise FALSE.
int Input_accept_comma(void)
{
	SKIPSPACE();
	if (GotByte != ',')
		return FALSE;

	NEXTANDSKIPSPACE();
	return TRUE;
}

// read optional info about parameter length
int Input_get_force_bit(void)
{
	char	byte;
	int	force_bit	= 0;

	if (GotByte == '+') {
		byte = GetByte();
		if (byte == '1')
			force_bit = MVALUE_FORCE08;
		else if (byte == '2')
			force_bit = MVALUE_FORCE16;
		else if (byte == '3')
			force_bit = MVALUE_FORCE24;
		if (force_bit)
			GetByte();
		else
			Throw_error("Illegal postfix.");
	}
	SKIPSPACE();
	return force_bit;
}


// include path stuff - should be moved to its own file:

// ring list struct
struct ipi {
	struct ipi	*next,
			*prev;
	const char	*path;
};
static struct ipi	ipi_head;	// head element
static struct dynabuf	*pathbuf;	// buffer to combine search path and file spec

// init list
void includepaths_init(void)
{
	// init ring list
	ipi_head.next = &ipi_head;
	ipi_head.prev = &ipi_head;
	// init dynabuf
	pathbuf = DynaBuf_create(256);
}
// add entry
void includepaths_add(const char *path)
{
	struct ipi	*ipi;

	ipi = safe_malloc(sizeof(*ipi));
	ipi->path = path;
	ipi->next = &ipi_head;
	ipi->prev = ipi_head.prev;
	ipi->next->prev = ipi;
	ipi->prev->next = ipi;
}
// open file for reading (trying list entries as prefixes)
// "uses_lib" tells whether to access library or to make use of include paths
// file name is expected in GlobalDynaBuf
FILE *includepaths_open_ro(int uses_lib)
{
	FILE		*stream;
	struct ipi	*ipi;

	// first try directly, regardless of whether lib or not:
	stream = fopen(GLOBALDYNABUF_CURRENT, FILE_READBINARY);
	// if failed and not lib, try include paths:
	if ((stream == NULL) && !uses_lib) {
		for (ipi = ipi_head.next; ipi != &ipi_head; ipi = ipi->next) {
			DYNABUF_CLEAR(pathbuf);
			// add first part
			DynaBuf_add_string(pathbuf, ipi->path);
			// if wanted and possible, ensure last char is directory separator
			if (DIRECTORY_SEPARATOR
			&& pathbuf->size
			&& (pathbuf->buffer[pathbuf->size - 1] != DIRECTORY_SEPARATOR))
				DynaBuf_append(pathbuf, DIRECTORY_SEPARATOR);
			// add second part
			DynaBuf_add_string(pathbuf, GLOBALDYNABUF_CURRENT);
			// terminate
			DynaBuf_append(pathbuf, '\0');
			// try
			stream = fopen(pathbuf->buffer, FILE_READBINARY);
			//printf("trying <<%s>> - ", pathbuf->buffer);
			if (stream) {
				//printf("ok\n");
				break;
			} else {
				//printf("failed\n");
			}
		}
	}
	if (stream == NULL)
		Throw_error(exception_cannot_open_input_file);
	return stream;
}
