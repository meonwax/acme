// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2015 Marco Baye
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "acme.h"
#include "io.h"
#include "platform.h"
#include "version.h"


// guess what
int main(int argc, char *argv[])
{
	// handle "toacme -h" and "toacme --help" just like "toacme"
	if (argc == 2) {
		if ((strcmp(argv[1], "-h") == 0)
		|| (strcmp(argv[1], "--help") == 0))
			argc = 1;
	}
	// "toacme" without any switches gives info and exits successfully
	if (argc == 1) {
		version_show_info(argv[0]);
		return EXIT_SUCCESS;
	}
	// check argument count
	if (argc != 4) {
		fputs("Wrong number of arguments.\n", stderr);
		return EXIT_FAILURE;
	}
	// check format id
	if (version_parse_id(argv[1])) {
		fputs("Unknown format id.\n", stderr);
		return EXIT_FAILURE;
	}
	// be nice and ensure input and output are different
	if (strcmp(argv[2], argv[3]) == 0) {
		fputs("Input and output files must be different.\n", stderr);
		return EXIT_FAILURE;
	}
	// try to open input file
	global_input_stream = fopen(argv[2], "rb");
	if (global_input_stream == NULL) {
		fputs("Cannot open input file.\n", stderr);
		return EXIT_FAILURE;
	}
	// try to open output file
	global_output_stream = fopen(argv[3], "w");
	if (global_output_stream == NULL) {
		fputs("Cannot open output file.\n", stderr);
		return EXIT_FAILURE;
	}
	// do the actual work
	version_main();
	// and then tidy up and exit
	fclose(global_output_stream);
	PLATFORM_SETFILETYPE_TEXT(argv[3]);
	fclose(global_input_stream);
	return EXIT_SUCCESS;
}
