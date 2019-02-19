// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2017 Marco Baye
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#include "acme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alu.h"
#include "cliargs.h"
#include "config.h"
#include "cpu.h"
#include "dynabuf.h"
#include "encoding.h"
#include "flow.h"
#include "global.h"
#include "input.h"
#include "macro.h"
#include "mnemo.h"
#include "output.h"
#include "platform.h"
#include "pseudoopcodes.h"
#include "section.h"
#include "symbol.h"
#include "version.h"


// constants
static const char	FILE_WRITETEXT[]	= "w";
static const char	FILE_WRITEBINARY[]	= "wb";
// names for error messages
static const char	name_outfile[]		= "output filename";
static const char	arg_symbollist[]	= "symbol list filename";
static const char	arg_reportfile[]	= "report filename";
static const char	arg_vicelabels[]	= "VICE labels filename";
// long options
#define OPTION_HELP		"help"
#define OPTION_FORMAT		"format"
#define OPTION_OUTFILE		"outfile"
#define OPTION_LABELDUMP	"labeldump"	// old
#define OPTION_SYMBOLLIST	"symbollist"	// new
#define OPTION_VICELABELS	"vicelabels"
#define OPTION_REPORT		"report"
#define OPTION_SETPC		"setpc"
#define OPTION_CPU		"cpu"
#define OPTION_INITMEM		"initmem"
#define OPTION_MAXERRORS	"maxerrors"
#define OPTION_MAXDEPTH		"maxdepth"
#define OPTION_USE_STDOUT	"use-stdout"
#define OPTION_VERSION		"version"
#define OPTION_MSVC		"msvc"
#define OPTION_COLOR		"color"
#define OPTION_FULLSTOP		"fullstop"
// options for "-W"
#define OPTIONWNO_LABEL_INDENT	"no-label-indent"
#define OPTIONWNO_OLD_FOR	"no-old-for"
#define OPTIONWTYPE_MISMATCH	"type-mismatch"


// variables
static const char	**toplevel_sources;
static int		toplevel_src_count	= 0;
#define ILLEGAL_START_ADDRESS	(-1)
static signed long	start_address		= ILLEGAL_START_ADDRESS;
static signed long	fill_value		= MEMINIT_USE_DEFAULT;
static const struct cpu_type	*default_cpu	= NULL;
const char		*symbollist_filename	= NULL;
const char		*vicelabels_filename	= NULL;
const char		*output_filename	= NULL;
const char		*report_filename	= NULL;
// maximum recursion depth for macro calls and "!source"
signed long	macro_recursions_left	= MAX_NESTING;
signed long	source_recursions_left	= MAX_NESTING;


// show release and platform info (and exit, if wanted)
static void show_version(int exit_after)
{
	puts(
"This is ACME, release " RELEASE " (\"" CODENAME "\"), " CHANGE_DATE " " CHANGE_YEAR "\n"
"  " PLATFORM_VERSION);
	if (exit_after)
		exit(EXIT_SUCCESS);
}


