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

#include "widelands.h"
#include "ui.h"
#include "options.h"
#include "input.h"
#include "cursor.h"
#include "mainmenue.h"
#include "setup.h"
#include "font.h"
#include "IntPlayer.h"

#include <SDL.h>


LayeredFileSystem *g_fs;


/** g_main function 
 * This is the OS Independant main function.
 * 
 * It makes sure, Options and Commandline is parsed,
 * initializes Graphics and Resource Handler
 *
 * return Exitcode of App
 */
//#include "md5file.h"
int g_main(int argc, char** argv)
{
	// Create all the subsystems
	static Graphic g;
	static Input myip;
	static Font_Handler f;
	static Cursor cur;
	static User_Interface ui;

	try
	{
	   // Setup filesystem
		g_fs = LayeredFileSystem::Create();
		setup_searchpaths(argc, argv);

		// Handle options
		handle_options(argc, argv);

		// Initialize all subsystems after config has been read
		Section *s = g_options.get_safe_section("global");
		Interactive_Player::set_resolution(s->get_int("xres", 640), s->get_int("yres", 480));
		
		AutoPic::load_all();

		setup_fonthandler();
		setup_ui();

		g_ip.init();
		g_gr.init();

		// complain about unknown options in the configuration file and on the 
		// command line
		g_options.check_used();

		// run main_menue
		main_menue();

		// save options
		write_conf_file();
		
		delete g_fs;
	}
	catch(std::exception &e)
	{
		cerr << "Unhandled exception: " << e.what() << endl;
	}
	catch(...)
	{
		cerr << "Unhandled exception" << endl;
	}
	
	return RET_OK;
}

// ** unix, win32 console *****************************************************
#if !defined(WIN32) || (defined(WIN32) && defined(_CONSOLE))

int main(int argc, char** argv)
{
	return g_main(argc, argv);
}

// ** win32 gui ***************************************************************
#else // WIN32, !_CONSOLE

static int ParseCommandLine(char* cmdline, char** argv)
{
	char* bufp = cmdline;
	int argc = 0;

	while (*bufp)
	{
		// skip leading whitespace
		while (isspace(*bufp))
			++bufp;
		// skip arg
		if (*bufp == '"')
		{
			++bufp;
			if (*bufp)
			{
				if (argv)
					argv[argc] = bufp;
				++argc;
			}
			// skip
			while (*bufp && (*bufp != '"'))
				++bufp;
		}
		else
		{
			if (*bufp)
			{
				if (argv)
					argv[argc] = bufp;
				++argc;
			}
			// skip
			while (*bufp && ! isspace(*bufp))
				++bufp;
		}
		if (*bufp)
		{
			if (argv)
				*bufp = '\0';
			++bufp;
		}
	}
	if (argv)
		argv[argc] = NULL;
	return argc;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR /*cmdLine*/, int)
{
	// param cmdLine and GetCommandLine() are NOT the same
	char* cmdLine = new char[strlen(GetCommandLine()) + 1];
	strcpy(cmdLine, GetCommandLine());

	char** args = new char*[ParseCommandLine(cmdLine, NULL)+1];
	int argcount = ParseCommandLine(cmdLine, args);

	int ret =  g_main(argcount, args);

	delete[] args;
	delete[] cmdLine;

	return ret;
}

#endif
