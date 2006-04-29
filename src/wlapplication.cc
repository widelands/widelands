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
#include "error.h"
#include "filesystem.h"
#include "font_handler.h"
#include <iostream>
#include "network_ggz.h"
#include "profile.h"
#include "sound_handler.h"
#include <string>
#include "system.h" //only for init_double_game
#include "wlapplication.h"

WLApplication *WLApplication::the_singleton=0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Simple wrapper functions to make stdio file access less painful
 *
 * These will vanish when IO handling gets moved to C++ streams
*/
//@{
void write_record_char(char v)
{
	assert(WLApplication::get()->get_rec_file());

	if (fwrite(&v, sizeof(v), 1, WLApplication::get()->get_rec_file()) != 1)
		throw wexception("Write of 1 byte to record failed.");
	fflush(WLApplication::get()->get_rec_file());
}

char read_record_char()
{
	char v;

	assert(WLApplication::get()->get_play_file());

	if (fread(&v, sizeof(v), 1, WLApplication::get()->get_play_file()) != 1)
		throw wexception("Read of 1 byte from record failed.");

	return v;
}

void write_record_int(int v)
{
	assert(WLApplication::get()->get_rec_file());

	v = Little32(v);
	if (fwrite(&v, sizeof(v), 1, WLApplication::get()->get_rec_file()) != 1)
		throw wexception("Write of 4 bytes to record failed.");
	fflush(WLApplication::get()->get_rec_file());
}

int read_record_int()
{
	int v;

	assert(WLApplication::get()->get_play_file());

	if (fread(&v, sizeof(v), 1, WLApplication::get()->get_play_file()) != 1)
		throw wexception("Read of 4 bytes from record failed.");

	return Little32(v);
}

void write_record_code(uchar code)
{
	write_record_char(code);
}

void read_record_code(uchar code)
{
	uchar filecode;

	filecode = read_record_char();

	if (filecode != code)
		throw wexception("%08X: Bad code %02X during playback (%02X expected). Mismatching executable versions?",
				 WLApplication::get()->get_playback_offset()-1, filecode, code);
}
//@}

/*
Notes on the implementation
---------------------------

Mouse:
When in GrabInput mode (default), the system and SDL mouse cursor is not
connected to the internal mouse position. We rely on SDL to provide the correct
relative movement information even when the mouse cursor is close to the window
border. We don't not use the absolute mouse position provided by SDL at all.
The internal mouse position is kept with sub-pixel accuracy to make mouse speed
work.

When GrabInput mode is off
*/

/**
 * The main entry point for the WLApplication singleton.
 *
 * Regardless of circumstances, this will return the one and only valid
 * WLApplication object when called. If neccessary, a new WLApplication instance
 * is created.
 *
 * While you \e can do the first call without parameters, but it does not make
 * much sense.
 *
 * \param argc The number of command line arguments
 * \param argv Array of command line arguments
 * \return An (always valid!) pointer to the WLApplication singleton
 */
WLApplication *WLApplication::get(int argc, char **argv)
{
	if (the_singleton==0) {
		the_singleton=new WLApplication(argc, argv);
	}

	return the_singleton;
}

/**
 * Initialize an instance of WLApplication.
 *
 * This constructor is protected \e on \e purpose !
 * Use \ref WLApplication::get() instead.
 *
 * \param argc The number of command line arguments
 * \param argv Array of command line arguments
 */
WLApplication::WLApplication(int argc, char **argv):m_argc(argc),m_argv(argv)
{}


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
 * \return Whether the initalization was successful
 */
bool WLApplication::init()
{
	//create the filesystem abstraction
	//must be first - we wouldn't even find the config file
	g_fs = LayeredFileSystem::Create();
	setup_searchpaths(m_argc, m_argv);

	g_fh = new Font_Handler();

	//load config file and parse command line settings
	init_settings();
	init_hardware();
	init_recordplaybackfile();

	return true;
}

/**
 * Shut down all subsystems in an orderly manner
 */
void WLApplication::shutdown()
{
	//Use the opposite order of WLApplication::init()

	shutdown_recordplaybackfile();
	shutdown_hardware();
	shutdown_settings();

	assert(g_fh);
	delete g_fh;
	g_fh = 0;

	assert(g_fs!=0);
	delete g_fs;
	g_fs = 0;
}

/**
 * Grab a given TextDomain. We keep a stack
 * if a new one is grabbed, it is pushed on the stack.
 * On release, it is dropped and the previous
 * one is re-grabbed instead.
 *
 * So when a tribe loads, it grabs it's textdomain
 * loads all data and releases it -> we're back in
 * widelands domain. Negative: We can't translate error
 * messages. Who cares?
 */
void WLApplication::grab_textdomain( const char* domain)
{
	bind_textdomain_codeset (domain, "UTF-8");
	bindtextdomain( domain, LOCALE_PATH );
	textdomain(domain);

	m_textdomains.push_back( domain );
}

/**
 * See \ref grab_textdomain()
 */