// show full help (headline, release/platform/version, copyright, dedication,
// warranty disclaimer, usage) and exit program (SUCCESS)
static void show_help_and_exit(void)
{
	puts(
"ACME - the ACME Crossassembler for Multiple Environments\n"
"  Copyright (C) 1998-" CHANGE_YEAR " Marco Baye");
	show_version(FALSE);
	puts(
"ACME comes with ABSOLUTELY NO WARRANTY; for details read the help file.\n"
"  This is free software, and you are welcome to redistribute it under\n"
"  certain conditions; as outlined in the GNU General Public License.\n"
"Dedicated to the wisest being I ever had the pleasure of reading\n"
"  books of (currently spending some time dead for tax reasons).\n"
"The newest version can be found at the ACME homepage:\n"
"  " HOME_PAGE "\n"
"\n"
"Usage:\n"
"acme [OPTION...] [FILE]...\n"
"\n"
"Options:\n"
"  -h, --" OPTION_HELP "             show this help and exit\n"
"  -f, --" OPTION_FORMAT " FORMAT    set output file format\n"
"  -o, --" OPTION_OUTFILE " FILE     set output file name\n"
"  -r, --" OPTION_REPORT " FILE      set report file name\n"
"  -l, --" OPTION_SYMBOLLIST " FILE  set symbol list file name\n"
"      --" OPTION_LABELDUMP "        (old name for --" OPTION_SYMBOLLIST ")\n"
"      --" OPTION_VICELABELS " FILE  set file name for label dump in VICE format\n"
"      --" OPTION_SETPC " NUMBER     set program counter\n"
"      --" OPTION_CPU " CPU          set target processor\n"
"      --" OPTION_INITMEM " NUMBER   define 'empty' memory\n"
"      --" OPTION_MAXERRORS " NUMBER set number of errors before exiting\n"
"      --" OPTION_MAXDEPTH " NUMBER  set recursion depth for macro calls and !src\n"
"  -vDIGIT                set verbosity level\n"
"  -DSYMBOL=VALUE         define global symbol\n"
"  -I PATH/TO/DIR         add search path for input files\n"
// as long as there is only one -W option:
#define OPTIONWNO_LABEL_INDENT	"no-label-indent"
"  -W" OPTIONWNO_LABEL_INDENT "      suppress warnings about indented labels\n"
"  -W" OPTIONWNO_OLD_FOR "           suppress warnings about old \"!for\" syntax\n"
"  -W" OPTIONWTYPE_MISMATCH "        enable type checking (warn about type mismatch)\n"
// when there are more, use next line and add a separate function:
//"  -W                     show warning level options\n"
"      --" OPTION_USE_STDOUT "       fix for 'Relaunch64' IDE (see docs)\n"
"      --" OPTION_MSVC "             output errors in MS VS format\n"
"      --" OPTION_COLOR "            uses ANSI color codes for error output\n"
"      --" OPTION_FULLSTOP "         use '.' as pseudo opcode prefix\n"
PLATFORM_OPTION_HELP
"  -V, --" OPTION_VERSION "          show version and exit\n");
	exit(EXIT_SUCCESS);
}


// initialise report struct
static void report_init(struct report *report)
{
	report->fd = NULL;
	report->asc_used = 0;
	report->bin_used = 0;
	report->last_input = NULL;
}
// open report file
static int report_open(struct report *report, const char *filename)
{
	report->fd = fopen(filename, FILE_WRITETEXT);
	if (report->fd == NULL) {
		fprintf(stderr, "Error: Cannot open report file \"%s\".\n", filename);
		return 1;
	}
	return 0;	// success
}
// close report file
static void report_close(struct report *report)
{
	if (report && report->fd) {
		fclose(report->fd);
		report->fd = NULL;
	}
}


// error handling

// tidy up before exiting by saving symbol list and close other output files
int ACME_finalize(int exit_code)
{
	FILE	*fd;

	report_close(report);
	if (symbollist_filename) {
		fd = fopen(symbollist_filename, FILE_WRITETEXT);	// FIXME - what if filename is given via !sl in sub-dir? fix path!
		if (fd) {
			symbols_list(fd);
			fclose(fd);
			PLATFORM_SETFILETYPE_TEXT(symbollist_filename);
		} else {
			fprintf(stderr, "Error: Cannot open symbol list file \"%s\".\n", symbollist_filename);
			exit_code = EXIT_FAILURE;
		}
	}
	if (vicelabels_filename) {
		fd = fopen(vicelabels_filename, FILE_WRITETEXT);
		if (fd) {
			symbols_vicelabels(fd);
			fclose(fd);
			PLATFORM_SETFILETYPE_TEXT(vicelabels_filename);
		} else {
			fprintf(stderr, "Error: Cannot open VICE label dump file \"%s\".\n", vicelabels_filename);
			exit_code = EXIT_FAILURE;
		}
	}
	return exit_code;
}


// save output file
static void save_output_file(void)
{
	FILE	*fd;

	// if no output file chosen, tell user and do nothing
	if (output_filename == NULL) {
		fputs("No output file specified (use the \"-o\" option or the \"!to\" pseudo opcode).\n", stderr);
		return;
	}
	fd = fopen(output_filename, FILE_WRITEBINARY);	// FIXME - what if filename is given via !to in sub-dir? fix path!
	if (fd == NULL) {
		fprintf(stderr, "Error: Cannot open output file \"%s\".\n",
			output_filename);
		return;
	}
	Output_save_file(fd);
	fclose(fd);
}


