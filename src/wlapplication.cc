/*
 * Copyright (C) 2006 by the Widelands Development Team
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

#include "constants.h"
#include "filesystem.h"
#include <iostream>
#include "network_ggz.h"
#include "profile.h"
#include "sound_handler.h"
#include <string>
#include "system.h" //only for init_double_game
#include "wlapplication.h"

/**
 * The main loop. Plain and Simple.
 *
 * Push the first event on the event queue, then keep dispatching events until
 * it is time to quit.
 */
void WLApplication::run()
{
}

/**
 * Initialize everything that needs to be set up before starting the main loop
 * (and consequently, as the loop's starting point, the main menu)
 *
 * \param argc The number of command line arguments
 * \param argv Array of command line arguments
 * \return Whether the initalization was successful
 */
bool WLApplication::init(int argc, char **argv)
{
	//create the filesystem abstraction
	//must be first - we wouldn't even find the config file
	g_fs = LayeredFileSystem::Create();
	setup_searchpaths(argc, argv);

	//read in the configuration file
	g_options.read("config", "global");

	//then parse the commandline - overwrites conffile settings
	if (!parse_command_line(argc, argv)) {
		return false;
	}

	Section *s = g_options.pull_section("global");
	// Set Locale and grab default domain
	Sys_SetLocale( s->get_string( "language" ));
	Sys_GrabTextdomain("widelands");

	//TODO: open record or playback file

	init_hardware();

	return true;
}

/**
 * Shut down all subsystems in an orderly manner
 */
void WLApplication::shutdown()
{
	//Use the opposite order of WLApplication::init()

	shutdown_hardware();

	//TODO: close record or playback file

	// To be proper, release our textdomain
	Sys_ReleaseTextdomain();

	// overwrite the old config file
	g_options.write("config", true);

	assert(g_fs!=0);
	delete g_fs;
	g_fs = 0;
}

/**
 * Print usage information
 */
void WLApplication::show_usage()
{
	//TODO: i18n this whole block
	std::cout<<"This is Widelands-"<<VERSION<<std::endl<<std::endl
	<<"Usage: widelands <option0>=<value0> ... <optionN>=<valueN>"<<std::endl
	<<std::endl
	<<"Options:"<<std::endl
	<<std::endl
	<<" --<config-entry-name>=value overwrites any config file setting"<<std::endl
	<<std::endl
	<<" --record=FILENAME         Record all events to the given filename for later playback"<<std::endl
	<<" --playback=FILENAME       Playback given filename (see --record)"<<std::endl
	<<std::endl
	<<" --coredump=[yes|no]       Generates a core dump on segfaults instead of using the SDL"<<std::endl
	<<std::endl
	<<" --ggz                     Starts game as GGZ Gaming Zone client (don't use!)"<<std::endl
	<<" --nosound                 Starts the game with sound disabled"<<std::endl
	<<" --nozip                   Do not save files as binary zip archives."<<std::endl
	<<std::endl
#ifdef DEBUG
#ifndef __WIN32__
	<<" --double                  Start the game twice (for localhost network testing)"<<std::endl
	<<std::endl
#endif
#endif
	<<" --help                    Show this help"<<std::endl
	<<std::endl
	<<"Bug reports? Suggestions? Check out the project website:"<<std::endl
	<<"  http://www.sourceforge.net/projects/widelands"<<std::endl
	<<"Hope you enjoy this game!"<<std::endl;
}

/**
 * Parse the standard cmd line of the program
 *
 * \return true if no errors, otherwise false
 *
 * \todo We don't support single-dash-options. Shouldn't we do so at least for
 * --help ?
 */
