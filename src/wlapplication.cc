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
#include "editor.h"
#include "error.h"
#include "filesystem.h"
#include "font_handler.h"
#include "fullscreen_menu_fileview.h"
#include "fullscreen_menu_intro.h"
#include "fullscreen_menu_inet_lobby.h"
#include "fullscreen_menu_inet_server_options.h"
#include "fullscreen_menu_main.h"
#include "fullscreen_menu_netsetup.h"
#include "fullscreen_menu_options.h"
#include "fullscreen_menu_singleplayer.h"
#include "fullscreen_menu_tutorial_select_map.h"
#include "game_server_connection.h"
#include "game_server_proto.h"
#include <iostream>
#include "network.h"
#include "network_ggz.h"
#include "profile.h"
#include "sound_handler.h"
#include <string>
#include "wlapplication.h"

#ifdef DEBUG
#ifndef __WIN32__
#include <signal.h>
#endif // WIN32
#endif // DEBUG

//Always specifying namespaces is good, but let's not go too far ;-)
using std::cout;
using std::endl;

//Initialize the class variables
int WLApplication::pid_me=0;
int WLApplication::pid_peer=0;
volatile int WLApplication::may_run=0;
WLApplication *WLApplication::the_singleton=0;

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
		throw wexception("%08lX: Bad code %02X during playback (%02X expected). Mismatching executable versions?",
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
 * While you \e can do the first call without parameters, it does not make
 * much sense.
 *
 * \param argc The number of command line arguments
 * \param argv Array of command line arguments
 * \return An (always valid!) pointer to the WLApplication singleton
 */
WLApplication * const WLApplication::get(const int argc, const char **argv)
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
WLApplication::WLApplication(const int argc, const char **argv)
		:m_argc(argc),m_argv(argv)
{}


/**
 * The main loop. Plain and Simple.
 *
 * Push the first event on the event queue, then keep dispatching events until
 * it is time to quit.
 */
void WLApplication::run()
{
	g_sound_handler.start_music("intro");

	Fullscreen_Menu_Intro *intro=new Fullscreen_Menu_Intro;
	intro->run();
	delete intro;

	//TODO: ??? #fweber
	if(NetGGZ::ref()->used())
	{
		if(NetGGZ::ref()->connect())
		{
			NetGame *netgame;

			if(NetGGZ::ref()->host()) netgame = new NetHost();
			else
			{
				while(!NetGGZ::ref()->ip()) NetGGZ::ref()->data();

				IPaddress peer;
				SDLNet_ResolveHost (&peer, NetGGZ::ref()->ip(), WIDELANDS_PORT);
				netgame = new NetClient(&peer);
			}
			netgame->run();
			delete netgame;
		}
	}

	g_sound_handler.change_music("menu", 1000);

	mainmenu();

	g_sound_handler.stop_music(500);
}

/**
 * Get an event from the SDL queue, just like SDL_PollEvent.
 * Perform the meat of playback/record stuff when needed.
 *
 * Throttle is a hack to stop record files from getting extremely huge.
 * If it is set to true, we will idle loop if we can't get an SDL_Event
 * returned immediately if we're recording. If there is no user input,
 * the actual mainloop will be throttled to 100fps.
 *
 * \param ev the retrieved event will be put here
 * \param throttle Limit event loop to 100fps max
 * \return
 */
const bool WLApplication::poll_event(SDL_Event *ev, const bool throttle)
{
	bool haveevent;

restart:
	//inject synthesized events into the event queue when playing back
	if (WLApplication::get()->get_playback())
		{
			uchar code = read_record_char();

			if (code == RFC_EVENT)
			{
				code = read_record_char();

				switch(code) {
				case RFC_KEYDOWN:
				case RFC_KEYUP:
					ev->type = (code == RFC_KEYUP) ? SDL_KEYUP : SDL_KEYDOWN;
					ev->key.keysym.sym = (SDLKey)read_record_int();
					ev->key.keysym.unicode = read_record_int();
					break;

				case RFC_MOUSEBUTTONDOWN:
				case RFC_MOUSEBUTTONUP:
					ev->type = (code == RFC_MOUSEBUTTONUP) ? SDL_MOUSEBUTTONUP : SDL_MOUSEBUTTONDOWN;
					ev->button.button = read_record_char();
					break;

				case RFC_MOUSEMOTION:
					ev->type = SDL_MOUSEMOTION;
					ev->motion.x = read_record_int();
					ev->motion.y = read_record_int();
					ev->motion.xrel = read_record_int();
					ev->motion.yrel = read_record_int();
					break;

				case RFC_QUIT:
					ev->type = SDL_QUIT;
					break;

				default:
					throw wexception("%08lX: Unknown event type %02X in playback.",
					                 WLApplication::get()->get_playback_offset()-1, code);
				}

				haveevent = true;
			}
			else if (code == RFC_ENDEVENTS)
			{
				haveevent = false;
			}
			else
				throw wexception("%08lX: Bad code %02X in event playback.",
				                 WLApplication::get()->get_playback_offset()-1, code);
		}
	else
	{ //not playing back
		haveevent = SDL_PollEvent(ev);

		if (haveevent)
		{
			// We edit mouse motion events in here, so that differences caused by
			// GrabInput or mouse speed settings are invisible to the rest of the code
			switch(ev->type) {
			case SDL_MOUSEMOTION:
				ev->motion.xrel += m_mouse_internal_compx;
				ev->motion.yrel += m_mouse_internal_compy;
				m_mouse_internal_compx = m_mouse_internal_compy = 0;

				if (m_input_grab)
				{
					float xlast = m_mouse_internal_x;
					float ylast = m_mouse_internal_y;

					m_mouse_internal_x += ev->motion.xrel * m_mouse_speed;
					m_mouse_internal_y += ev->motion.yrel * m_mouse_speed;

					ev->motion.xrel = (int)m_mouse_internal_x - (int)xlast;
					ev->motion.yrel = (int)m_mouse_internal_y - (int)ylast;

					if (m_mouse_locked)
					{
						// mouse is locked; so don't move the cursor
						m_mouse_internal_x = xlast;
						m_mouse_internal_y = ylast;
					}
					else
					{
						if (m_mouse_internal_x < 0)
							m_mouse_internal_x = 0;
						else if (m_mouse_internal_x >= m_mouse_maxx-1)
							m_mouse_internal_x = m_mouse_maxx-1;
						if (m_mouse_internal_y < 0)
							m_mouse_internal_y = 0;
						else if (m_mouse_internal_y >= m_mouse_maxy-1)
							m_mouse_internal_y = m_mouse_maxy-1;
					}

					ev->motion.x = (int)m_mouse_internal_x;
					ev->motion.y = (int)m_mouse_internal_y;
				}
				else
				{
					int xlast = m_mouse_x;
					int ylast = m_mouse_y;

					if (m_mouse_locked)
					{
						set_mouse_pos(xlast, ylast);

						ev->motion.x = xlast;
						ev->motion.y = ylast;
					}
				}

				break;
			case SDL_USEREVENT:
				if (ev->user.code==Sound_Handler::SOUND_HANDLER_CHANGE_MUSIC)
					g_sound_handler.change_music();

				break;
			}
		}
	}

	// log all events into the journal file
	if (WLApplication::get()->get_record())
		{
			if (haveevent)
			{
				switch(ev->type) {
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					write_record_char(RFC_EVENT);
					write_record_char((ev->type == SDL_KEYUP) ? RFC_KEYUP : RFC_KEYDOWN);
					write_record_int(ev->key.keysym.sym);
					write_record_int(ev->key.keysym.unicode);
					break;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					write_record_char(RFC_EVENT);
					write_record_char((ev->type == SDL_MOUSEBUTTONUP) ? RFC_MOUSEBUTTONUP : RFC_MOUSEBUTTONDOWN);
					write_record_char(ev->button.button);
					break;

				case SDL_MOUSEMOTION:
					write_record_char(RFC_EVENT);
					write_record_char(RFC_MOUSEMOTION);
					write_record_int(ev->motion.x);
					write_record_int(ev->motion.y);
					write_record_int(ev->motion.xrel);
					write_record_int(ev->motion.yrel);
					break;

				case SDL_QUIT:
					write_record_char(RFC_EVENT);
					write_record_char(RFC_QUIT);
					break;

				default:
					goto restart; // can't really do anything useful with this command
				}
			}
			else
			{
				// Implement the throttle to avoid very quick inner mainloops when
				// recoding a session
				if (throttle && !WLApplication::get()->get_playback())
					{
						static int lastthrottle = 0;
						int time = SDL_GetTicks();

						if (time - lastthrottle < 10)
							goto restart;

						lastthrottle = time;
					}

				write_record_char(RFC_ENDEVENTS);
			}
		}
	else
	{ //not recording
		if (haveevent)
		{
			// Eliminate any unhandled events to make sure record and playback are
			// _really_ the same.
			// Yes I know, it's overly paranoid but hey...
			switch(ev->type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
			case SDL_QUIT:
				break;

			default:
				goto restart;
			}
		}
	}

	return haveevent;
}


/**
 * Pump the event queue, get packets from the network, etc...
 */
void WLApplication::handle_input(const InputCallback *cb)
{
	bool gotevents = false;
	SDL_Event ev;

	NetGGZ::ref()->data();
	NetGGZ::ref()->datacore();

	// We need to empty the SDL message queue always, even in playback mode
	// In playback mode, only F10 for premature exiting works
	if (WLApplication::get()->get_playback()) {
			while(SDL_PollEvent(&ev)) {
				switch(ev.type) {
				case SDL_KEYDOWN:
					if (ev.key.keysym.sym == SDLK_F10) // TEMP - get out of here quick
						m_should_die = true;
					break;

				case SDL_QUIT:
					m_should_die = true;
					break;
				}
			}
		}

	// Usual event queue
	while(poll_event(&ev, !gotevents)) {
		int button;

		gotevents = true;

		// CAREFUL: Record files do not save the entire SDL_Event structure.
		// Therefore, playbacks are incomplete. When you change the following
		// code so that it uses previously unused fields in SDL_Event,
		// please also take a look at Sys_PollEvent()

		switch(ev.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (ev.key.keysym.sym == SDLK_F10) // TEMP - get out of here quick
			{
				if (ev.type == SDL_KEYDOWN)
					m_should_die = true;
				break;
			}
			if (ev.key.keysym.sym == SDLK_F11) // take screenshot
			{
				if (ev.type == SDL_KEYDOWN) {
					char buf[256];
					int nr;

					for(nr = 0; nr < 10000; nr++) {
						snprintf(buf, sizeof(buf), "shot%04i.bmp", nr);
						if (g_fs->FileExists(buf))
							continue;
						g_gr->screenshot(buf);
						break;
					}
				}
				break;
			}
			if (cb && cb->key) {
				int c;

				c = ev.key.keysym.unicode;
				if (c < 32 || c >= 128)
					c = 0;

				cb->key(ev.type == SDL_KEYDOWN, ev.key.keysym.sym, (char)c);
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			button = ev.button.button-1;
			if (m_mouse_swapped) {
				if (button == MOUSE_LEFT) button = MOUSE_RIGHT;
				else if (button == MOUSE_RIGHT) button = MOUSE_LEFT;
			}

			if (ev.type == SDL_MOUSEBUTTONDOWN)
				//TODO: no bitshifting
				m_mouse_buttons |= 1 << button;
			else
				//TODO: no bitshifting
				m_mouse_buttons &= ~(1 << button);

			if (cb && cb->mouse_click)
				cb->mouse_click(ev.type == SDL_MOUSEBUTTONDOWN, button, m_mouse_buttons,
				                (int)m_mouse_x, (int)m_mouse_y);
			break;

		case SDL_MOUSEMOTION: {
				// All the interesting stuff is now in Sys_PollEvent()
				int xdiff = ev.motion.xrel;
				int ydiff = ev.motion.yrel;

				m_mouse_x = ev.motion.x;
				m_mouse_y = ev.motion.y;

				if (!xdiff && !ydiff)
					break;

				if (cb && cb->mouse_move)
					cb->mouse_move(m_mouse_buttons, m_mouse_x, m_mouse_y, xdiff, ydiff);

				break;
			}

		case SDL_QUIT:
			m_should_die = true;
			break;

		default:
			break;
		}
	}
}

/**
 * Initialize everything that needs to be set up before starting the main loop
 * (and consequently, as the loop's starting point, the main menu)
 *
 * \return Whether the initalization was successful
 */
const bool WLApplication::init()
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
 * Set the locale to the given string
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
const long int WLApplication::get_playback_offset()
{
	assert(get_playback());

	return ftell(get_play_file());
}

/**
 * Return the current time, in milliseconds
 * \todo Convert from int to Uint32
 */
const int WLApplication::get_time()
{
	int time;

	if (get_playback()) {
		read_record_code(RFC_GETTIME);
		time = read_record_int();
	} else
		time = SDL_GetTicks();

	if (get_record()) {
		write_record_code(RFC_GETTIME);
		write_record_int(time);
	}

	return time;
}

/**
 * Move the mouse cursor.
 * No mouse moved event will be issued.
*/
void WLApplication::set_mouse_pos(int x, int y)
{
	m_mouse_x = x;
	m_mouse_y = y;
	m_mouse_internal_x = x;
	m_mouse_internal_y = y;

	if (!m_input_grab)
		do_warp_mouse(x, y); // sync mouse positions
}

/**
 * Changes input grab mode.
 */
void WLApplication::set_input_grab(bool grab)
{
	if (get_playback())
		return; // ignore in playback mode

	m_input_grab = grab;

	if (grab) {
		SDL_WM_GrabInput(SDL_GRAB_ON);

		m_mouse_internal_x = m_mouse_x;
		m_mouse_internal_y = m_mouse_y;
	} else {
		SDL_WM_GrabInput(SDL_GRAB_OFF);

		do_warp_mouse(m_mouse_x, m_mouse_y);
	}
}

void WLApplication::set_mouse_speed(float speed)
{
	if (speed <= 0.1 || speed >= 10.0)
		speed = 1.0;
	m_mouse_speed = speed;
}

/** [kludge]
 * Set the mouse boundary after a change of resolution
 * This is manually imported by graphic.cc
 */
void WLApplication::set_max_mouse_coords(const int x, const int y)
{
	m_mouse_maxx = x;
	m_mouse_maxy = y;
}

/**
 * Warp the SDL mouse cursor to the given position.
 * Store the delta mouse_internal_compx/y, so that the resulting motion
 * event can be eliminated.
 */
void WLApplication::do_warp_mouse(const int x, const int y)
{
	int curx, cury;

	if (get_playback()) // don't warp anything during playback
		return;

	SDL_GetMouseState(&curx, &cury);

	if (curx == x && cury == y)
		return;

	m_mouse_internal_compx += curx - x;
	m_mouse_internal_compy += cury - y;

	SDL_WarpMouse(x, y);
}

/**
 * Initialize the graphics subsystem (or shutdown, if system == GFXSYS_NONE) with
 * the given resolution.
 * Throws an exception on failure.
 *
 * \note Because of the way pictures are handled now, this function must not be
 * called while UI elements are active.
 */
Graphic* SW16_CreateGraphics(int w, int h, int bpp, bool fullscreen);
void WLApplication::init_graphics(const int w, const int h,
                                  const int bpp, const bool fullscreen)
{
	if (w == m_gfx_w && h == m_gfx_h && fullscreen == m_gfx_fullscreen)
		return;

	if (g_gr)
	{
		delete g_gr;
		g_gr = 0;
	}

	m_gfx_w = w;
	m_gfx_h = h;
	m_gfx_fullscreen = fullscreen;

	// If we are not to be shut down
	if( w && h ) {
		g_gr = SW16_CreateGraphics(w, h, bpp, fullscreen);
		set_max_mouse_coords(w, h);
	}
}

/**
 * Read the config file, parse the commandline and give all other internal
 * parameters sensible default values
 */
const bool WLApplication::init_settings()
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
	m_should_die = false;
	m_input_grab = false;
	m_mouse_swapped = false;
	m_mouse_locked = false;
	m_mouse_speed = 1.0;
	m_mouse_buttons = 0;
	m_mouse_x = m_mouse_y = 0;
	m_mouse_maxx = m_mouse_maxy = 0;
	m_mouse_internal_x = m_mouse_internal_y = 0;
	m_mouse_internal_compx = m_mouse_internal_compy = 0;

	set_input_grab(s->get_bool("inputgrab", false));
	set_mouse_swap(s->get_bool("swapmouse", false));
	set_mouse_speed(s->get_float("mousespeed", 1.0));

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

/**
 * Remember the last settings: writ them into the config file
 */
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
const bool WLApplication::init_hardware()
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
		m_sdl_active=true;

	SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableUNICODE(1); // useful for e.g. chat messages
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	init_graphics(640, 480, s->get_int("depth",16), s->get_bool("fullscreen", false));

	// Start the audio subsystem
	// must know the locale before calling this!
	g_sound_handler.init();

	return true;
}

/**
 * Shut the hardware down: stop graphics mode, stop sound handler
 */
void WLApplication::shutdown_hardware()
{
	g_sound_handler.shutdown();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	if (g_gr) {
		cout<<"WARNING: Hardware shutting down although graphics system"
		<<" ist still alive!"<<endl;
	}
	init_graphics(0, 0, 0, false);

	SDL_Quit();
	m_sdl_active = false;
}

/**
 * Open record and/or playback file for writing/reading.
 */
const bool WLApplication::init_recordplaybackfile()
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
 * Parse the command line given in \ref m_argc and \ref m_argv
 *
 * \return true if no errors, otherwise false
*/
const bool WLApplication::parse_command_line()
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
			cout<<endl
			<<"ERROR: invalid option: --"<<opt<<endl<<endl;

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
				cout<<endl
				<<"ERROR: --record needs a filename!"<<endl<<endl;
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
				cout<<endl
				<<"ERROR: --playback needs a filename!"<<endl<<endl;
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
	cout<<"This is Widelands-"<<VERSION<<endl<<endl
	<<"Usage: widelands <option0>=<value0> ... <optionN>=<valueN>"<<endl
	<<endl
	<<"Options:"<<endl
	<<endl
	<<" --<config-entry-name>=value overwrites any config file setting"<<endl
	<<endl
	<<" --record=FILENAME         Record all events to the given filename for later playback"<<endl
	<<" --playback=FILENAME       Playback given filename (see --record)"<<endl
	<<endl
	<<" --coredump=[yes|no]       Generates a core dump on segfaults instead of using the SDL"<<endl
	<<endl
	<<" --ggz                     Starts game as GGZ Gaming Zone client (don't use!)"<<endl
	<<" --nosound                 Starts the game with sound disabled"<<endl
	<<" --nozip                   Do not save files as binary zip archives."<<endl
	<<endl
#ifdef DEBUG
#ifndef __WIN32__
	<<" --double                  Start the game twice (for localhost network testing)"<<endl
	<<endl
#endif
#endif
	<<" --help                    Show this help"<<endl
	<<endl
	<<"Bug reports? Suggestions? Check out the project website:"<<endl
	<<"  http://www.sourceforge.net/projects/widelands"<<endl
	<<"Hope you enjoy this game!"<<endl;
}

