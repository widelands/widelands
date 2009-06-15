/*
 * Copyright (C) 2006-2009 by the Widelands Development Team
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

#include "wlapplication.h"

#include "build_info.h"
#include "computer_player.h"
#include "io/filesystem/disk_filesystem.h"
#include "editor/editorinteractive.h"
#include "font_handler.h"
#include "ui_fsmenu/campaign_select.h"
#include "ui_fsmenu/editor.h"
#include "ui_fsmenu/editor_mapselect.h"
#include "ui_fsmenu/fileview.h"
#include "ui_fsmenu/intro.h"
#include "ui_fsmenu/launchgame.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/loadreplay.h"
#include "ui_fsmenu/main.h"
#include "ui_fsmenu/multiplayer.h"
#include "ui_fsmenu/netsetup_ggz.h"
#include "ui_fsmenu/netsetup_lan.h"
#include "ui_fsmenu/options.h"
#include "ui_fsmenu/singleplayer.h"
#include "logic/game.h"
#include "wui/game_tips.h"
#include "gamesettings.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"
#include "journal.h"
#include "io/filesystem/layered_filesystem.h"
#include "map.h"
#include "map_io/map_loader.h"
#include "network/netclient.h"
#include "network/nethost.h"
#include "network/network_ggz.h"
#include "profile/profile.h"
#include "replay.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "warning.h"
#include "wexception.h"

#include "log.h"

#include "timestring.h"

#include <boost/scoped_ptr.hpp>
#include <sys/types.h>
#include <sys/stat.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

#ifdef DEBUG
#ifndef WIN32
int32_t WLApplication::pid_me   = 0;
int32_t WLApplication::pid_peer = 0;
volatile int32_t WLApplication::may_run = 0;
#include <csignal>
#endif
#endif


//Always specifying namespaces is good, but let's not go too far ;-)
//using std::cout;
std::ostream & wout = std::cout;
using std::endl;

/**
 * Sets the filelocators default searchpaths (partly OS specific)
 * \todo Handle exception FileType_error
 * \todo Handle case when \e no data can be found
 */
void WLApplication::setup_searchpaths(std::string argv0)
{
	try {
#ifdef __APPLE__
		// on mac, the default data dir is relative to the current directory
		log ("Adding directory:Widelands.app/Contents/Resources/\n");
		g_fs->AddFileSystem
			(FileSystem::Create("Widelands.app/Contents/Resources/"));
#else
		// first, try the data directory used in the last scons invocation
		log ("Adding directory:%s\n", INSTALL_PREFIX "/" INSTALL_DATADIR);
		g_fs->AddFileSystem //  see config.h
			(FileSystem::Create
			 	(std::string(INSTALL_PREFIX) + '/' + INSTALL_DATADIR));
#endif
	}
	catch (FileNotFound_error e) {}
	catch (FileAccessDenied_error e) {
		log("Access denied on %s. Continuing.\n", e.m_filename.c_str());
	}
	catch (FileType_error e) {
		//TODO: handle me
	}

	try {
#ifndef WIN32
		// if that fails, search in FHS standard location (obviously UNIX-only)
		log ("Adding directory:/usr/share/games/widelands\n");
		g_fs->AddFileSystem(FileSystem::Create("/usr/share/games/widelands"));
#else
		//TODO: is there a "default dir" for this on win32 and mac ?
#endif
	}
	catch (FileNotFound_error e) {}
	catch (FileAccessDenied_error e) {
		log("Access denied on %s. Continuing.\n", e.m_filename.c_str());
	}
	catch (FileType_error e) {
		//TODO: handle me
	}

	try {
		// absolute fallback directory is the CWD
		log ("Adding directory:.\n");
		g_fs->AddFileSystem(FileSystem::Create("."));
	}
	catch (FileNotFound_error e) {}
	catch (FileAccessDenied_error e) {
		log("Access denied on %s. Continuing.\n", e.m_filename.c_str());
	}
	catch (FileType_error e) {
		//TODO: handle me
	}

	//TODO: what if all the searching failed? Bail out!

	// the directory the executable is in is the default game data directory
	std::string::size_type slash = argv0.rfind('/');
	std::string::size_type backslash = argv0.rfind('\\');

	if
		(backslash != std::string::npos &&
		 (slash == std::string::npos || backslash > slash))
		slash = backslash;

	if (slash != std::string::npos) {
		argv0.erase(slash);
		if (argv0 != ".") {
			try {
				log ("Adding directory: %s\n", argv0.c_str());
				g_fs->AddFileSystem(FileSystem::Create(argv0));
#ifdef USE_DATAFILE
				argv0.append ("/widelands.dat");
				g_fs->AddFileSystem(new Datafile(argv0.c_str()));
#endif
			}
			catch (FileNotFound_error e) {}
			catch (FileAccessDenied_error e) {
				log ("Access denied on %s. Continuing.\n", e.m_filename.c_str());
			}
			catch (FileType_error e) {
				//TODO: handle me
			}
		}
	}
	//now make sure we always access the file with the right version first
	g_fs->PutRightVersionOnTop();
}
void WLApplication::setup_homedir() {
	std::string path = FileSystem::GetHomedir();

	//If we don't have a home directory don't do anything
	if (path.size()) {
		RealFSImpl(path).EnsureDirectoryExists(".widelands");
		path += "/.widelands";
		try {
			log ("Set home directory: %s\n", path.c_str());
			g_fs->SetHomeFileSystem(FileSystem::Create(path.c_str()));
		} catch (FileNotFound_error     const & e) {
		} catch (FileAccessDenied_error const & e) {
			log("Access denied on %s. Continuing.\n", e.m_filename.c_str());
		} catch (FileType_error         const & e) {
			//TODO: handle me
		}
	} else {
		//TODO: complain
	}
}

WLApplication * WLApplication::the_singleton = 0;

/**
 * The main entry point for the WLApplication singleton.
 *
 * Regardless of circumstances, this will return the one and only valid
 * WLApplication object when called. If necessary, a new WLApplication instance
 * is created.
 *
 * While you \e can do the first call to this method without parameters, it does
 * not make much sense.
 *
 * \param argc The number of command line arguments
 * \param argv Array of command line arguments
 * \return An (always valid!) pointer to the WLApplication singleton
 *
 * \todo Return a reference - the return value is always valid anyway
 */
WLApplication * WLApplication::get(int const argc, char const * * argv) {
	if (the_singleton == 0)
		the_singleton = new WLApplication(argc, argv);
	return the_singleton;
}

/**
 * Initialize an instance of WLApplication.
 *
 * This constructor is protected \e on \e purpose !
 * Use WLApplication::get() instead and look at the class description.
 *
 * For easier access, we repackage argc/argv into an STL map here.
 * If you specify the same option more than once, only the last occurrence
 * is effective.
 *
 * \param argc The number of command line arguments
 * \param argv Array of command line arguments
 */