void WLApplication::release_textdomain()
{
	m_textdomains.pop_back();

	//don't try to get the previous TD when the very first one ('widelands') just got dropped
	if (m_textdomains.size()>0) {
		const char* domain = m_textdomains.back().c_str();
		bind_textdomain_codeset (domain, "UTF-8");
		bindtextdomain( domain, LOCALE_PATH );
		textdomain(domain);
	}
}

/**
 * Set The locale to the given string
 */
void WLApplication::set_locale( const char* str ) {
	if( !str )
		str = "";

	// Somehow setlocale doesn't behave same on
	// some systems.
#ifdef __BEOS__
	setenv ("LANG", str, 1);
	setenv ("LC_ALL", str, 1);
#endif
#ifdef __APPLE__
	setenv ("LANGUAGE", str, 1);
	setenv ("LC_ALL", str, 1);
#endif

#ifdef _WIN32
	const std::string env = std::string("LANG=") + str;
	putenv(env.c_str());
#endif

	setlocale(LC_ALL, str); //call to libintl
	m_locale=str;

	if( m_textdomains.size() ) {
		const char* domain = m_textdomains.back().c_str();
		bind_textdomain_codeset (domain, "UTF-8");
		bindtextdomain( domain, LOCALE_PATH );
		textdomain(domain);
	}
}

/**
 * Returns the position in the playback file
 */
int WLApplication::get_playback_offset()
{
	assert(get_playback());

	return ftell(get_play_file());
}

bool WLApplication::init_settings()
{
	Section *s=0;

	//read in the configuration file
	g_options.read("config", "global");
	s=g_options.pull_section("global");

	//then parse the commandline - overwrites conffile settings
	if (!parse_command_line()) {
		return false;
	}

	// Set Locale and grab default domain
	set_locale( s->get_string( "language" ));
	grab_textdomain("widelands");

	// Input
	sys.should_die = false;
	sys.input_grab = false;
	sys.mouse_swapped = false;
	sys.mouse_locked = false;
	sys.mouse_speed = 1.0;
	sys.mouse_buttons = 0;
	sys.mouse_x = sys.mouse_y = 0;
	sys.mouse_maxx = sys.mouse_maxy = 0;
	sys.mouse_internal_x = sys.mouse_internal_y = 0;
	sys.mouse_internal_compx = sys.mouse_internal_compy = 0;

	Sys_SetInputGrab(s->get_bool("inputgrab", false));
	Sys_SetMouseSwap(s->get_bool("swapmouse", false));
	Sys_SetMouseSpeed(s->get_float("mousespeed", 1.0));

	// KLUDGE!
	// Without this, xres, yres and workareapreview get dropped by
	// check_used().
	// Profile needs support for a Syntax definition to solve this in a sensible way
	s->get_string("xres");
	s->get_string("yres");
	s->get_bool("workareapreview");
	s->get_bool("nozip");
	s->get_int("border_snap_distance");
	s->get_int("panel_snap_distance");
	s->get_bool("snap_windows_only_when_overlapping");
	s->get_bool("dock_windows_to_edges");
	// KLUDGE!

	//make sure we didn't forget to read any global option
	g_options.check_used();

	return true;
}

void WLApplication::shutdown_settings()
{
	// To be proper, release our textdomain
	release_textdomain();

	// overwrite the old config file
	g_options.write("config", true);
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

	if (g_gr) {
		cout<<"WARNING: Hardware shutting down although graphics system"
		<<" ist still alive!"<<std::endl;
	}
	Sys_InitGraphics(0, 0, 0, false);

	SDL_Quit();
	sys.sdl_active = false;
}

bool WLApplication::init_recordplaybackfile()
{
	m_frecord = 0;
	m_fplayback = 0;

	// Open record file if necessary
	if (m_recordname[0]) {
		m_frecord = fopen(m_recordname, "wb");
		if (!m_frecord)
			throw wexception("Failed to open record file %s", m_recordname);
		else
			log("Recording into %s\n", m_recordname);

		write_record_int(RFC_MAGIC);

		m_record=true;
	}

	// Open playback file if necessary
	if (m_playbackname[0]) {
		m_fplayback = fopen(m_playbackname, "rb");
		if (!m_fplayback)
			throw wexception("Failed to open playback file %s", m_playbackname);
		else
			log("Playing back from %s\n", m_playbackname);

		if (read_record_int() != RFC_MAGIC)
			throw wexception("Playback file has wrong magic number");

		m_playback=true;
	}

	return true;
}

/**
 * Close any open journal file
 */
void WLApplication::shutdown_recordplaybackfile()
{
	if (m_record) {
		fclose(m_frecord);
		m_record = 0;
	}
	if (m_playback) {
		fclose(m_fplayback);
		m_playback = 0;
	}
}

/**
 * Parse the standard cmd line of the program
 *
 * \return true if no errors, otherwise false
*/
bool WLApplication::parse_command_line()
{
	for(int i = 1; i < m_argc; i++) {
		std::string opt=m_argv[i];
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
		if (pos==std::string::npos) {
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
			snprintf(m_recordname, sizeof(m_recordname), "%s", expanded_filename);
			continue;
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
			snprintf(m_playbackname, sizeof(m_playbackname), "%s", expanded_filename);
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