/**
 * Fork off a second game to test network gaming
 *
 * \warning You must call this \e before any hardware initialization - most
 * notably before \ref SDL_Init()
 */
void WLApplication::init_double_game ()
{
	if (pid_me!=0)
		return;

	pid_me=getpid();
	pid_peer=fork();
	//TODO: handle fork errors

	assert (pid_peer>=0);

	if (pid_peer==0) {
		pid_peer=pid_me;
		pid_me=getpid();

		may_run=1;
	}

	signal (SIGUSR1, signal_handler);

	atexit (quit_handler);
}

void WLApplication::signal_handler (int sig)
{
	may_run++;
}

void WLApplication::quit_handler()
{
	kill (pid_peer, SIGTERM);
	sleep (2);
	kill (pid_peer, SIGKILL);
}

void WLApplication::yield_double_game()
{
	if (pid_me==0)
		return;

	if (may_run>0) {
		may_run--;
		kill (pid_peer, SIGUSR1);
	}

	if (may_run==0)
		usleep (500000);

	// using sleep instead of pause avoids a race condition
	// and a deadlock during connect
}

/**
 * Menus
 */
//@{
void WLApplication::mainmenu()
{
	bool done=false;

	while(!done) {
		unsigned char code;

		Fullscreen_Menu_Main *mm = new Fullscreen_Menu_Main;
		code = mm->run();
		delete mm;

		switch(code) {
		case Fullscreen_Menu_Main::mm_singleplayer:
			mainmenu_singleplayer();
			break;

		case Fullscreen_Menu_Main::mm_multiplayer:
			mainmenu_multiplayer();
			break;

		case Fullscreen_Menu_Main::mm_options:
			{
				Section *s = g_options.pull_section("global");
				Options_Ctrl *om = new Options_Ctrl(s);
				delete om;
			}
			break;

		case Fullscreen_Menu_Main::mm_readme:
			{
				Fullscreen_Menu_FileView* ff=new Fullscreen_Menu_FileView( "txts/README" );
				ff->run();
				delete ff;
			}
			break;

		case Fullscreen_Menu_Main::mm_license:
			{
				Fullscreen_Menu_FileView* ff=new Fullscreen_Menu_FileView( "txts/COPYING" );
				ff->run();
				delete ff;
			}
			break;

		case Fullscreen_Menu_Main::mm_editor:
			{
				Editor* e=new Editor();
				e->run();
				delete e;
				break;
			}

		default:
		case Fullscreen_Menu_Main::mm_exit:
			done=true;
			break;
		}
	}
}

