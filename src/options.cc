/*
 * Copyright (C) 2002 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

// 2002-02-10	sft+	added config-existing check in handle_options

#include "widelands.h"
#include "options.h"
#include "config.h"
#include "graphic.h"
#include "input.h"
#include "menuecommon.h"
#include "IntPlayer.h"


Profile g_options(Profile::err_log);


/** write_conf_file()
 *
 * This overrites the old config file with a new one containing
 * the current options
 */
void write_conf_file()
{
	// write only used configuration entries to the file to sieve out typos
	// on the commands line etc..
	g_options.write("config", true);
}

		 
/** show_usage()
 *
 * This functions finally prints the usage and ends the programm
 */
static void show_usage(void)
{
	log(
"Usage: widelands <option0>=<value0> ... <optionN>=<valueN>\n"
"Options:\n"
"\n"
" --<config-entry-name>=value overwrites a config file setting\n"
"\n"
" --help           Show this help\n"
" --version        Show version\n"
"\n"
"Bug reports? Suggestions? Check out the project website:\n"
"  http://www.sourceforge.net/projects/widelands\n"
"Hope you enjoy this game!\n");
}

/** show_version()
 *
 * Print version information
 */
static void show_version(void)
{
	log(VERSION);
}

/** parse_command_line(int argc, char** argv)
 *
 * Parses the standard cmd line of the program
 *
 * Returns: true if execution may continue
 */
static bool parse_command_line(int argc, char** argv)
{
	for(int i = 1; i < argc; i++) {
		char *opt = argv[i];
		char *value;

		if (strncmp(opt, "--", 2)) {
			show_usage();
			return false;
		}
		opt += 2;

		if (!strcmp(opt, "help")) {
			show_usage();
			return false;
		}
		if (!strcmp(opt, "version")) {
			show_version();
			return false;
		}

		value = strchr(opt, '=');
		if (!value) {
			show_usage();
			return false;
		}

		*value++ = 0;

		g_options.get_safe_section("global")->create_val(opt, value);
	}

	return true;
}

/** handle_options(int argc, char** argv)
 *
 * This function parses the config file and the cmdline.
 */
void handle_options(int argc, char **argv)
{
	g_options.read("config", "global");
	
	if (!parse_command_line(argc, argv))
		exit(0);
}