WLApplication::WLApplication(const int argc, const char **argv):
m_commandline          (std::map<std::string, std::string>()),
m_game_type            (NONE),
journal                (0),
m_mouse_swapped        (false),
m_mouse_position       (0, 0),
m_mouse_locked         (0),
m_mouse_compensate_warp(0, 0),
m_should_die           (false),
m_gfx_w(0), m_gfx_h(0),
m_gfx_fullscreen       (false),
m_gfx_hw_improvements  (false),
m_gfx_double_buffer    (false),
m_default_datadirs     (true)
{
	g_fs = new LayeredFileSystem();
	g_fh = new Font_Handler();

	m_editor_commandline = false;
	parse_commandline(argc, argv); //throws Parameter_error, handled by main.cc
	if (m_default_datadirs) {
		setup_searchpaths(m_commandline["EXENAME"]);
	}
	setup_homedir();
	init_settings();
	init_hardware();

	//make sure we didn't forget to read any global option
	g_options.check_used();
}

/**
 * Shut down all subsystems in an orderly manner
 * \todo Handle errors that happen here!
 */
WLApplication::~WLApplication()
{
	//Do use the opposite order of WLApplication::init()

	shutdown_hardware();
	shutdown_settings();

	assert(g_fh);
	delete g_fh;
	g_fh = 0;

	assert(g_fs);
	delete g_fs;
	g_fs = 0;
}

/**
 * The main loop. Plain and Simple.
 *
 * \todo Refactor the whole mainloop out of class \ref UI::Panel into here.
 * In the future: push the first event on the event queue, then keep
 * dispatching events until it is time to quit.
 */