void WLApplication::mainmenu_singleplayer()
{
	bool done=false;
	while(!done) {
		Fullscreen_Menu_SinglePlayer *sp = new Fullscreen_Menu_SinglePlayer;
		int code = sp->run();
		delete sp;

		switch(code) {
		case Fullscreen_Menu_SinglePlayer::sp_skirmish:
			{
				Game *g = new Game;
				bool ran = g->run_single_player();
				delete g;
				if (ran) {
					// game is over. everything's good. restart Main Menu
					done=true;
				}
				continue;
			}

		case Fullscreen_Menu_SinglePlayer::sp_loadgame:
			{
				Game* g = new Game;
				bool ran = g->run_load_game(true);
				delete g;
				if (ran) {
					done=true;
				}
				continue;
			}

		case Fullscreen_Menu_SinglePlayer::sp_tutorial:
			{
				Fullscreen_Menu_TutorialSelectMap* sm = new Fullscreen_Menu_TutorialSelectMap;
				int code = sm->run();
				if(code) {
					std::string mapname = sm->get_mapname( code );
					delete sm;

					Game* g = new Game;
					bool run = g->run_splayer_map_direct( mapname.c_str(), true);
					delete g;
					if(run)
						done = true;
					continue;
				}
				// Fallthrough if back was pressed
			}

		default:
		case Fullscreen_Menu_SinglePlayer::sp_back:
			done = true;
			break;
		}
	}
}

