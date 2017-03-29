// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// CLI argument stuff
#ifndef cliargs_H
#define cliargs_H


// constants
extern const char	cliargs_error[];


// handle options. Call fn_short for short options, fn_long for long ones.
extern void cliargs_handle_options(char (*fn_short)(const char *), const char *(*fn_long)(const char *));
// return next argument.
extern const char *cliargs_get_next(void);
// return next argument. If none left, complain with given name.
extern const char *cliargs_safe_get_next(const char name[]);
// initialise argument handler. Returns program name (argv[0]).
extern const char *cliargs_init(int argc, const char *argv[]);
// get unhandled args. If none left, complain with given error message.
extern void cliargs_get_rest(int *argc, const char ***argv, const char error[]);


#endif
