/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#include "constants.h"
#include "error.h"
#include "options.h"
#include "system.h"

Profile g_options(Profile::err_log);


/** options_shutdown()
 *
 * This overrites the old config file with a new one containing
 * the current options
 */
void options_shutdown()
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
" --record         Record all events to the given filename for later playback\n"
" --playback       Playback given filename (see --record)\n"
"\n"
" --coredump       Generates a core dump on segfaults instead of using the SDL\n"
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

		if (!strcmp(opt, "record")) {
			Sys_SetRecordFile(value);
			continue;
		}
		if (!strcmp(opt, "playback")) {
			Sys_SetPlaybackFile(value);
			continue;
		}

		g_options.pull_section("global")->create_val(opt, value);
	}

	return true;
}

/** options_init(int argc, char** argv)
 *
 * This function parses the config file and the cmdline.
 */
void options_init(int argc, char **argv)
{
	g_options.read("config", "global");

	if (!parse_command_line(argc, argv))
		exit(0);
}