void WLApplication::mainmenu_multiplayer()
{
	Fullscreen_Menu_NetSetup* ns = new Fullscreen_Menu_NetSetup();
	if(NetGGZ::ref()->tables().size() > 0) ns->fill(NetGGZ::ref()->tables());
	int code=ns->run();

	NetGame* netgame = 0;

	if (code==Fullscreen_Menu_NetSetup::HOSTGAME)
		netgame=new NetHost();
	else if (code==Fullscreen_Menu_NetSetup::JOINGAME) {
		IPaddress peer;

		//			    if (SDLNet_ResolveHost (&peer, ns->get_host_address(), WIDELANDS_PORT) < 0)
		//				    throw wexception("Error resolving hostname %s: %s\n", ns->get_host_address(), SDLNet_GetError());
		ulong addr;
		ushort port;

		if (!ns->get_host_address(addr,port))
			throw wexception("Address of game server is no good");

		peer.host=addr;
		peer.port=port;

		netgame=new NetClient(&peer);
	} else if(code==Fullscreen_Menu_NetSetup::INTERNETGAME) {
		delete ns;
		Fullscreen_Menu_InetServerOptions* igo = new Fullscreen_Menu_InetServerOptions();
		int code=igo->run();

		// Get informations here
		std::string host = igo->get_server_name();
		std::string player = igo->get_player_name();
		delete igo;

		if(code) {
			Game_Server_Connection csc(host, GAME_SERVER_PORT);

			try {
				csc.connect();
			} catch(...) {
				// TODO: error handling here
				throw;
			}

			csc.set_username(player.c_str());

			// Wowi, we are connected. Let's start the lobby
			Fullscreen_Menu_InetLobby* il = new Fullscreen_Menu_InetLobby(&csc);
			il->run();
			delete il;
		}
		//break;
	}
	else if((code == Fullscreen_Menu_NetSetup::JOINGGZGAME)
	        || (code == Fullscreen_Menu_NetSetup::HOSTGGZGAME)) {
		if(code == Fullscreen_Menu_NetSetup::HOSTGGZGAME) NetGGZ::ref()->launch();
		if(NetGGZ::ref()->host()) netgame = new NetHost();
		else
		{
			while(!NetGGZ::ref()->ip()) NetGGZ::ref()->data();

			IPaddress peer;
			SDLNet_ResolveHost (&peer, NetGGZ::ref()->ip(), WIDELANDS_PORT);
			netgame = new NetClient(&peer);
		}
	}
	else;
	//break;

	delete ns;

	netgame->run();
	delete netgame;
}
//@}