// perform a single pass. Returns number of "NeedValue" type errors.
static int perform_pass(void)
{
	FILE	*fd;
	int	ii;

	// call modules' "pass init" functions
	Output_passinit();	// disable output, PC undefined
	cputype_passinit(default_cpu);	// set default cpu type
	// if start address was given on command line, use it:
	if (start_address != ILLEGAL_START_ADDRESS)
		vcpu_set_pc(start_address, 0);
	encoding_passinit();	// set default encoding
	section_passinit();	// set initial zone (untitled)
	// init variables
	pass_undefined_count = 0;	// no "NeedValue" errors yet
	pass_real_errors = 0;	// no real errors yet
	// Process toplevel files
	for (ii = 0; ii < toplevel_src_count; ++ii) {
		if ((fd = fopen(toplevel_sources[ii], FILE_READBINARY))) {
			flow_parse_and_close_file(fd, toplevel_sources[ii]);
		} else {
			fprintf(stderr, "Error: Cannot open toplevel file \"%s\".\n", toplevel_sources[ii]);
			if (toplevel_sources[ii][0] == '-')
				fprintf(stderr, "Options (starting with '-') must be given _before_ source files!\n");
 			++pass_real_errors;
		}
	}
	if (pass_real_errors)
		exit(ACME_finalize(EXIT_FAILURE));
	else
		Output_end_segment();
	return pass_undefined_count;
}


static struct report	global_report;
// do passes until done (or errors occurred). Return whether output is ready.
static int do_actual_work(void)
{
	int		undefined_prev,	// "NeedValue" errors of previous pass
			undefined_curr;	// "NeedValue" errors of current pass

	report = &global_report;	// let global pointer point to something
	report_init(report);	// we must init struct before doing passes
	if (config.process_verbosity > 1)
		puts("First pass.");
	pass_count = 0;
	undefined_curr = perform_pass();	// First pass
	// now pretend there has been a pass before the first one
	undefined_prev = undefined_curr + 1;
	// As long as the number of "NeedValue" errors is decreasing but
	// non-zero, keep doing passes.
	while (undefined_curr && (undefined_curr < undefined_prev)) {
		++pass_count;
		undefined_prev = undefined_curr;
		if (config.process_verbosity > 1)
			puts("Further pass.");
		undefined_curr = perform_pass();
	}
	// any errors left?
	if (undefined_curr == 0) {
		// if listing report is wanted and there were no errors,
		// do another pass to generate listing report
		if (report_filename) {
			if (config.process_verbosity > 1)
				puts("Extra pass to generate listing report.");
			if (report_open(report, report_filename) == 0) {
				++pass_count;
				perform_pass();
				report_close(report);
			}
		}
		return 1;
	}
	// There are still errors (unsolvable by doing further passes),
	// so perform additional pass to find and show them.
	if (config.process_verbosity > 1)
		puts("Extra pass needed to find error.");
	// activate error output
	ALU_optional_notdef_handler = Throw_error;

	++pass_count;
	perform_pass();	// perform pass, but now show "value undefined"
	return 0;
}


// copy string to DynaBuf
static void keyword_to_dynabuf(const char keyword[])
{
	DYNABUF_CLEAR(GlobalDynaBuf);
	DynaBuf_add_string(GlobalDynaBuf, keyword);
	DynaBuf_append(GlobalDynaBuf, '\0');
	DynaBuf_to_lower(GlobalDynaBuf, GlobalDynaBuf);	// convert to lower case
}


// check output format (the output format tree must be set up at this point!)
static void set_output_format(void)
{
	keyword_to_dynabuf(cliargs_safe_get_next("output format"));
	if (outputfile_set_format()) {
		fprintf(stderr, "%sUnknown output format (known formats are: %s).\n", cliargs_error, outputfile_formats);
		exit(EXIT_FAILURE);
	}
}


