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
#include "cursor.h"
#include "mainmenue.h"
#include "setup.h"
#include "font.h"
#include "intro.h"

#include <SDL.h>


LayeredFileSystem *g_fs;


static void g_shutdown();

/*
===============
g_init

Initialize all subsystems
===============
*/
static void g_init(int argc, char **argv)
{
	try
	{
		// Create filesystem
		g_fs = LayeredFileSystem::Create();
		setup_searchpaths(argc, argv);

		// Handle options
		options_init(argc, argv);

		// Create all subsystems after config has been read
		Sys_Init();

		static Cursor cur;

		AutoPic::load_all();
		
		g_font = Font::load("fixed_font1");
		setup_ui();
		
		// Initialize graphics last
		Section *s = g_options.pull_section("global");
		
		Sys_InitGraphics(GFXSYS_SW16, 640, 480, s->get_bool("fullscreen", false));
		
		// complain about unknown options in the configuration file and on the 
		// command line
		
		// KLUDGE!
		// Without this, xres and yres get dropped by check_used().
		// Profile needs support for a Syntax definition to solve this in a sensible way
		s->get_string("xres");
		s->get_string("yres");
		// KLUDGE!
		
		g_options.check_used();
	}
	catch(std::exception &e) {
		critical_error("Initialization error: %s", e.what());
		g_shutdown();
		exit(-1);
	}
}

/*
===============
g_shutdown

Shutdown all subsystems
===============
*/
static void g_shutdown()
{
	// Shutdown subsystems
	Sys_InitGraphics(GFXSYS_NONE, 0, 0, false);
	
	if (g_font) {
		g_font->release();
		g_font = 0;
	}
	
	Sys_Shutdown();
	
	// Save options
	options_shutdown();
	
	// Destroy filesystem
	if (g_fs) {
		delete g_fs;
		g_fs = 0;
	}
}

/*
===============
g_main

This is the OS Independant main function.

Control the life-cycle of the game
===============
*/
void g_main(int argc, char** argv)
{
	try
	{
		g_init(argc, argv);

		try {
			intro();
         main_menue();
		} catch(std::exception &e) {
			critical_error("Unhandled exception: %s", e.what());
		}

		g_shutdown();
	}
	catch(std::exception &e) {
		g_gr = 0; // paranoia
		critical_error("Unhandled exception: %s", e.what());
	}
	catch(...) {
		g_gr = 0;
		critical_error("Unhandled exception");
	}
}

// ** unix, win32 console *****************************************************
#if !defined(WIN32) || (defined(WIN32) && defined(_CONSOLE)) || (defined(WIN32) && defined(__GNUC__))

int main(int argc, char** argv)
{
	g_main(argc, argv);
	return 0;
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

	g_main(argcount, args);

	delete[] args;
	delete[] cmdLine;

	return 0;
}

#endif