bool WLApplication::parse_command_line(int argc, char** argv)
{
	for(int i = 1; i < argc; i++) {
		std::string opt=argv[i];
		std::string value="";

		//special case for help because it allows single-dash-options
		if ((opt=="-h" || opt=="-help") ||
		    (opt=="help" || opt=="usage") ||
		    (opt=="-V" || opt=="--version"))
		{
			show_usage();
			return false;
		}

		//Are we looking at an option at all?
		if (opt.substr(0,2)!="--") {
			show_usage();
			return false;
		}

		//Yes, it is an option. Remove the leading "--"
		opt.erase(0,2);

		//Handle parameter-less options first
		if (opt=="ggz") {
			NetGGZ::ref()->init();
			continue;
		}
		if (opt=="nosound") {
			g_sound_handler.m_nosound=true;
			continue;
		}
		if (opt=="nozip") {
			g_options.pull_section("global")->create_val("nozip","true");
			continue;
		}

#ifdef DEBUG
#ifndef __WIN32__
		if (opt=="double") {
			init_double_game();
			continue;
		}
#endif // __WIN32__
#endif // DEBUG

		//OK, we're done with simple options

		//if opt doesn't contain "=" then we're looking at broken
		//command line (or an unhandled param-less option)
		SSS_T pos=opt.find("=");
		if (pos==0) {
			std::cout<<std::endl
			<<"ERROR: invalid option: --"<<opt<<std::endl<<std::endl;

			show_usage();
			return false;
		}

		//Extract the option value
		value=opt.substr(pos+1, opt.size()-pos);

		//Extract the option name
		opt.erase(pos, opt.size()-pos);

		//Note: value might still be an empty string!
		//This is valid because conffile params might be overridden to
		//be empty

		//Note: it should be possible to record and playback at the same time,
		//but why would you?
		if (opt=="record") {
			if (value.empty()) {
				std::cout<<std::endl
				<<"ERROR: --record needs a filename!"<<std::endl<<std::endl;
				show_usage();
				return false;
			}

			char expanded_filename[1024];

			//this bypasses the layered filesystem on purpose!
			FS_CanonicalizeName(expanded_filename, 1024, value.c_str());
			snprintf(sys.recordname, sizeof(sys.recordname), "%s", expanded_filename);
			return false;
		}

		//Note: it should be possible to record and playback at the same time,
		//but why would you?
		if (opt=="playback") {
			if (value.empty()) {
				std::cout<<std::endl
				<<"ERROR: --playback needs a filename!"<<std::endl<<std::endl;
				show_usage();
				return false;
			}

			char expanded_filename[1024];

			//this bypasses the layered filesystem on purpose!
			FS_CanonicalizeName(expanded_filename, 1024, value.c_str());
			snprintf(sys.playbackname, sizeof(sys.playbackname), "%s", expanded_filename);
			continue;
		}

		//TODO: barf here on unkown option

		//If it hasn't been handled yet it's probably an attempt to
		//override a conffile setting
		//With typos, this will create invalid config settings. They
		//will be taken care of (==ignored) when saving the options
		g_options.pull_section("global")->create_val(opt.c_str(),
		                                             value.c_str());

	}

	return true;
}

/**
 * Start the hardware: switch to graphics mode, start sound handler
 *
 * \return true if there were no fatal errors that prevent the game from running,
 *         false otherwise
 */
bool WLApplication::init_hardware()
{
	Uint32 sdl_flags=0;
	Section *s = g_options.pull_section("global");

	//Start the SDL core
	if(s->get_bool("coredump", false))
		sdl_flags=SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;
	else
		sdl_flags=SDL_INIT_VIDEO;

	if (SDL_Init(sdl_flags) == -1) {
		//TODO: that's not handled yet!
		throw wexception("Failed to initialize SDL: %s", SDL_GetError());
	} else
		sys.sdl_active=true;

	SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableUNICODE(1); // useful for e.g. chat messages
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	Sys_InitGraphics(640, 480, s->get_int("depth",16), s->get_bool("fullscreen", false));

	// Start the audio subsystem
	// must know the locale before calling this!
	g_sound_handler.init();

	return true;
}

/**
 * Shut the hardware down: switch graphics mode, stop sound handler
 */
void WLApplication::shutdown_hardware()
{
	g_sound_handler.shutdown();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	Sys_InitGraphics(0, 0, 0, false);

	SDL_Quit();
	sys.sdl_active = false;
}