// check CPU type (the cpu type tree must be set up at this point!)
static void set_starting_cpu(void)
{
	const struct cpu_type	*new_cpu_type;

	keyword_to_dynabuf(cliargs_safe_get_next("CPU type"));
	new_cpu_type = cputype_find();
	if (new_cpu_type) {
		default_cpu = new_cpu_type;
	} else {
		fprintf(stderr, "%sUnknown CPU type (known types are: %s).\n", cliargs_error, cputype_names);
		exit(EXIT_FAILURE);
	}
}


static void could_not_parse(const char strange[])
{
	fprintf(stderr, "%sCould not parse '%s'.\n", cliargs_error, strange);
	exit(EXIT_FAILURE);
}


// return signed long representation of string.
// copes with hexadecimal if prefixed with "$", "0x" or "0X".
// copes with octal if prefixed with "&".
// copes with binary if prefixed with "%".
// assumes decimal otherwise.
static signed long string_to_number(const char *string)
{
	signed long	result;
	char		*end;
	int		base	= 10;

	if (*string == '%') {
		base = 2;
		++string;
	} else if (*string == '&') {
		base = 8;
		++string;
	} else if (*string == '$') {
		base = 16;
		++string;
	} else if ((*string == '0') && ((string[1] == 'x') || (string[1] == 'X'))) {
		base = 16;
		string += 2;
	}
	result = strtol(string, &end, base);
	if (*end)
		could_not_parse(end);
	return result;
}


// set program counter
static void set_starting_pc(void)
{
	start_address = string_to_number(cliargs_safe_get_next("program counter"));
	if ((start_address > -1) && (start_address < 65536))
		return;
	fprintf(stderr, "%sProgram counter out of range (0-0xffff).\n", cliargs_error);
	exit(EXIT_FAILURE);
}


// set initial memory contents
static void set_mem_contents(void)
{
	fill_value = string_to_number(cliargs_safe_get_next("initmem value"));
	if ((fill_value >= -128) && (fill_value <= 255))
		return;
	fprintf(stderr, "%sInitmem value out of range (0-0xff).\n", cliargs_error);
	exit(EXIT_FAILURE);
}


// define symbol
static void define_symbol(const char definition[])
{
	const char	*walk	= definition;
	signed long	value;

	// copy definition to GlobalDynaBuf until '=' reached	
	DYNABUF_CLEAR(GlobalDynaBuf);
	while ((*walk != '=') && (*walk != '\0'))
		DynaBuf_append(GlobalDynaBuf, *walk++);
	if ((*walk == '\0') || (walk[1] == '\0'))
		could_not_parse(definition);
	value =  string_to_number(walk + 1);
	DynaBuf_append(GlobalDynaBuf, '\0');
	symbol_define(value);
}


// handle long options (like "--example"). Return unknown string.
static const char *long_option(const char *string)
{
	if (strcmp(string, OPTION_HELP) == 0)
		show_help_and_exit();
	else if (strcmp(string, OPTION_FORMAT) == 0)
		set_output_format();
	else if (strcmp(string, OPTION_OUTFILE) == 0)
		output_filename = cliargs_safe_get_next(name_outfile);
	else if (strcmp(string, OPTION_LABELDUMP) == 0)	// old
		symbollist_filename = cliargs_safe_get_next(arg_symbollist);
	else if (strcmp(string, OPTION_SYMBOLLIST) == 0)	// new
		symbollist_filename = cliargs_safe_get_next(arg_symbollist);
	else if (strcmp(string, OPTION_VICELABELS) == 0)
		vicelabels_filename = cliargs_safe_get_next(arg_vicelabels);
	else if (strcmp(string, OPTION_REPORT) == 0)
		report_filename = cliargs_safe_get_next(arg_reportfile);
	else if (strcmp(string, OPTION_SETPC) == 0)
		set_starting_pc();
	else if (strcmp(string, OPTION_CPU) == 0)
		set_starting_cpu();
	else if (strcmp(string, OPTION_INITMEM) == 0)
		set_mem_contents();
	else if (strcmp(string, OPTION_MAXERRORS) == 0)
		config.max_errors = string_to_number(cliargs_safe_get_next("maximum error count"));
	else if (strcmp(string, OPTION_MAXDEPTH) == 0)
		macro_recursions_left = (source_recursions_left = string_to_number(cliargs_safe_get_next("recursion depth")));
//	else if (strcmp(string, "strictsyntax") == 0)
//		strict_syntax = TRUE;
	else if (strcmp(string, OPTION_USE_STDOUT) == 0)
		msg_stream = stdout;
	else if (strcmp(string, OPTION_MSVC) == 0)
		config.format_msvc = TRUE;
	else if (strcmp(string, OPTION_FULLSTOP) == 0)
		config.pseudoop_prefix = '.';
	PLATFORM_LONGOPTION_CODE
	else if (strcmp(string, OPTION_COLOR) == 0)
		config.format_color = TRUE;
	else if (strcmp(string, OPTION_VERSION) == 0)
		show_version(TRUE);
	else
		return string;
	return NULL;
}


