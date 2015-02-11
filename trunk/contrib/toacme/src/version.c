// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// Version
//
#define RELEASE_NUMBER	"0.8"		// change before release (FIXME)
#define CHANGE_DATE	"16 Jun"	// change before release
#define CHANGE_YEAR	"2005"		// change before release
#define HOME_PAGE	"http://home.pages.de/~mac_bacon/smorbrod/acme/"


// Includes
//
#include <stdio.h>
#include <string.h>
#include "io.h"
#include "platform.h"


// Variables
//
void	(*client_main)(void)	= NULL;


// Functions
//

// show version info and usage
//
void version_show_info(const char program_name[]) {
	printf(
"\n"
"ToACME - converts other assemblers' source codes to ACME format.\n"
"Release "RELEASE_NUMBER" ("CHANGE_DATE" "CHANGE_YEAR"), Copyright (C) 1999-"CHANGE_YEAR" Marco Baye.\n"
PLATFORM_VERSION"\n"
"Fixes for AssBlaster macro conversion done by "STEFAN".\n"
"Token table for Giga-Assembler supplied by Andreas Paul.\n"
"\n"
"The newest version can be found at the ACME homepage:\n"
HOME_PAGE"\n"
"\n"
"ToACME comes with ABSOLUTELY NO WARRANTY; for details read the help file.\n"
"This is free software, and you are welcome to redistribute it under\n"
"certain conditions; as outlined in the GNU General Public License.\n"
"\n"
"Syntax: %s <format_id> <input_file> <output_file>\n"
"\n"
"format_id   source file format             quality\n"
"--------------------------------------------------\n"
"ab3         C64: AssBlaster 3.0 to 3.2        good\n"
"f8ab        C64: Flash8-AssBlaster              ok\n"
"giga        C64: Giga-Assembler      needs testing\n"
"hypra       C64: Hypra-Assembler                ok\n"
"object      object code files                 poor\n"
"\n"
	, program_name);
}

// Check id string. Returns whether illegal.
//
extern void ab3_main(void);
extern void f8ab_main(void);
extern void giga_main(void);
extern void hypra_main(void);
extern void obj_main(void);
int version_parse_id(const char id[]) {

	if(strcmp(id, "ab3") == NULL)
		client_main = ab3_main;
	else if(strcmp(id, "f8ab") == NULL)
		client_main = f8ab_main;
	else if(strcmp(id, "giga") == NULL)
		client_main = giga_main;
	else if(strcmp(id, "hypra") == NULL)
		client_main = hypra_main;
	else if(strcmp(id, "object") == NULL)
		client_main = obj_main;
	return(client_main ? 0 : 1);
}

// do the actual work
//
void version_main(void) {
	fputs(
";ACME 0.85\n"
"; ToACME: Converted by ToACME, release " RELEASE_NUMBER ".\n"
	, global_output_stream);
	client_main();
}