void WLApplication::run()
{
	if (m_game_type == EDITOR) {
		g_sound_handler.start_music("ingame");
		Editor_Interactive::run_editor(m_filename);
	} else if (m_game_type == LOADGAME) {
		Widelands::Game game;
		try {
			game.run_load_game(m_filename.c_str());
		} catch (...) {
			emergency_save(game);
			throw;
		}
	} else if (m_game_type == SCENARIO) {
		Widelands::Game game;
		try {
			game.run_splayer_scenario_direct(m_filename.c_str());
		} catch (...) {
			emergency_save(game);
			throw;
		}
#if HAVE_GGZ
	} else if (m_game_type == GGZ) {
		Widelands::Game game;
		try {
		  //Fullscreen_Menu_NetSetupGGZ ns;

		  //setup some detalis about a dedicated server
		  std::string playername = "dedicated";
		  uint32_t mp = static_cast<uint32_t>(10);

		  NetHost netgame(playername, true);

		  NetGGZ::ref().set_local_maxplayers(mp);

		  //Load te requested map
		  GameSettings const & settings = netgame.settings();
		  Widelands::Map map;
		  i18n::Textdomain td("maps");
		  map.set_filename(m_filename.c_str());
		  Widelands::Map_Loader * const ml = map.get_correct_loader(m_filename.c_str());
		  ml->preload_map(true);

		  //fill in the mapdata structure
		  MapData mapdata;
		  mapdata.filename = m_filename;
		  mapdata.name = map.get_name();
		  mapdata.author = map.get_author();
		  mapdata.description = map.get_description();
		  mapdata.world = map.get_world_name();
		  mapdata.nrplayers = map.get_nrplayers();
		  mapdata.width = map.get_width();
		  mapdata.height = map.get_height();

		  //set the map
		  netgame.setMap(mapdata.name, mapdata.filename, mapdata.nrplayers);

		  //run the network game, autostart when everyone is ready
		  netgame.run(true);

		  NetGGZ::ref().deinitcore();
		} catch (...) {
			emergency_save(game);
			throw;
		}
#endif
	} else {

		g_sound_handler.start_music("intro");

		{
			Fullscreen_Menu_Intro intro;
			intro.run();
		}

		g_sound_handler.change_music("menu", 1000);
		mainmenu();

		delete g_gr;
		g_gr = 0;
	}

	g_sound_handler.stop_music(500);

	return;
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
 * \param throttle Limit recording to 100fps max (not the event loop itself!)
 *
 * \return true if an event was returned inside ev, false otherwise
 *
 * \todo Catch Journalfile_error
 */
bool WLApplication::poll_event(SDL_Event *ev, const bool throttle) {
	bool haveevent = false;

restart:
	//inject synthesized events into the event queue when playing back
	if (journal->is_playingback()) {
		try {
			haveevent = journal->read_event(ev);
		} catch (Journalfile_error const & e) {
			// An error might occur here when playing back a file that
			// was not finalized due to a crash etc.
			// Since playbacks are intended precisely for debugging such
			// crashes, we must ignore the error and continue.
			log("JOURNAL: read error, continue without playback: %s\n", e.what());
			journal->stop_playback();
		}
	} else {
		haveevent = SDL_PollEvent(ev);

		if (haveevent) {
			// We edit mouse motion events in here, so that
			// differences caused by GrabInput or mouse speed
			// settings are invisible to the rest of the code
			switch (ev->type) {
			case SDL_MOUSEMOTION:
				ev->motion.xrel += m_mouse_compensate_warp.x;
				ev->motion.yrel += m_mouse_compensate_warp.y;
				m_mouse_compensate_warp = Point(0, 0);

				if (m_mouse_locked) {
					warp_mouse(m_mouse_position);

					ev->motion.x = m_mouse_position.x;
					ev->motion.y = m_mouse_position.y;
				}

				break;
			case SDL_USEREVENT:
				if (ev->user.code == CHANGE_MUSIC)
					g_sound_handler.change_music();

				break;
			}
		}
	}

	// log all events into the journal file
	if (journal->is_recording()) {
		if (haveevent)
			journal->record_event(ev);
		else {
			// Implement the throttle to avoid very quick inner mainloops when
			// recoding a session
			if (throttle && journal->is_playingback()) {
				static int32_t lastthrottle = 0;
				int32_t time = SDL_GetTicks();

				if (time - lastthrottle < 10)
					goto restart;

				lastthrottle = time;
			}

			journal->set_idle_mark();
		}
	}
	else
	{ //not recording
		if (haveevent)
		{
			// Eliminate any unhandled events to make sure record
			// and playback are _really_ the same.
			// Yes I know, it's overly paranoid but hey...
			switch (ev->type) {
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
	SDL_Event ev; //  Valgrind says:
	// Conditional jump or move depends on uninitialised value(s)
	// at 0x407EEDA: (within /usr/lib/libSDL-1.2.so.0.11.0)
	// by 0x407F78F: (within /usr/lib/libSDL-1.2.so.0.11.0)
	// by 0x404FB12: SDL_PumpEvents (in /usr/lib/libSDL-1.2.so.0.11.0)
	// by 0x404FFC3: SDL_PollEvent (in /usr/lib/libSDL-1.2.so.0.11.0)
	// by 0x8252545: WLApplication::poll_event(SDL_Event*, bool)
	//     (wlapplication.cc:309)
	// by 0x8252EB6: WLApplication::handle_input(InputCallback const*)
	// (wlapplication.cc:459) by 0x828B56E: UI::Panel::run() (ui_panel.cc:148)
	// by 0x8252FAB: WLApplication::run() (wlapplication.cc:212)
	// by 0x81427A6: main (main.cc:39)

	// We need to empty the SDL message queue always, even in playback mode
	// In playback mode, only F10 for premature exiting works
	if (journal->is_playingback()) {
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
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
	while (poll_event(&ev, !gotevents)) {

		gotevents = true;

		// CAREFUL: Record files do not save the entire SDL_Event structure.
		// Therefore, playbacks are incomplete. When you change the following
		// code so that it uses previously unused fields in SDL_Event,
		// please also take a look at Journal::read_event and
		// Journal::record_event

		switch (ev.type) {
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
				if (ev.type == SDL_KEYDOWN)
					for (uint32_t nr = 0; nr < 10000; ++nr) {
						char buffer[256];
						snprintf(buffer, sizeof(buffer), "shot%04u.bmp", nr); // FIXME
						if (g_fs->FileExists(buffer)) continue;
						g_gr->screenshot(*buffer);
						break;
					}
				break;
			}
			if (cb && cb->key) {
				int16_t c = ev.key.keysym.unicode;

				//TODO: this kills international characters
				if (c < 32 || c >= 128)
					c = 0;

				cb->key(ev.type == SDL_KEYDOWN, ev.key.keysym);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (cb and cb->mouse_press) {
				if (m_mouse_swapped) {
					switch (ev.button.button) {
					case SDL_BUTTON_LEFT:
						ev.button.button = SDL_BUTTON_RIGHT;
						break;
					case SDL_BUTTON_RIGHT:
						ev.button.button = SDL_BUTTON_LEFT;
						break;
					}
				}
				assert(ev.button.state == SDL_PRESSED);
				cb->mouse_press(ev.button.button, ev.button.x, ev.button.y);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (cb and cb->mouse_release) {
				if (m_mouse_swapped) {
					switch (ev.button.button) {
					case SDL_BUTTON_LEFT:
						ev.button.button = SDL_BUTTON_RIGHT;
						break;
					case SDL_BUTTON_RIGHT:
						ev.button.button = SDL_BUTTON_LEFT;
						break;
					}
				}
				assert(ev.button.state == SDL_RELEASED);
				cb->mouse_release(ev.button.button, ev.button.x, ev.button.y);
			}
			break;

		case SDL_MOUSEMOTION:
			// All the interesting stuff is now in Sys_PollEvent()

			m_mouse_position = Point(ev.motion.x, ev.motion.y);

			if ((ev.motion.xrel or ev.motion.yrel) and cb and cb->mouse_move)
				cb->mouse_move
					(ev.motion.state,
					 ev.motion.x,    ev.motion.y,
					 ev.motion.xrel, ev.motion.yrel);
			break;

		case SDL_QUIT:
			m_should_die = true;
			break;
		}
	}
}

/**
 * Return the current time, in milliseconds
 * \todo Use our internally defined time type
 */
int32_t WLApplication::get_time() {
	Uint32 time;

	time = SDL_GetTicks();
	journal->timestamp_handler(&time); //might change the time when playing back!

	return time;
}

/**
 * Instantaneously move the mouse cursor without creating a motion event.
 *
 * SDL_WarpMouse() *will* create a mousemotion event, which we do not want. As a
 * workaround, we store the delta in m_mouse_compensate_warp and use that to
 * eliminate the motion event in poll_event()
 * \todo Should this method have to care about playback at all???
 *
 * \param position The new mouse position
 */
void WLApplication::warp_mouse(const Point position)
{
	m_mouse_position = position;

	if (not journal->is_playingback()) { //  don't warp anything during playback
		Point cur_position;
		SDL_GetMouseState(&cur_position.x, &cur_position.y);
		if (cur_position != position) {
			m_mouse_compensate_warp += cur_position - position;
			SDL_WarpMouse(position.x, position.y);
		}
	}
}

/**
 * Changes input grab mode.
 *
 * This makes sure that the mouse cannot leave our window (and also that we get
 * mouse/keyboard input nearly unmodified, but we don't really care about that).
 *
 * \note This also cuts out any mouse-speed modifications that a generous window
 * manager might be doing.
 */
void WLApplication::set_input_grab(bool grab)
{
	if (journal->is_playingback())
		return; // ignore in playback mode

	if (grab) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
	} else {
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		warp_mouse(m_mouse_position); //TODO: is this redundant?
	}
}

/**
 * Initialize the graphics subsystem (or shutdown, if system == GFXSYS_NONE)
 * with the given resolution.
 * Throws an exception on failure.
 *
 * \note Because of the way pictures are handled now, this function must not be
 * called while UI elements are active.
 *
 * \todo Ensure that calling this with active UI elements does barf
 * \todo Document parameters
 */
void WLApplication::init_graphics
	(int32_t const w, int32_t const h, int32_t const bpp,
	 bool const fullscreen, bool const hw_improvements, bool const double_buffer)
{
	if
		(w == m_gfx_w && h == m_gfx_h &&
		 fullscreen == m_gfx_fullscreen &&
		 hw_improvements == m_gfx_hw_improvements &&
		 double_buffer == m_gfx_double_buffer)
		return;

	delete g_gr;
	g_gr = 0;

	m_gfx_w = w;
	m_gfx_h = h;
	m_gfx_fullscreen = fullscreen;
	m_gfx_hw_improvements = hw_improvements;
	m_gfx_double_buffer = double_buffer;

	// If we are not to be shut down
	if (w && h) {
		g_gr = new Graphic(w, h, bpp, fullscreen, hw_improvements, double_buffer);
	}
}

/**
 * Read the config file, parse the commandline and give all other internal
 * parameters sensible default values
 */
bool WLApplication::init_settings() {

	//create a journal so that handle_commandline_parameters can open the
	//journal files
	journal = new Journal();

	//read in the configuration file
	g_options.read("config", "global");
	Section & s = g_options.pull_section("global");

	// Set Locale and grab default domain
	i18n::set_locale(s.get_string("language", ""));
	i18n::grab_textdomain("widelands");

	//then parse the commandline - overwrites conffile settings
	handle_commandline_parameters();

	set_input_grab(s.get_bool("inputgrab", false));
	set_mouse_swap(s.get_bool("swapmouse", false));

	m_gfx_fullscreen = s.get_bool("fullscreen", false);
	m_gfx_hw_improvements = s.get_bool("hw_improvements", false);
	m_gfx_double_buffer = s.get_bool("double_buffer", false);

	// KLUDGE!
	// Without this the following config options get dropped by check_used().
	// Profile needs support for a Syntax definition to solve this in a
	// sensible way
	s.get_int("xres");
	s.get_int("yres");
	s.get_int("border_snap_distance");
	s.get_int("maxfps");
	s.get_int("panel_snap_distance");
	s.get_int("speed_of_new_game");
	s.get_int("autosave");
	s.get_bool("single_watchwin");
	s.get_bool("auto_roadbuild_mode");
	s.get_bool("workareapreview");
	s.get_bool("nozip");
	s.get_bool("snap_windows_only_when_overlapping");
	s.get_bool("dock_windows_to_edges");
	s.get_string("nickname");
	s.get_string("lasthost");
	s.get_string("servername");
	s.get_string("realname");
	s.get_string("ui_font");
	// KLUDGE!

	return true;
}

/**
 * Remember the last settings: write them into the config file
 */
void WLApplication::shutdown_settings()
{
	// To be proper, release our textdomain
	i18n::release_textdomain();

	try { //  overwrite the old config file
		g_options.write("config", true);
	} catch (std::exception const & e) {
		log("WARNING: could not save configuration: %s\n", e.what());
	} catch (...)                      {
		log("WARNING: could not save configuration");
	}

	assert(journal);
	delete journal;
	journal = 0;
}

/**
 * Start the hardware: switch to graphics mode, start sound handler
 *
 * \pre The locale must be known before calling this
 *
 * \return true if there were no fatal errors that prevent the game from running
 */
bool WLApplication::init_hardware() {
	Uint32 sdl_flags = 0;
	Section & s = g_options.pull_section("global");

	//Start the SDL core
	sdl_flags =
		SDL_INIT_VIDEO
		|
		(s.get_bool("coredump", false) ? SDL_INIT_NOPARACHUTE : 0);

	//  NOTE Enable a workaround for bug #1784815, caused by SDL, which thinks
	//  NOTE that it is perfectly fine for a library to tamper with the user's
	//  NOTE privacy/powermanagement settings on the sly. The workaround was
	//  NOTE introduced in SDL 1.2.13, so it will not work for older versions.
	//  NOTE -> there is no such stdlib-function on win32
	#ifndef WIN32
	setenv("SDL_VIDEO_ALLOW_SCREENSAVER", "1", 0);
	#endif

	//try all available video drivers till we find one that matches
	std::vector<std::string> videomode;
	int result = -1;

	//add default video mode
#ifdef linux
	videomode.push_back("x11");
#elif WIN32
	videomode.push_back("windib");
#elif __APPLE__
	videomode.push_back("Quartz");
#endif

	//add experimental video modes
	if (m_gfx_hw_improvements) {
#ifdef linux
		videomode.push_back("svga");
		videomode.push_back("fbcon");
		videomode.push_back("directfb");
		videomode.push_back("dga");
#elif WIN32
		videomode.push_back("directx");
#endif
	}

	//if a video mode is given on the command line, add that one first
	const char * videodrv;
	videodrv = getenv("SDL_VIDEODRIVER");
	if (videodrv) {
		log("Also adding video driver %s\n", videodrv);
		videomode.push_back(videodrv);
	}

	char videodrvused[26];
	strcpy(videodrvused, "SDL_VIDEODRIVER=\0");
	wout << videodrvused << "&" << std::endl;
	for (int i = videomode.size() - 1; result == -1 && i >= 0; --i) {
		strcpy(videodrvused + 16, videomode[i].c_str());
		videodrvused[16 + videomode[i].size()] = '\0';
		putenv(videodrvused);
		//SDL_VideoDriverName(videodrvused, 16);
		videodrv = getenv("SDL_VIDEODRIVER");
		log
			("Graphics: Trying Video driver: %i %s %s\n",
			 i, videomode[i].c_str(), videodrvused);
		result = SDL_Init(sdl_flags);
	}

	if (result == -1)
		throw wexception
			("Failed to initialize SDL, no valid video driver: %s",
			 SDL_GetError());

	SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableUNICODE(1); //needed by helper.h:is_printable()
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	uint32_t xres = s.get_int("xres", XRES);
	uint32_t yres = s.get_int("yres", YRES);

	init_graphics
		(xres, yres, s.get_int("depth", 16),
		 m_gfx_fullscreen, m_gfx_hw_improvements, m_gfx_double_buffer);

	// Start the audio subsystem
	// must know the locale before calling this!
	g_sound_handler.init(); //  FIXME memory leak!

	return true;
}

/**
 * Shut the hardware down: stop graphics mode, stop sound handler
 */
void WLApplication::shutdown_hardware()
{
	g_sound_handler.shutdown();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	if (g_gr)
		wout
			<<
			"WARNING: Hardware shutting down although graphics system is still "
			"alive!"
			<< endl;
	init_graphics(0, 0, 0, false, false, false);

	SDL_Quit();
}

/**
 * Parse the commandline and translate the options into name/value pairs
 *
 * The format for commandline parameters is --paramname[=value], that means:
 * \li starts with "--", i.e. all parameters are "long options"
 * \li arguments are passed with "="
 *
 * \param argc The number of command line arguments
 * \param argv Array of command line arguments
 */
void WLApplication::parse_commandline(const int argc, const char **argv)
{
	//TODO: EXENAME gets written out on windows!
	m_commandline["EXENAME"] = argv[0];

	for (int i = 1; i < argc; ++i) {
		std::string opt = argv[i];
		std::string value;
		SSS_T pos;

		//are we looking at an option at all?
		if (opt.substr(0, 2) == "--") {
			//yes. remove the leading "--", just for cosmetics
			opt.erase(0, 2);
		} else {
			throw Parameter_error();
		}

		//look if this option has a value
		pos = opt.find("=");

		if (pos == std::string::npos) { //  if no equals sign found
			value = "";
		} else {
			//extract option value
			value = opt.substr(pos + 1, opt.size() - pos);

			//remove value from option name
			opt.erase(pos, opt.size() - pos);
		}

		m_commandline[opt] = value;
	}
}

/**
 * Parse the command line given in m_commandline
 *
 * \return false if there were errors during parsing \e or if "--help"
 * was given,
 * true otherwise.
*/
void WLApplication::handle_commandline_parameters() throw (Parameter_error)
{
	if (m_commandline.count("help") || m_commandline.count("version")) {
		throw Parameter_error(); //no message on purpose
	}
	if (m_commandline.count("logfile")) {
		m_logfile = m_commandline["logfile"];
		std::cerr << "Redirecting log target to: " <<  m_logfile << std::endl;
		if (m_logfile.size() != 0) {
			//FIXME (very small) memory leak of 1 ofstream;
			//swaw the buffers (internally) of the file and wout
			std::ofstream * widelands_out = new std::ofstream(m_logfile.c_str());
			std::streambuf * logbuf = widelands_out->rdbuf();
			wout.rdbuf(logbuf);
		} else {
			//wout = std::cout;
		}
		m_commandline.erase("logfile");
	}
	if (m_commandline.count("nosound")) {
		g_sound_handler.m_nosound = true;
		m_commandline.erase("nosound");
	}
	if (m_commandline.count("nozip")) {
		g_options.pull_section("global").create_val("nozip", "true");
		m_commandline.erase("nozip");
	}
	if (m_commandline.count("hw_improvement")) {
		if (m_commandline["hw_improvement"].compare("0") == 0) {
			g_options.pull_section("global").create_val("hw_improvement", "false");
		} else if (m_commandline["hw_improvement"].compare("1") == 0) {
			g_options.pull_section("global").create_val("hw_improvement", "true");
		} else {
			log ("Invalid option hw_improvement=[0|1]\n");
		}
		m_commandline.erase("hw_improvement");
	}
	if (m_commandline.count("double_buffer")) {
		if (m_commandline["double_buffer"].compare("0") == 0) {
			g_options.pull_section("global").create_val("double_buffer", "false");
		} else if (m_commandline["double_buffer"].compare("1") == 0) {
			g_options.pull_section("global").create_val("double_buffer", "true");
		} else {
			log ("Invalid double_buffer=[0|1]\n");
		}
		m_commandline.erase("double_buffer");
	}
	if (m_commandline.count("datadir")) {
		log ("Adding directory: %s\n", m_commandline["datadir"].c_str());
		g_fs->AddFileSystem(FileSystem::Create(m_commandline["datadir"]));
		m_default_datadirs = false;
		m_commandline.erase("datadir");
	}

	if (m_commandline.count("double")) {
#ifdef DEBUG
#ifndef WIN32
		init_double_game();
#else
		wout << _("\nSorry, no double-instance debugging on WIN32.\n\n");
#endif
#else
		wout << _("--double is disabled. This is not a debug build!") << endl;
#endif

		m_commandline.erase("double");
	}

	if (m_commandline.count("editor")) {
		m_filename = m_commandline["editor"];
		if (m_filename.size() and *m_filename.rbegin() == '/')
			m_filename.erase(m_filename.size() - 1);
		m_editor_commandline = true;
		m_game_type = EDITOR;
		m_commandline.erase("editor");
	}

	if (m_commandline.count("loadgame")) {
		m_filename = m_commandline["loadgame"];
		if (m_filename.empty())
			throw wexception("empty value of command line parameter --loadgame");
		if (*m_filename.rbegin() == '/')
			m_filename.erase(m_filename.size() - 1);
		m_game_type = LOADGAME;
		m_commandline.erase("loadgame");
	}

	if (m_commandline.count("scenario")) {
		m_filename = m_commandline["scenario"];
		if (m_filename.empty())
			throw wexception("empty value of command line parameter --scenario");
		if (*m_filename.rbegin() == '/')
			m_filename.erase(m_filename.size() - 1);
		m_game_type = SCENARIO;
		m_commandline.erase("scenario");
	}
#if HAVE_GGZ
	if (m_commandline.count("dedicated")) {
		m_filename = m_commandline["dedicated"];
		if (m_filename.empty())
			throw wexception("empty value of commandline parameter --dedicated");
		if (*m_filename.rbegin() == '/')
			m_filename.erase(m_filename.size() - 1);
		m_game_type = GGZ;
		m_commandline.erase("dedicated");
	}
#endif
	//Note: it should be possible to record and playback at the same time,
	//but why would you?
	if (m_commandline.count("record")) {
		if (m_commandline["record"].empty())
			throw Parameter_error("ERROR: --record needs a filename!");

		try {
			journal->start_recording(m_commandline["record"]);
		} catch (Journalfile_error e) {
			wout << "Journal file error: " << e.what() << endl;
		}

		m_commandline.erase("record");
	}

	if (m_commandline.count("playback")) {
		if (m_commandline["playback"].empty())
			throw Parameter_error("ERROR: --playback needs a filename!");

		try {
			journal->start_playback(m_commandline["playback"]);
		}
		catch (Journalfile_error e) {
			wout << "Journal file error: " << e.what() << endl;
		}

		m_commandline.erase("playback");
	}

	//If it hasn't been handled yet it's probably an attempt to
	//override a conffile setting
	//With typos, this will create invalid config settings. They
	//will be taken care of (==ignored) when saving the options

	const std::map<std::string, std::string>::const_iterator commandline_end =
		m_commandline.end();
	for
		(std::map<std::string, std::string>::const_iterator it =
		 	m_commandline.begin();
		 it != commandline_end;
		 ++it)
	{
		//TODO: barf here on unknown option; the list of known options
		//TODO: needs to be centralized

		g_options.pull_section("global").create_val
			(it->first.c_str(), it->second.c_str());
	}
}

/**
 * Print usage information
 */
void WLApplication::show_usage()
{
	i18n::Textdomain textdomain("widelands"); //  uses system standard language

	wout << _("This is Widelands-") << build_id() << '(' << build_type();
	wout << ")\n\n";
	wout << _("Usage: widelands <option0>=<value0> ... <optionN>=<valueN>\n\n");
	wout << _("Options:\n\n");
	wout
		<<
		_
			(" --<config-entry-name>=value overwrites any config file setting\n\n"
			 " --logfile=FILENAME   Log output to file FILENAME instead of \n"
			 "                      terminal output\n"
			 " --datadir=DIRNAME    Use specified direction for the widelands\n"
			 "                      data files\n"
			 " --record=FILENAME    Record all events to the given filename for\n"
			 "                      later playback\n"
			 " --playback=FILENAME  Playback given filename (see --record)\n\n"
			 " --coredump=[yes|no]  Generates a core dump on segfaults instead\n"
			 "                      of using the SDL\n"
			 " --language=[de_DE|sv_SE|...]\n"
			 "                      The locale to use.\n"
			 "\n")
		<<
		_
			("Sound options:\n"
			 " --nosound            Starts the game with sound disabled.\n"
			 " --disable_fx         Disable sound effects.\n"
			 " --disable_music      Disable music.\n"
			 "\n"
			 " --nozip              Do not save files as binary zip archives.\n"
			 "\n"
			 " --editor             Directly starts the Widelands editor.\n"
			 "                      You can add a =FILENAME to directly load\n"
			 "                      the map FILENAME in editor.\n"
			 " --scenario=FILENAME  Directly starts the map FILENAME as scenario\n"
			 "                      map.\n"
			 " --loadgame=FILENAME  Directly loads the savegame FILENAME.\n"
			 " --speed_of_new_game  The speed that the new game will run at\n"
			 "                      when started, with factor 1000 (0 is pause,\n"
			 "                      1000 is normal speed).\n"
			 " --auto_roadbuild_mode=[yes|no]\n"
			 "                      Whether to enter roadbuilding mode\n"
			 "                      automatically after placing a flag that is\n"
			 "                      not connected to a road.\n"
			 " --write_HTML=[yes|no]\n"
			 "                      Write HTML-helpfiles for parsed game data.\n"
			 "\n"
			 "Graphic options:\n"
			 " --fullscreen=[yes|no]\n"
			 "                      Whether to use the whole display for the\n"
			 "                      game screen.\n"
			 " --depth=[16|32]      Color depth in number of bits per pixel.\n"
			 " --xres=[...]         Width of the window in pixel.\n"
			 " --yres=[...]         Height of the window in pixel.\n"
			 " --hw_improvements=[0|1]\n"
			 "                      Activate hardware acceleration\n"
			 "                      *HIGHLY EXPERIMENTAL*\n"
			 " --double_buffer=[0|1]\n"
			 "                      Enables double buffering\n"
			 "                      *HIGHLY EXPERIMENTAL*\n"
			 "\n"
			 "Options for the internal window manager:\n"
			 " --border_snap_distance=[0 ...]\n"
			 "                      Move a window to the edge of the screen\n"
			 "                      when the edge of the window comes within\n"
			 "                      this distance from the edge of the screen.\n"
			 " --dock_windows_to_edges=[yes|no]\n"
			 "                      Eliminate a window's border towards the\n"
			 "                      edge of the screen when the edge of the\n"
			 "                      window is next to the edge of the screen.\n"
			 " --panel_snap_distance=[0 ...]\n"
			 "                      Move a window to the edge of the panel when\n"
			 "                      the edge of the window comes within this\n"
			 "                      distance from the edge of the panel.\n"
			 " --snap_windows_only_when_overlapping=[yes|no]\n"
			 "                      Only move a window to the edge of a panel\n"
			 "                      if the window is overlapping with the\n"
			 "                      panel.\n"
			 "\n");
#ifdef DEBUG
#ifndef WIN32
	wout
		<<
		_
			(" --double             Start the game twice (for localhost network\n"
			 "                      testing)\n\n");
#endif
#endif
	wout << _(" --help               Show this help\n") << endl;
	wout
		<<
		_
			("Bug reports? Suggestions? Check out the project website:\n"
			 "        http://www.sourceforge.net/projects/widelands\n\n"
			 "Hope you enjoy this game!\n\n");
}

#ifdef DEBUG
#ifndef WIN32
/**
 * Fork off a second game to test network gaming
 *
 * \warning You must call this \e before any hardware initialization - most
 * notably before \ref SDL_Init()
 */
void WLApplication::init_double_game ()
{
	if (pid_me != 0)
		return;

	pid_me = getpid();
	pid_peer = fork();
	//TODO: handle fork errors

	assert (pid_peer >= 0);

	if (pid_peer == 0) {
		pid_peer = pid_me;
		pid_me   = getpid();

		may_run = 1;
	}

	signal (SIGUSR1, signal_handler);

	atexit (quit_handler);
}

/**
 * On SIGUSR1, allow ourselves to continue running
 */
void WLApplication::signal_handler(int32_t) {++may_run;}

/**
 * Kill the other instance when exiting
 *
 * \todo This works but is not very clean (each process killing each other)
 */
void WLApplication::quit_handler()
{
	kill (pid_peer, SIGTERM);
	sleep (2);
	kill (pid_peer, SIGKILL);
}

/**
 * Voluntarily yield to the second Widelands process. This was implemented
 * because some machines got horrible responsiveness when using --double, so we
 * forced good reponsiveness by using cooperative multitasking (between the two
 * Widelands instances, that is)
 */
void WLApplication::yield_double_game()
{
	if (pid_me == 0)
		return;

	if (may_run > 0) {
		--may_run;
		kill (pid_peer, SIGUSR1);
	}

	if (may_run == 0)
		usleep (500000);

	// using sleep instead of pause avoids a race condition
	// and a deadlock during connect
}
#endif
#endif

/**
 * Run the main menu
 */
void WLApplication::mainmenu()
{
	std::string messagetitle;
	std::string message;

	for (;;) {
		Fullscreen_Menu_Main mm;

		if (message.size()) {
			log("\n%s\n%s\n", messagetitle.c_str(), message.c_str());

			UI::MessageBox mmb
				(&mm,
				 messagetitle,
				 message,
				 UI::MessageBox::OK);
			mmb.set_align(Align_Left);
			mmb.run();

			message.clear();
			messagetitle.clear();
		}

		try {
			switch (mm.run()) {
			case Fullscreen_Menu_Main::mm_singleplayer:
				mainmenu_singleplayer();
				break;
			case Fullscreen_Menu_Main::mm_multiplayer:
				mainmenu_multiplayer();
				break;
			case Fullscreen_Menu_Main::mm_replay:
				replay();
				break;
			case Fullscreen_Menu_Main::mm_options: {
				Section & s = g_options.pull_section("global");
				Options_Ctrl om(s);
				break;
			}
			case Fullscreen_Menu_Main::mm_readme: {
				Fullscreen_Menu_FileView ff("txts/README");
				ff.run();
				break;
			}
			case Fullscreen_Menu_Main::mm_license: {
				Fullscreen_Menu_FileView ff("txts/COPYING");
				ff.run();
				break;
			}
			case Fullscreen_Menu_Main::mm_editor:
				mainmenu_editor();
				break;
			default:
			case Fullscreen_Menu_Main::mm_exit:
				return;
			}
		} catch (warning const & e) {
			messagetitle = _("Warning: ");
			messagetitle += e.title();
			message = e.what();
		} catch (std::exception const & e) {
			messagetitle = _("Unexpected error during the game");
			message = e.what();
			message +=
				_
					("\n\nPlease report this problem to help us improve Widelands. "
					 "You will find related messages in the standard output "
					 "(stdout.txt on Windows). You are using build ");
			message += build_id() + "(" + build_type() + ")";
			message +=
				_
					(". Please add this information to your report.\n\n"
					 "Widelands attempts to create a savegame when errors occur "
					 "during the game. It is often - though not always - possible "
					 "to load it and continue playing.\n");
		}

	}
}

/**
 * Run the singleplayer menu
 */
void WLApplication::mainmenu_singleplayer()
{
	//  This is the code returned by UI::Panel::run() when the panel is dying.
	//  Make sure that the program exits when the window manager says so.
	compile_assert(Fullscreen_Menu_SinglePlayer::Back == UI::Panel::dying_code);

	for (;;) {
		int32_t code;
		{
			Fullscreen_Menu_SinglePlayer single_player_menu;
			code = single_player_menu.run();
		}
		switch (code) {
		case Fullscreen_Menu_SinglePlayer::Back:
			return;
		case Fullscreen_Menu_SinglePlayer::New_Game:
			if (new_game())
				return;
			break;
		case Fullscreen_Menu_SinglePlayer::Load_Game:
			if (load_game())
				return;
			break;
		case Fullscreen_Menu_SinglePlayer::Campaign:
			if (campaign_game())
				return;
			break;
		default:
			assert(false);
		}
	}

}

/**
 * Run the multiplayer menu
 */
void WLApplication::mainmenu_multiplayer()
{
#if WIN32
	//  The Winsock2 library needs to get called through WSAStartup, to initiate
	//  the use of the Winsock DLL by Widelands.
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
		throw wexception("initialization of Wsock2-library failed");
#endif // WIN32

	int32_t menu_result = Fullscreen_Menu_NetSetupLAN::JOINGAME; // dummy init;
	for (;;) { // stay in menu until player clicks "back" button
		std::string playername;

#if HAVE_GGZ
		bool ggz = false;
		NetGGZ::ref().deinitcore(); // cleanup for reconnect to the metaserver
		{
			Fullscreen_Menu_MultiPlayer mp;
			menu_result = mp.run();
		}
		switch (menu_result) {
			case Fullscreen_Menu_MultiPlayer::Back:
				return;
			case Fullscreen_Menu_MultiPlayer::Metaserver:
				ggz = true;
				break;
			case Fullscreen_Menu_MultiPlayer::Lan:
				break;
			default:
				assert(false);
		}

		if (ggz) {
			// reinitalise in every run, else graphics look strange
			Fullscreen_Menu_NetSetupGGZ ns;
			menu_result = ns.run();
			playername = ns.get_playername();

			switch (menu_result) {
				case Fullscreen_Menu_NetSetupGGZ::HOSTGAME: {
					uint32_t mp = static_cast<uint32_t>(ns.get_maxplayers());
					NetGGZ::ref().set_local_maxplayers(mp);
					NetHost netgame(playername, true);
					netgame.run();
					NetGGZ::ref().deinitcore();
					break;
				}
				case Fullscreen_Menu_NetSetupGGZ::JOINGAME: {
					uint32_t const secs = time(0);
					while (!NetGGZ::ref().ip()) {
						NetGGZ::ref().data();
						if (10 < time(0) - secs)
							throw warning
								(_("Connection timeouted"), "%s",
								 _
								 	("Widelands has not been able to get the IP "
								 	 "address of the server in time.\n"
								 	 "There seems to be a network problem, either on "
								 	 "your side or on side\n"
								 	 "of the server.\n"));
					}
					std::string ip = NetGGZ::ref().ip();

					// Handle "IPv4 compatible" IPv6 adresses returned by ggzd
					if (ip.size() > 7 && ip.substr(0, 7) == "::ffff:")
					{
						ip = ip.substr(7, ip.size() - 7);
						log("GGZClient ## cutted IPv6 adress: %s\n", ip.c_str());
					}

					IPaddress peer;
					if (hostent * const he = gethostbyname(ip.c_str())) {
						peer.host =
							(reinterpret_cast<in_addr *>(he->h_addr_list[0]))->s_addr;
						peer.port = htons(WIDELANDS_PORT);
					} else
						throw warning
							(_("Connection problem"), "%s",
							 _
							 	("Widelands has not been able to connect to the "
							 	 "host."));
					SDLNet_ResolveHost (&peer, ip.c_str(), WIDELANDS_PORT);

					NetClient netgame(&peer, playername, true);
					netgame.run();
					NetGGZ::ref().deinitcore();
					break;
				}
				default:
					break;
			}
		}

#else
		// If compiled without ggz support, only lan-netsetup will be visible
		if (menu_result == Fullscreen_Menu_NetSetupLAN::CANCEL)
			return;
#endif // HAVE_GGZ

		else {
			// reinitalise in every run, else graphics look strange
			Fullscreen_Menu_NetSetupLAN ns;
			menu_result = ns.run();
			playername = ns.get_playername();
			uint32_t addr;
			uint16_t port;
			bool const host_address = ns.get_host_address(addr, port);

			switch (menu_result) {
				case Fullscreen_Menu_NetSetupLAN::HOSTGAME: {
					NetHost netgame(playername);
					netgame.run();
					break;
				}
				case Fullscreen_Menu_NetSetupLAN::JOINGAME: {
					IPaddress peer;

					if (not host_address)
						throw warning
							("Invalid Address", "%s",
							 _("The address of the game server is invalid"));

					peer.host = addr;
					peer.port = port;

					NetClient netgame(&peer, playername);
					netgame.run();
					break;
				}
				default:
					break;
			}
		}
	}
}

void WLApplication::mainmenu_editor()
{
	//  This is the code returned by UI::Panel::run() when the panel is dying.
	//  Make sure that the program exits when the window manager says so.
	compile_assert(Fullscreen_Menu_Editor::Back == UI::Panel::dying_code);

	for (;;) {
		int32_t code;
		{
			Fullscreen_Menu_Editor editor_menu;
			code = editor_menu.run();
		}
		switch (code) {
		case Fullscreen_Menu_Editor::Back:
			return;
		case Fullscreen_Menu_Editor::New_Map:
			Editor_Interactive::run_editor(m_filename);
			return;
		case Fullscreen_Menu_Editor::Load_Map: {
			std::string filename;
			{
				Fullscreen_Menu_Editor_MapSelect emsm;
				if (emsm.run() <= 0)
					break;

				filename = emsm.get_map();
			}
			Editor_Interactive::run_editor(filename.c_str());
			return;
		}
		default:
			assert(false);
		}
	}
}

// The settings provider for normal singleplayer games:
// The user can change everything, except that they are themselves human.
struct SinglePlayerGameSettingsProvider : public GameSettingsProvider {
	SinglePlayerGameSettingsProvider() {
		Widelands::Tribe_Descr::get_all_tribe_infos(s.tribes);
		s.scenario = false;
		s.multiplayer = false;
		s.playernum = 0;
	}

	virtual void setScenario(bool const set) {s.scenario = set;}

	virtual GameSettings const & settings() {return s;}

	virtual bool canChangeMap() {return true;}
	virtual bool canChangePlayerState(uint8_t number) {
		return (!s.scenario & (number != s.playernum));
	}
	virtual bool canChangePlayerTribe(uint8_t) {return !s.scenario;}
	virtual bool canChangePlayerInit (uint8_t) {return true;}

	virtual bool canLaunch() {
		return s.mapname.size() != 0 && s.players.size() >= 1;
	}

	virtual std::string getMap() {
		return s.mapfilename;
	}

	virtual void setMap
		(std::string const &       mapname,
		 std::string const &       mapfilename,
		 uint32_t            const maxplayers,
		 bool                const savegame)
	{
		s.mapname = mapname;
		s.mapfilename = mapfilename;
		s.savegame = savegame;

		uint32_t oldplayers = s.players.size();
		s.players.resize(maxplayers);

		while (oldplayers < maxplayers) {
			PlayerSettings & player = s.players[oldplayers];
			player.state = (oldplayers == 0) ? PlayerSettings::stateHuman :
				PlayerSettings::stateComputer;
			player.tribe                = s.tribes.at(0).name;
			player.initialization_index = 0;
			char buf[200];
			snprintf(buf, sizeof(buf), "%s %u", _("Player"), oldplayers + 1);
			player.name = buf;
			// Set default computerplayer ai type
			if (player.state == PlayerSettings::stateComputer) {
				Computer_Player::ImplementationVector const & impls =
					Computer_Player::getImplementations();
				if (impls.size() > 1)
					player.ai = impls.at(0)->name;
			}
			++oldplayers;
		}
	}

	virtual void setPlayerState
		(uint8_t const number, PlayerSettings::State state)
	{
		if (number == s.playernum || number >= s.players.size())
			return;

		if (state == PlayerSettings::stateOpen)
			state = PlayerSettings::stateComputer;

		s.players[number].state = state;
	}

	virtual void setPlayerAI(uint8_t const number, std::string const & ai) {
		if (number >= s.players.size())
			return;

		s.players[number].ai = ai;
	}
	virtual void nextPlayerState(uint8_t const number) {
		if (number == s.playernum || number >= s.players.size())
			return;

		Computer_Player::ImplementationVector const & impls =
			Computer_Player::getImplementations();
		if (impls.size() > 1) {
			Computer_Player::ImplementationVector::const_iterator it =
				impls.begin();
			do {
				++it;
				if ((*(it - 1))->name == s.players[number].ai)
					break;
			} while (it != impls.end());
			if (it == impls.end())
				it = impls.begin();
			s.players[number].ai = (*it)->name;
		}

		s.players[number].state = PlayerSettings::stateComputer;
	}

	virtual void setPlayerTribe(uint8_t const number, std::string const & tribe)
	{
		if (number >= s.players.size())
			return;

		PlayerSettings & player = s.players[number];
		container_iterate_const(std::vector<TribeBasicInfo>, s.tribes, i)
			if (i.current->name == player.tribe) {
				s.players[number].tribe = tribe;
				if
					(i.current->initializations.size()
					 <=
					 player.initialization_index)
					player.initialization_index = 0;
			}
	}
	virtual void setPlayerInit(uint8_t const number, uint8_t const index) {
		if (number >= s.players.size())
			return;

		container_iterate_const(std::vector<TribeBasicInfo>, s.tribes, i)
			if (i.current->name == s.players[number].tribe) {
				if (index < i.current->initializations.size())
					s.players[number].initialization_index = index;
				return;
			}
		assert(false);
	}

	virtual void setPlayerName(uint8_t const number, std::string const & name) {
		if (number >= s.players.size())
			return;

		s.players[number].name = name;
	}

	virtual void setPlayer(uint8_t const number, PlayerSettings const ps) {
		if (number >= s.players.size())
			return;

		s.players[number] = ps;
	}

	virtual void setPlayerNumber(int32_t const number) {
		if (number >= static_cast<int32_t>(s.players.size()))
			return;

		s.playernum = number;
	}

	virtual void setPlayerReady
		(uint8_t const, PlayerSettings::ReadyState const)
	{
		//ignore, a single player is always ready to start the game if he wants to
	}

	virtual bool getPlayerReady(uint8_t) {
		//a single player is always ready
		return true;
	}

private:
	GameSettings s;
};

/**
 * Handle the "New game" menu option: Configure a single player game and
 * run it.
 *
 * \return @c true if a game was played, @c false if the player pressed Back
 * or aborted the game setup via some other means.
 */
bool WLApplication::new_game()
{
	SinglePlayerGameSettingsProvider sp;
	Fullscreen_Menu_LaunchGame lgm(&sp);
	const int32_t code = lgm.run();
	Widelands::Game game;

	if (code > 2) // code > 2 is a multi player savegame.
		throw wexception("Something went wrong! a savegame was selected");
	if (code <= 0)
		return false;
	if (code == 2) { // scenario
		try {
			game.run_splayer_scenario_direct(sp.getMap().c_str());
		} catch (...) {
			emergency_save(game);
			throw;
		}
	} else { // normal singleplayer
		uint8_t const pn = sp.settings().playernum + 1;
		boost::scoped_ptr<GameController> ctrl
			(GameController::createSinglePlayer(game, true, pn));
		try {
			UI::ProgressWindow loaderUI;
			std::vector<std::string> tipstext;
			tipstext.push_back("general_game");
			tipstext.push_back("singleplayer");
			std::string tribe = sp.getPlayersTribe();
			if (!tribe.empty())
				tipstext.push_back(tribe);
			GameTips tips (loaderUI, tipstext);

			loaderUI.step(_("Preparing game"));

			game.set_game_controller(ctrl.get());
			game.set_ibase
				(new Interactive_Player
				 	(game, g_options.pull_section("global"), pn, false, false));
			game.init_newgame(loaderUI, sp.settings());
			game.run(loaderUI, Widelands::Game::NewNonScenario);
		} catch (...) {
			emergency_save(game);
			throw;
		}
	}
	return true;
}


/**
 * Handle the "Load game" menu option:
 * Configure a single player game, care about player position and run it.
 *
 * \return @c true if a game was loaded, @c false if the player pressed Back
 * or aborted the game setup via some other means.
 */
bool WLApplication::load_game()
{
	Widelands::Game game;
	std::string filename;

	Fullscreen_Menu_LoadGame ssg(game);
	if (ssg.run() > 0)
		filename = ssg.filename();
	else
		return false;

	try {
		if (game.run_load_game(filename))
			return true;
	} catch (...) {
		emergency_save(game);
		throw;
	}
	return false; // keep compiler silent.
}


/**
 * Handle the "Campaign" menu option:
 * Show campaign UI, let player select scenario and run it.
 *
 * \return @c true if a scenario was played, @c false if the player pressed Back
 * or aborted the game setup via some other means.
 */
bool WLApplication::campaign_game()
{
	Widelands::Game game;
	std::string filename;
	for (;;) { // Campaign UI - Loop
		int32_t campaign;
		{ //  First start UI for selecting the campaign
			Fullscreen_Menu_CampaignSelect select_campaign;
			if (select_campaign.run() > 0)
				campaign = select_campaign.get_campaign();
			else { //  back was pressed
				filename = "";
				break;
			}
		}
		// Than start UI for the selected campaign
		Fullscreen_Menu_CampaignMapSelect select_campaignmap;
		select_campaignmap.set_campaign(campaign);
		if (select_campaignmap.run() > 0) {
			filename = select_campaignmap.get_map();
			break;
		}
	}
	try {
		// Load selected campaign-map-file
		if (!filename.empty())
			return game.run_splayer_scenario_direct(filename.c_str());
	} catch (...) {
		emergency_save(game);
		throw;
	}
	return false;
}

struct ReplayGameController : public GameController {
	ReplayGameController(Widelands::Game & game, std::string const & filename) :
		m_game     (game),
		m_lastframe(WLApplication::get()->get_time()),
		m_time     (m_game.get_gametime()),
		m_speed    (1000)
	{
		m_game.set_game_controller(this);

		// We have to create an empty map, otherwise nothing will load properly
		game.set_map(new Widelands::Map);
		m_replayreader.reset(new Widelands::ReplayReader(m_game, filename));
	}

	struct Cmd_ReplayEnd : public Widelands::Command {
		Cmd_ReplayEnd (int32_t const _duetime) : Widelands::Command(_duetime) {}
		virtual void execute (Widelands::Game & game) {
			game.gameController()->setDesiredSpeed(0);
			UI::MessageBox mmb
				(game.get_ibase(),
				 _("End of replay"),
				 _
				 	("The end of the replay has been reached and the game has "
				 	 "been paused. You may unpause the game and continue watching "
				 	 "if you want to."),
				 UI::MessageBox::OK);
			mmb.run();
		}
		virtual uint8_t id() const {return QUEUE_CMD_REPLAYEND;}
	};

	void think() {
		int32_t curtime = WLApplication::get()->get_time();
		int32_t frametime = curtime - m_lastframe;
		m_lastframe = curtime;

		// prevent crazy frametimes
		if (frametime < 0)
			frametime = 0;
		else if (frametime > 1000)
			frametime = 1000;

		frametime = frametime * m_speed / 1000;

		m_time = m_game.get_gametime() + frametime;

		if (m_replayreader) {
			while
				(Widelands::Command * const cmd =
				 	m_replayreader->GetNextCommand(m_time))
				m_game.enqueue_command(cmd);

			if (m_replayreader->EndOfReplay()) {
				m_replayreader.reset(0);
				m_game.enqueue_command
					(new Cmd_ReplayEnd(m_time = m_game.get_gametime()));
			}
		}
	}

	__attribute__((noreturn)) void sendPlayerCommand(Widelands::PlayerCommand &)
	{
		throw wexception("Trying to send a player command during replay");
	}
	int32_t getFrametime() {
		return m_time - m_game.get_gametime();
	}
	std::string getGameDescription() {
		return "replay";
	}
	uint32_t realSpeed() {return m_speed;}
	uint32_t desiredSpeed() {return m_speed;}
	void setDesiredSpeed(uint32_t const speed) {m_speed = speed;}

private:
	Widelands::Game & m_game;
	boost::scoped_ptr<Widelands::ReplayReader> m_replayreader;
	int32_t m_lastframe;
	int32_t m_time;
	uint32_t m_speed;
};

/**
 * Show the replay menu and play a replay.
 */
void WLApplication::replay()
{
	std::string fname;

	{
		Fullscreen_Menu_LoadReplay rm;
		if (rm.run() <= 0)
			return;

		fname = rm.filename();
	}

	Widelands::Game game;
	try {
		UI::ProgressWindow loaderUI;
		std::vector<std::string> tipstext;
		tipstext.push_back("general_game");
		GameTips tips (loaderUI, tipstext);

		loaderUI.step(_("Loading..."));

		game.set_ibase
			(new Interactive_Spectator(game, g_options.pull_section("global")));
		game.set_write_replay(false);
		ReplayGameController rgc(game, fname);

		game.run(loaderUI, Widelands::Game::Loaded);
	} catch (...) {
		emergency_save(game);
		throw;
	}
}


/**
* Try to save the game instance if possible
 */
void WLApplication::emergency_save(Widelands::Game & game) {
	if (game.is_loaded()) {
		try {
			SaveHandler & save_handler = game.save_handler();
			std::string error;
			if
				(!
				 save_handler.save_game
				 	(game,
				 	 save_handler.create_file_name
				 	 	(save_handler.get_base_dir(), timestring()),
				 	 &error))
			{
				log("Emergency save failed: %s\n", error.c_str());
			}
		} catch (...) {
			log ("Emergency save failed");
			throw;
		}
	}
}
