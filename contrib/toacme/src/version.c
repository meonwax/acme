// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2015 Marco Baye
// Have a look at "main.c" for further info
//
// Version

#define RELEASE_NUMBER	"0.14"		// change before release (FIXME)
#define CHANGE_DATE	"19 Feb"		// change before release
#define CHANGE_YEAR	"2017"		// change before release
#define HOME_PAGE	"http://sourceforge.net/projects/acme-crossass/"
//			"http://home.pages.de/~mac_bacon/smorbrod/acme/"
#define FILE_TAG	";ACME 0.96.1"	// check before release

#include <stdio.h>
#include <string.h>
#include "io.h"
#include "platform.h"


// variables
void	(*client_main)(void)	= NULL;


// functions

// show version info and usage
void version_show_info(const char program_name[]) {
	printf(
"\n"
"ToACME - converts other assemblers' source codes to ACME format.\n"
"Release " RELEASE_NUMBER " (" CHANGE_DATE " " CHANGE_YEAR "), Copyright (C) 1999-" CHANGE_YEAR " Marco Baye.\n"
PLATFORM_VERSION "\n"
"Thanks to Stefan Hübner for fixing the AssBlaster macro conversion code.\n"
"Thanks to Andreas Paul for helping with the Giga-Assembler mode.\n"
"Thanks to Arndt Dettke for helping with the Hypra-Assembler mode.\n"
"Thanks to Hoogo for helping with the Professional Assembler mode.\n"
"\n"
"The newest version can be found at the ACME homepage:\n"
HOME_PAGE "\n"
"\n"
"ToACME comes with ABSOLUTELY NO WARRANTY; for details read the help file.\n"
"This is free software, and you are welcome to redistribute it under\n"
"certain conditions; as outlined in the GNU General Public License.\n"
"\n"
"Syntax: %s  FORMAT_ID  INPUT_FILE  OUTPUT_FILE\n"
"\n"
"Format ID:  source file format             quality\n"
"--------------------------------------------------\n"
"object      object code files                 poor\n"
"hypra       C64: Hypra-Assembler                ok\n"
"giga        C64: Giga-Assembler                 ok\n"
"vis         C64: VisAss                   untested\n"
"ab3         C64: AssBlaster 3.0 to 3.2        good\n"
"f8ab        C64: Flash8-AssBlaster              ok\n"
"prof        C64: Professional Assembler       poor (work in progress)\n"
"\n"
	, program_name);
}


extern void visass_main(void);
extern void ab3_main(void);
extern void f8ab_main(void);
extern void giga_main(void);
extern void hypra_main(void);
extern void obj_main(void);
extern void prof_main(void);


// check id string. returns whether illegal.
int version_parse_id(const char id[])
{
	if (strcmp(id, "vis") == 0)
		client_main = visass_main;
	else if (strcmp(id, "ab3") == 0)
		client_main = ab3_main;
	else if (strcmp(id, "f8ab") == 0)
		client_main = f8ab_main;
	else if (strcmp(id, "giga") == 0)
		client_main = giga_main;
	else if (strcmp(id, "hypra") == 0)
		client_main = hypra_main;
	else if (strcmp(id, "object") == 0)
		client_main = obj_main;
	else if (strcmp(id, "prof") == 0)
		client_main = prof_main;
	return client_main ? 0 : 1;
}


// do the actual work
void version_main(void)
{
	IO_put_string(
FILE_TAG "\n"
"; ToACME: Converted by ToACME, release " RELEASE_NUMBER ".\n"
	);
	client_main();
}
