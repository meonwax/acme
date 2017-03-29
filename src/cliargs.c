// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// CLI argument stuff
#include "cliargs.h"
#include <stdlib.h>
#include <stdio.h>
#include "config.h"


// constants
const char	cliargs_error[]	= "Error in CLI arguments: ";


// variables
static int		arguments_left;		// number of CLI arguments left
static const char	**next_argument;	// next argument pointer


// return pointer to next command line argument (NULL if no more)
const char *cliargs_get_next(void)
{
	if (arguments_left == 0)
		return NULL;

	--arguments_left;
	return *next_argument++;
}


// parse command line switches
void cliargs_handle_options(char (*fn_short)(const char *), const char *(*fn_long)(const char *))
{
	const char	*problem_string,
			*argument;
	char		problem_char;

	for (;;) {
		// if there are no more arguments, return immediately
		if (arguments_left == 0)
			return;

		// if next argument is not an option, return immediately
		if ((**next_argument) != '-')
			return;

		// officially fetch argument. We already know the
		// first character is a '-', so advance pointer.
		argument = cliargs_get_next() + 1;
		// Check for "--"
		if (*argument == '-') {
			// long argument
			if (argument[1] == '\0')
				return;	// when encountering "--", return

			problem_string = fn_long(argument + 1);
			if (problem_string) {
				fprintf(stderr, "%sUnknown option (--%s).\n", cliargs_error, problem_string);
				exit(EXIT_FAILURE);
			}
		} else {
			problem_char = fn_short(argument);
			if (problem_char) {
				fprintf(stderr, "%sUnknown switch (-%c).\n", cliargs_error, problem_char);
				exit(EXIT_FAILURE);
			}
		}
	}
}


// return next arg. If there is none, complain and exit
const char *cliargs_safe_get_next(const char name[])
{
	const char	*string;

	string = cliargs_get_next();
	if (string)
		return string;

	fprintf(stderr, "%sMissing %s.\n", cliargs_error, name);
	exit(EXIT_FAILURE);
}


// init command line handling stuff
const char *cliargs_init(int argc, const char *argv[])
{
	arguments_left = argc;
	next_argument = argv;
	return cliargs_get_next();
}


// return unhandled (non-option) arguments. Complains if none.
void cliargs_get_rest(int *argc, const char ***argv, const char error[])
{
	*argc = arguments_left;
	*argv = next_argument;
	if (error && (arguments_left == 0)) {
		fprintf(stderr, "%s%s.\n", cliargs_error, error);
		exit(EXIT_FAILURE);
	}
}