// handle short options (like "-e"). Return unknown character.
static char short_option(const char *argument)
{
	while (*argument) {
		switch (*argument) {
		case 'D':	// "-D" define constants
			define_symbol(argument + 1);
			goto done;
		case 'f':	// "-f" selects output format
			set_output_format();
			break;
		case 'h':	// "-h" shows help
			show_help_and_exit();
			break;
		case 'I':	// "-I" adds an include directory
			if (argument[1])
				includepaths_add(argument + 1);
			else
				includepaths_add(cliargs_safe_get_next("include path"));
			goto done;
		case 'l':	// "-l" selects symbol list filename
			symbollist_filename = cliargs_safe_get_next(arg_symbollist);
			break;
		case 'o':	// "-o" selects output filename
			output_filename = cliargs_safe_get_next(name_outfile);
			break;
		case 'r':	// "-r" selects report filename
			report_filename = cliargs_safe_get_next(arg_reportfile);
			break;
		case 'v':	// "-v" changes verbosity
			++config.process_verbosity;
			if ((argument[1] >= '0') && (argument[1] <= '9'))
				config.process_verbosity = *(++argument) - '0';
			break;
		// platform specific switches are inserted here
			PLATFORM_SHORTOPTION_CODE
		case 'V':	// "-V" shows version
			show_version(TRUE);
			break;
		case 'W':	// "-W" tunes warning level
			if (strcmp(argument + 1, OPTIONWNO_LABEL_INDENT) == 0) {
				config.warn_on_indented_labels = FALSE;
				goto done;
			} else if (strcmp(argument + 1, OPTIONWNO_OLD_FOR) == 0) {
				config.warn_on_old_for = FALSE;
				goto done;
			} else if (strcmp(argument + 1, OPTIONWTYPE_MISMATCH) == 0) {
				config.warn_on_type_mismatch = TRUE;
				goto done;
			} else {
				fprintf(stderr, "%sUnknown warning level.\n", cliargs_error);
				exit(EXIT_FAILURE);
			}
			break;
		default:	// unknown ones: program termination
			return *argument;
		}
		++argument;
	}
done:
	return '\0';
}


// guess what
int main(int argc, const char *argv[])
{
	config_default(&config);
	// if called without any arguments, show usage info (not full help)
	if (argc == 1)
		show_help_and_exit();
	msg_stream = stderr;
	cliargs_init(argc, argv);
	DynaBuf_init();	// inits *global* dynamic buffer - important, so first
	// Init platform-specific stuff.
	// For example, this could read the library path from an
	// environment variable, which in turn may need DynaBuf already.
	PLATFORM_INIT;
	// prepare a buffer large enough to hold pointers to "-D" switch values
//	cli_defines = safe_malloc(argc * sizeof(*cli_defines));
	includepaths_init();	// must be done before cli arg handling
	// handle command line arguments
	cliargs_handle_options(short_option, long_option);
	// generate list of files to process
	cliargs_get_rest(&toplevel_src_count, &toplevel_sources, "No top level sources given");
	// Init modules (most of them will just build keyword trees)
	ALU_init();
	Macro_init();
	Mnemo_init();
	Output_init(fill_value);
	pseudoopcodes_init();	// setup keyword tree for pseudo opcodes
	if (do_actual_work())
		save_output_file();
	return ACME_finalize(EXIT_SUCCESS);	// dump labels, if wanted
}
