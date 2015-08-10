/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wlapplication.h"

#include <cerrno>
#ifndef _WIN32
#include <csignal>
#endif
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <SDL_image.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/time_string.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "build_info.h"
#include "config.h"
#include "editor/editorinteractive.h"
#include "graphic/default_resolution.h"
#include "graphic/font_handler.h"
#include "graphic/font_handler1.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "helper.h"
#include "io/dedicated_log.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/game_settings.h"
#include "logic/map.h"
#include "logic/replay.h"
#include "logic/replay_game_controller.h"
#include "logic/single_player_game_controller.h"
#include "logic/single_player_game_settings_provider.h"
#include "logic/tribe.h"
#include "map_io/map_loader.h"
#include "network/internet_gaming.h"
#include "network/netclient.h"
#include "network/nethost.h"
#include "profile/profile.h"
#include "sound/sound_handler.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/campaign_select.h"
#include "ui_fsmenu/fileview.h"
#include "ui_fsmenu/internet_lobby.h"
#include "ui_fsmenu/intro.h"
#include "ui_fsmenu/launch_spg.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/main.h"
#include "ui_fsmenu/mapselect.h"
#include "ui_fsmenu/multiplayer.h"
#include "ui_fsmenu/netsetup_lan.h"
#include "ui_fsmenu/options.h"
#include "ui_fsmenu/singleplayer.h"
#include "wlapplication_messages.h"
#include "wui/game_tips.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"

#define MINIMUM_DISK_SPACE 250000000lu
#define SCREENSHOT_DIR "screenshots"

namespace {

/**
 * Shut the hardware down: stop graphics mode, stop sound handler
 */
void terminate(int) {
	log("Waited 5 seconds to close audio. There are some problems here, so killing Widelands."
			" Update your sound driver and/or SDL to fix this problem\n");
#ifndef _WIN32
	raise(SIGKILL);
#endif
}

/**
 * Returns the widelands executable path.
 */
std::string get_executable_directory()
{
	std::string executabledir;
#ifdef __APPLE__
	uint32_t buffersize = 0;
	_NSGetExecutablePath(nullptr, &buffersize);
	std::unique_ptr<char []> buffer(new char[buffersize]);
	int32_t check = _NSGetExecutablePath(buffer.get(), &buffersize);
	if (check != 0) {
		throw wexception ("could not find the path of the main executable");
	}
	executabledir = std::string(buffer.get());
	executabledir.resize(executabledir.rfind('/') + 1);
#endif
#ifdef __linux__
	char buffer[PATH_MAX];
	size_t size = readlink("/proc/self/exe", buffer, PATH_MAX);
	if (size <= 0) {
		throw wexception ("could not find the path of the main executable");
	}
	executabledir = std::string(buffer, size);
	executabledir.resize(executabledir.rfind('/') + 1);
#endif
#ifdef _WIN32
	char filename[_MAX_PATH + 1] = {0};
	GetModuleFileName(nullptr, filename, MAX_PATH);
	executabledir = filename;
	executabledir = executabledir.substr(0, executabledir.rfind('\\'));
#endif
	log("Widelands executable directory: %s\n", executabledir.c_str());
	return executabledir;
}

bool is_absolute_path(const std::string& path) {
	boost::regex re("^/|\\w:");
	return boost::regex_search(path.c_str(), re);
}

// Returns the absolute path of 'path' which might be relative.
std::string absolute_path_if_not_windows(const std::string& path)
{
#ifndef _WIN32
	char buffer[PATH_MAX];
	realpath(path.c_str(), buffer);
	log("Realpath: %s\n", buffer);
	return std::string(buffer);
#else
	return path;
#endif
}

// On Mac OS, we bundle the shared libraries that Widelands needs directly in
// the executable directory. This is so that SDL_Image and SDL_Mixer can load
// them dynamically. Unfortunately, linking them statically has led to problems
// in the past.
//
// Changing LD_LIBRARY_PATH does not work, so we resort to the hack of chdir()
// in the directory so that dlopen() finds the library.
void changedir_on_mac() {
#ifdef __APPLE__
	chdir(get_executable_directory().c_str());
#endif
}
}  // namespace

void WLApplication::setup_homedir() {
	//If we don't have a home directory don't do anything
	if (m_homedir.size()) {
		//assume some dir exists
		try {
			log ("Set home directory: %s\n", m_homedir.c_str());

			std::unique_ptr<FileSystem> home(new RealFSImpl(m_homedir));
			home->ensure_directory_exists(".");
			g_fs->set_home_file_system(home.release());
		} catch (const std::exception & e) {
			log("Failed to add home directory: %s\n", e.what());
		}
	} else {
		//TODO(unknown): complain
	}
}

WLApplication * WLApplication::the_singleton = nullptr;

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
 */
// TODO(unknown): Return a reference - the return value is always valid anyway
WLApplication * WLApplication::get(int const argc, char const * * argv) {
	if (the_singleton == nullptr)
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
WLApplication::WLApplication(int const argc, char const * const * const argv) :
m_commandline          (std::map<std::string, std::string>()),
m_game_type            (NONE),
m_mouse_swapped        (false),
m_faking_middle_mouse_button(false),
m_mouse_position       (0, 0),
m_mouse_locked         (0),
m_mouse_compensate_warp(0, 0),
m_should_die           (false),
#ifdef _WIN32
m_homedir(FileSystem::get_homedir() + "\\.widelands"),
#else
m_homedir(FileSystem::get_homedir() + "/.widelands"),
#endif
m_redirected_stdio(false)
{
	g_fs = new LayeredFileSystem();

	parse_commandline(argc, argv); //throws ParameterError, handled by main.cc

	if (m_commandline.count("homedir")) {
		log ("Adding home directory: %s\n", m_commandline["homedir"].c_str());
		m_homedir = m_commandline["homedir"];
		m_commandline.erase("homedir");
	}
#ifdef REDIRECT_OUTPUT
	if (!redirect_output())
		redirect_output(m_homedir);
#endif

	setup_homedir();
	init_settings();

	log("Adding directory: %s\n", m_datadir.c_str());
	g_fs->add_file_system(&FileSystem::create(m_datadir));

	init_language(); // search paths must already be set up
	changedir_on_mac();
	cleanup_replays();

	// handling of graphics
	init_hardware();

	// This might grab the input.
	refresh_graphics();

	if (TTF_Init() == -1)
		throw wexception
			("True Type library did not initialize: %s\n", TTF_GetError());

	UI::g_fh1 = UI::create_fonthandler(g_gr); // This will create the fontset, so loading it first.
	UI::g_fh = new UI::FontHandler();

	if (SDLNet_Init() == -1)
		throw wexception("SDLNet_Init failed: %s\n", SDLNet_GetError());

	// seed random number generator used for random tribe selection
	std::srand(time(nullptr));

	//make sure we didn't forget to read any global option
	g_options.check_used();
}

/**
 * Shut down all subsystems in an orderly manner
 */
// TODO(unknown): Handle errors that happen here!
WLApplication::~WLApplication()
{
	//Do use the opposite order of WLApplication::init()

	shutdown_hardware();
	shutdown_settings();

	assert(UI::g_fh);
	delete UI::g_fh;
	UI::g_fh = nullptr;

	assert(UI::g_fh1);
	delete UI::g_fh1;
	UI::g_fh1 = nullptr;

	SDLNet_Quit();

	TTF_Quit(); // TODO(unknown): not here

	assert(g_fs);
	delete g_fs;
	g_fs = nullptr;

	if (m_redirected_stdio)
	{
		std::cout.flush();
		fclose(stdout);
		std::cerr.flush();
		fclose(stderr);
	}

	SDL_Quit();
}

/**
 * The main loop. Plain and Simple.
 */
// TODO(unknown): Refactor the whole mainloop out of class \ref UI::Panel into here.
// In the future: push the first event on the event queue, then keep
// dispatching events until it is time to quit.
void WLApplication::run()
{
	// This also grabs the mouse cursor if so desired.
	refresh_graphics();

	if (m_game_type == EDITOR) {
		g_sound_handler.start_music("ingame");
		EditorInteractive::run_editor(m_filename, m_script_to_run);
	} else if (m_game_type == REPLAY)   {
		replay();
	} else if (m_game_type == LOADGAME) {
		Widelands::Game game;
		try {
			game.run_load_game(m_filename.c_str(), m_script_to_run);
		} catch (const Widelands::GameDataError & e) {
			log("Game not loaded: Game data error: %s\n", e.what());
		} catch (const std::exception & e) {
			log("Fatal exception: %s\n", e.what());
			emergency_save(game);
			throw;
		}
	} else if (m_game_type == SCENARIO) {
		Widelands::Game game;
		try {
			game.run_splayer_scenario_direct(m_filename.c_str(), m_script_to_run);
		} catch (const Widelands::GameDataError & e) {
			log("Scenario not started: Game data error: %s\n", e.what());
		} catch (const std::exception & e) {
			log("Fatal exception: %s\n", e.what());
			emergency_save(game);
			throw;
		}
	} else if (m_game_type == INTERNET) {
		Widelands::Game game;
		try {
			// disable sound completely
			g_sound_handler.nosound_ = true;

			// setup some details of the dedicated server
			Section & s = g_options.pull_section      ("global");
			const std::string & meta   = s.get_string ("metaserver",     INTERNET_GAMING_METASERVER.c_str());
			uint32_t            port   = s.get_natural("metaserverport", INTERNET_GAMING_PORT);
			const std::string & name   = s.get_string ("nickname",       "dedicated");
			const std::string & server = s.get_string ("servername",     name.c_str());
			const bool registered      = s.get_bool   ("registered",     false);
			const std::string & pwd    = s.get_string ("password",       "");
			for (;;) { // endless loop
				if (!InternetGaming::ref().login(name, pwd, registered, meta, port)) {
					dedicatedlog("ERROR: Could not connect to metaserver (reason above)!\n");
					return;
				}
				std::string realservername(server);
				bool name_valid = false;
				while (!name_valid) {
					name_valid = true;
					const std::vector<InternetGame> & hosts = InternetGaming::ref().games();
					for (uint32_t i = 0; i < hosts.size(); ++i) {
						if (hosts.at(i).name == realservername)
							name_valid = false;
					}
					if (!name_valid)
						realservername += "*";
				}

				InternetGaming::ref().set_local_servername(realservername);

				NetHost netgame(name, true);

				// Load the requested map
				Widelands::Map map;
				i18n::Textdomain td("maps");
				map.set_filename(m_filename);
				std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(m_filename);
				if (!ml) {
					throw WLWarning
						("Unsupported format",
						 "Widelands could not load the file \"%s\". The file format seems to be incompatible.",
						 m_filename.c_str());
				}
				ml->preload_map(true);

				// fill in the mapdata structure
				MapData mapdata;
				mapdata.filename = m_filename;
				mapdata.name = map.get_name();
				mapdata.authors.parse(map.get_author());
				mapdata.description = map.get_description();
				mapdata.nrplayers = map.get_nrplayers();
				mapdata.width = map.get_width();
				mapdata.height = map.get_height();

				// set the map
				netgame.set_map(mapdata.name, mapdata.filename, mapdata.nrplayers);

				// run the network game
				// -> autostarts when a player sends "/start" as pm to the server.
				netgame.run(true);

				InternetGaming::ref().logout();
			}
		} catch (const std::exception & e) {
			log("Fatal exception: %s\n", e.what());
			emergency_save(game);
			throw;
		}
	} else {
		g_sound_handler.start_music("intro");

		{
			FullscreenMenuIntro intro;
			intro.run<FullscreenMenuBase::MenuTarget>();
		}

		g_sound_handler.change_music("menu", 1000);
		mainmenu();
	}

	g_sound_handler.stop_music(500);

	return;
}

/**
 * Get an event from the SDL queue, just like SDL_PollEvent.
 *
 * \param ev the retrieved event will be put here
 *
 * \return true if an event was returned inside ev, false otherwise
 */
bool WLApplication::poll_event(SDL_Event& ev) {
	if (!SDL_PollEvent(&ev)) {
		return false;
	}

	// We edit mouse motion events in here, so that
	// differences caused by GrabInput or mouse speed
	// settings are invisible to the rest of the code
	switch (ev.type) {
	case SDL_MOUSEMOTION:
		ev.motion.xrel += m_mouse_compensate_warp.x;
		ev.motion.yrel += m_mouse_compensate_warp.y;
		m_mouse_compensate_warp = Point(0, 0);

		if (m_mouse_locked) {
			warp_mouse(m_mouse_position);

			ev.motion.x = m_mouse_position.x;
			ev.motion.y = m_mouse_position.y;
		}
		break;

	case SDL_USEREVENT:
		if (ev.user.code == CHANGE_MUSIC)
			g_sound_handler.change_music();
		break;

	default:
		break;
	}
	return true;
}

bool WLApplication::handle_key(bool down, const SDL_Keycode& keycode, int modifiers) {
	if (down) {
		const bool ctrl = (modifiers & KMOD_LCTRL) || (modifiers & KMOD_RCTRL);
		switch (keycode) {
		case SDLK_F10:
			// exits the game.
			if (ctrl) {
				m_should_die = true;
			}
			return true;

		case SDLK_F11:
			// Takes a screenshot.
			if (ctrl) {
				if (g_fs->disk_space() < MINIMUM_DISK_SPACE) {
					log("Omitting screenshot because diskspace is lower than %luMB\n",
						 MINIMUM_DISK_SPACE / (1000 * 1000));
					break;
				}
				g_fs->ensure_directory_exists(SCREENSHOT_DIR);
				for (uint32_t nr = 0; nr < 10000; ++nr) {
					const std::string filename = (boost::format(SCREENSHOT_DIR "/shot%04u.png") % nr).str();
					if (g_fs->file_exists(filename)) {
						continue;
					}
					g_gr->screenshot(filename);
					break;
				}
			}
			return true;

		case SDLK_f: {
			// toggle fullscreen
			bool value = !g_gr->fullscreen();
			g_gr->set_fullscreen(value);
			g_options.pull_section("global").set_bool("fullscreen", value);
			return true;
		}

		default:
			break;
		}
	}
	return false;
}

void WLApplication::handle_input(InputCallback const * cb)
{
	SDL_Event ev;
	while (poll_event(ev)) {
		switch (ev.type) {
		case SDL_KEYUP:
		case SDL_KEYDOWN: {
			bool handled = false;
			if (cb && cb->key) {
				handled = cb->key(ev.type == SDL_KEYDOWN, ev.key.keysym);
			}
			if (!handled) {
				handle_key(ev.type == SDL_KEYDOWN, ev.key.keysym.sym, ev.key.keysym.mod);
			}
		} break;

		case SDL_TEXTINPUT:
			if (cb && cb->textinput) {
				cb->textinput(ev.text.text);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			_handle_mousebutton(ev, cb);
			break;
		case SDL_MOUSEWHEEL:
			if (cb && cb->mouse_wheel) {
				cb->mouse_wheel(ev.wheel.which, ev.wheel.x, ev.wheel.y);
			}
			break;
		case SDL_MOUSEMOTION:
			m_mouse_position = Point(ev.motion.x, ev.motion.y);

			if ((ev.motion.xrel || ev.motion.yrel) && cb && cb->mouse_move)
				cb->mouse_move
					(ev.motion.state,
					 ev.motion.x,    ev.motion.y,
					 ev.motion.xrel, ev.motion.yrel);
			break;
		case SDL_QUIT:
			m_should_die = true;
			break;
		default:;
		}
	}
}

/*
 * Capsule repetitive code for mouse buttons
 */
void WLApplication::_handle_mousebutton
	(SDL_Event & ev, InputCallback const * cb)
{
		if (m_mouse_swapped) {
			switch (ev.button.button) {
				case SDL_BUTTON_LEFT:
					ev.button.button = SDL_BUTTON_RIGHT;
					break;
				case SDL_BUTTON_RIGHT:
					ev.button.button = SDL_BUTTON_LEFT;
					break;
				default:
					break;
			}
		}

#ifdef __APPLE__
		//  On Mac, SDL does middle mouse button emulation (alt+left). This
		//  interferes with the editor, which is using alt+left click for
		//  third tool. So if we ever see a middle mouse button on Mac,
		//  check if any ALT Key is pressed and if, treat it like a left
		//  mouse button.
		if
			(ev.button.button == SDL_BUTTON_MIDDLE &&
			 (get_key_state(SDL_SCANCODE_LALT) || get_key_state(SDL_SCANCODE_RALT)))
		{
			ev.button.button = SDL_BUTTON_LEFT;
			m_faking_middle_mouse_button = true;
		}
#endif

		if (ev.type == SDL_MOUSEBUTTONDOWN && cb && cb->mouse_press)
			cb->mouse_press(ev.button.button, ev.button.x, ev.button.y);
		else if (ev.type == SDL_MOUSEBUTTONUP) {
			if (cb && cb->mouse_release) {
				if (ev.button.button == SDL_BUTTON_MIDDLE && m_faking_middle_mouse_button) {
					cb->mouse_release(SDL_BUTTON_LEFT, ev.button.x, ev.button.y);
					m_faking_middle_mouse_button = false;
				}
				cb->mouse_release(ev.button.button, ev.button.x, ev.button.y);
			}
		}
}

/**
 * Return the current time, in milliseconds
 */
// TODO(unknown): Use our internally defined time type
// TODO(sirver): get rid of this method and use SDL_GetTicks() directly.
int32_t WLApplication::get_time() {
	uint32_t time = SDL_GetTicks();

	return time;
}

/// Instantaneously move the mouse cursor without creating a motion event.
///
/// SDL_WarpMouseInWindow() *will* create a mousemotion event, which we do not want.
/// As a workaround, we store the delta in m_mouse_compensate_warp and use that to
/// eliminate the motion event in poll_event()
///
/// \param position The new mouse position
void WLApplication::warp_mouse(const Point position)
{
	m_mouse_position = position;

	Point cur_position;
	SDL_GetMouseState(&cur_position.x, &cur_position.y);
	if (cur_position != position) {
		m_mouse_compensate_warp += cur_position - position;
		SDL_Window* sdl_window = g_gr->get_sdlwindow();
		if (sdl_window) {
			SDL_WarpMouseInWindow(sdl_window, position.x, position.y);
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
	if (!g_gr) {
		return;
	}
	SDL_Window * sdl_window = g_gr->get_sdlwindow();
	if (grab) {
		if (sdl_window) {
			SDL_SetWindowGrab(sdl_window, SDL_TRUE);
		}
	} else {
		if (sdl_window) {
			SDL_SetWindowGrab(sdl_window, SDL_FALSE);
		}
		warp_mouse(m_mouse_position); //TODO(unknown): is this redundant?
	}
}

void WLApplication::refresh_graphics()
{
	Section & s = g_options.pull_section("global");

	g_gr->change_resolution(
	   s.get_int("xres", DEFAULT_RESOLUTION_W), s.get_int("yres", DEFAULT_RESOLUTION_H));
	g_gr->set_fullscreen(s.get_bool("fullscreen", false));

	// does only work with a window
	set_input_grab(s.get_bool("inputgrab", false));
}

/**
 * Read the config file, parse the commandline and give all other internal
 * parameters sensible default values
 */
bool WLApplication::init_settings() {

	//read in the configuration file
	g_options.read("config", "global");
	Section & s = g_options.pull_section("global");

	//then parse the commandline - overwrites conffile settings
	handle_commandline_parameters();

	set_mouse_swap(s.get_bool("swapmouse", false));

	// TODO(unknown): KLUDGE!
	// Without this the following config options get dropped by check_used().
	// Profile needs support for a Syntax definition to solve this in a
	// sensible way
	s.get_bool("fullscreen");
	s.get_int("xres");
	s.get_int("yres");
	s.get_int("border_snap_distance");
	s.get_int("maxfps");
	s.get_int("panel_snap_distance");
	s.get_int("autosave");
	s.get_int("rolling_autosave");
	s.get_int("remove_replays");
	s.get_bool("single_watchwin");
	s.get_bool("auto_roadbuild_mode");
	s.get_bool("workareapreview");
	s.get_bool("nozip");
	s.get_bool("snap_windows_only_when_overlapping");
	s.get_bool("dock_windows_to_edges");
	s.get_bool("remove_syncstreams");
	s.get_bool("sound_at_message");
	s.get_bool("transparent_chat");
	s.get_bool("dedicated_saving"); // saving via chatcommand on dedicated servers -> nethost.cc
	s.get_string("registered");
	s.get_string("nickname");
	s.get_string("password");
	s.get_string("emailadd");
	s.get_string("auto_log");
	s.get_string("lasthost");
	s.get_string("servername");
	s.get_string("realname");
	s.get_string("metaserver");
	s.get_natural("metaserverport");
	// KLUDGE!

	return true;
}

/**
 * Initialize language settings
 */
void WLApplication::init_language() {
	// retrieve configuration settings
	Section & s = g_options.pull_section("global");

	// Initialize locale and grab "widelands" textdomain
	i18n::init_locale();

	i18n::set_localedir(m_datadir + "/locale");
	i18n::grab_textdomain("widelands");

	// Set locale corresponding to selected language
	std::string language = s.get_string("language", "");
	i18n::set_locale(language);
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
	} catch (const std::exception & e) {
		log("WARNING: could not save configuration: %s\n", e.what());
	} catch (...)                      {
		log("WARNING: could not save configuration");
	}
}

/**
 * Start the hardware: switch to graphics mode, start sound handler
 *
 * \pre The locale must be known before calling this
 *
 * \return true if there were no fatal errors that prevent the game from running
 */
bool WLApplication::init_hardware() {
	Section & s = g_options.pull_section("global");

	//Start the SDL core
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
		throw wexception
			("Failed to initialize SDL, no valid video driver: %s",
			 SDL_GetError());

	SDL_ShowCursor(SDL_DISABLE);

	g_gr = new Graphic(s.get_int("xres", DEFAULT_RESOLUTION_W),
	                   s.get_int("yres", DEFAULT_RESOLUTION_H),
	                   s.get_bool("fullscreen", false));

	g_sound_handler.init(); //  TODO(unknown): memory leak!

	return true;
}

void WLApplication::shutdown_hardware()
{
	delete g_gr;
	g_gr = nullptr;

#ifndef _WIN32
	// SOUND can lock up with buggy SDL/drivers. we try to do the right thing
	// but if it doesn't happen we will kill widelands anyway in 5 seconds.
	signal(SIGALRM, terminate);
	alarm(5);
#endif

	g_sound_handler.shutdown();

	SDL_QuitSubSystem(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
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
void WLApplication::parse_commandline
	(int const argc, char const * const * const argv)
{
	for (int i = 1; i < argc; ++i) {
		std::string opt = argv[i];
		std::string value;

		if (!opt.compare(0, 5, "-psn_")) {
			// Mac OS passes this on the commandline when launched from finder.
			// SDL1 removed it for us (apparently), but SDL2 does no longer, so we
			// have to do this ourselves.
			continue;
		}

		//are we looking at an option at all?
		if (opt.compare(0, 2, "--"))
			throw ParameterError();
		else
			opt.erase(0, 2); //  yes. remove the leading "--", just for cosmetics

		//look if this option has a value
		std::string::size_type const pos = opt.find('=');

		if (pos == std::string::npos) { //  if no equals sign found
			value = "";
		} else {
			//extract option value
			value = opt.substr(pos + 1);

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
void WLApplication::handle_commandline_parameters()
{
	if (m_commandline.count("logfile")) {
		m_logfile = m_commandline["logfile"];
		std::cerr << "Redirecting log target to: " <<  m_logfile << std::endl;
		if (m_logfile.size() != 0) {
			//TODO(unknown): (very small) memory leak of 1 ofstream;
			//swaw the buffers (internally) of the file and wout
			std::ofstream * widelands_out = new std::ofstream(m_logfile.c_str());
			std::streambuf * logbuf = widelands_out->rdbuf();
			wout.rdbuf(logbuf);
		}
		m_commandline.erase("logfile");
	}
	if (m_commandline.count("nosound")) {
		g_sound_handler.nosound_ = true;
		m_commandline.erase("nosound");
	}
	if (m_commandline.count("nozip")) {
		g_options.pull_section("global").create_val("nozip", "true");
		m_commandline.erase("nozip");
	}

	if (m_commandline.count("datadir")) {
		m_datadir = m_commandline["datadir"];
		m_commandline.erase("datadir");
	} else {
		m_datadir = is_absolute_path(INSTALL_DATADIR) ?
		               INSTALL_DATADIR :
		               get_executable_directory() + INSTALL_DATADIR;
	}
	if (!is_absolute_path(m_datadir)) {
		m_datadir = absolute_path_if_not_windows(FileSystem::get_working_directory() +
		                                         FileSystem::file_separator() + m_datadir);
	}

	if (m_commandline.count("verbose")) {
		g_verbose = true;

		m_commandline.erase("verbose");
	}

	if (m_commandline.count("editor")) {
		m_filename = m_commandline["editor"];
		if (m_filename.size() && *m_filename.rbegin() == '/')
			m_filename.erase(m_filename.size() - 1);
		m_game_type = EDITOR;
		m_commandline.erase("editor");
	}

	if (m_commandline.count("replay")) {
		if (m_game_type != NONE)
			throw wexception("replay can not be combined with other actions");
		m_filename = m_commandline["replay"];
		if (m_filename.size() && *m_filename.rbegin() == '/')
			m_filename.erase(m_filename.size() - 1);
		m_game_type = REPLAY;
		m_commandline.erase("replay");
	}

	if (m_commandline.count("loadgame")) {
		if (m_game_type != NONE)
			throw wexception("loadgame can not be combined with other actions");
		m_filename = m_commandline["loadgame"];
		if (m_filename.empty())
			throw wexception("empty value of command line parameter --loadgame");
		if (*m_filename.rbegin() == '/')
			m_filename.erase(m_filename.size() - 1);
		m_game_type = LOADGAME;
		m_commandline.erase("loadgame");
	}

	if (m_commandline.count("scenario")) {
		if (m_game_type != NONE)
			throw wexception("scenario can not be combined with other actions");
		m_filename = m_commandline["scenario"];
		if (m_filename.empty())
			throw wexception("empty value of command line parameter --scenario");
		if (*m_filename.rbegin() == '/')
			m_filename.erase(m_filename.size() - 1);
		m_game_type = SCENARIO;
		m_commandline.erase("scenario");
	}
	if (m_commandline.count("dedicated")) {
		if (m_game_type != NONE)
			throw wexception("dedicated can not be combined with other actions");
		m_filename = m_commandline["dedicated"];
		if (m_filename.empty())
			throw wexception("empty value of commandline parameter --dedicated");
		if (*m_filename.rbegin() == '/')
			m_filename.erase(m_filename.size() - 1);
		m_game_type = INTERNET;
		m_commandline.erase("dedicated");
	}
	if (m_commandline.count("script")) {
		m_script_to_run = m_commandline["script"];
		if (m_script_to_run.empty())
			throw wexception("empty value of command line parameter --script");
		if (*m_script_to_run.rbegin() == '/')
			m_script_to_run.erase(m_script_to_run.size() - 1);
		m_commandline.erase("script");
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
		// TODO(unknown): barf here on unknown option; the list of known options
		// needs to be centralized

		g_options.pull_section("global").create_val
			(it->first.c_str(), it->second.c_str());
	}

	if (m_commandline.count("help") || m_commandline.count("version")) {
		init_language();
		throw ParameterError(); //no message on purpose
	}
}


/**
 * Run the main menu
 */
void WLApplication::mainmenu()
{
	std::string messagetitle;
	std::string message;

	for (;;) {
		// Refresh graphics system in case we just changed resolution.
		refresh_graphics();

		FullscreenMenuMain mm;

		if (message.size()) {
			log("\n%s\n%s\n", messagetitle.c_str(), message.c_str());

			UI::WLMessageBox mmb
				(&mm,
				 messagetitle,
				 message,
				 UI::WLMessageBox::MBoxType::kOk,
				 UI::Align_Left);
			mmb.run<UI::Panel::Returncodes>();

			message.clear();
			messagetitle.clear();
		}

		try {
			switch (mm.run<FullscreenMenuBase::MenuTarget>()) {
			case FullscreenMenuBase::MenuTarget::kTutorial:
				mainmenu_tutorial();
				break;
			case FullscreenMenuBase::MenuTarget::kSinglePlayer:
				mainmenu_singleplayer();
				break;
			case FullscreenMenuBase::MenuTarget::kMultiplayer:
				mainmenu_multiplayer();
				break;
			case FullscreenMenuBase::MenuTarget::kReplay:
				replay();
				break;
			case FullscreenMenuBase::MenuTarget::kOptions: {
				Section & s = g_options.pull_section("global");
				OptionsCtrl om(s);
				break;
			}
			case FullscreenMenuBase::MenuTarget::kReadme: {
				FullscreenMenuFileView ff("txts/README.lua");
				ff.run<FullscreenMenuBase::MenuTarget>();
				break;
			}
			case FullscreenMenuBase::MenuTarget::kLicense: {
				FullscreenMenuFileView ff("txts/LICENSE.lua");
				ff.run<FullscreenMenuBase::MenuTarget>();
				break;
			}
			case FullscreenMenuBase::MenuTarget::kAuthors: {
				FullscreenMenuFileView ff("txts/AUTHORS.lua");
				ff.run<FullscreenMenuBase::MenuTarget>();
				break;
			}
			case FullscreenMenuBase::MenuTarget::kEditor:
				EditorInteractive::run_editor(m_filename, m_script_to_run);
				break;
			default:
			case FullscreenMenuBase::MenuTarget::kExit:
				return;
			}
		} catch (const WLWarning & e) {
			messagetitle = (boost::format("Warning: %s") % e.title()).str();
			message = e.what();
		} catch (const Widelands::GameDataError & e) {
			messagetitle = _("Game data error");
			message = e.what();
		}
#ifdef NDEBUG
		catch (const std::exception & e) {
			messagetitle = "Unexpected error during the game";
			message = e.what();
			message +=

		(boost::format(_
					("\n\nPlease report this problem to help us improve Widelands. "
					 "You will find related messages in the standard output "
					 "(stdout.txt on Windows). You are using build %1$s (%2$s). ")
				) % build_id().c_str() % build_type().c_str()).str();

			message +=
				_
					("Please add this information to your report.\n\n"
					 "Widelands attempts to create a savegame when errors occur "
					 "during the game. It is often – though not always – possible "
					 "to load it and continue playing.\n");
		}
#endif
	}
}


/**
 * Handle the "Play Tutorial" menu option:
 * Show tutorial UI, let player select tutorial and run it.
 */
void WLApplication::mainmenu_tutorial()
{
	Widelands::Game game;
	std::string filename;
		//  Start UI for the tutorials.
		FullscreenMenuCampaignMapSelect select_campaignmap(true);
		select_campaignmap.set_campaign(0);
		if (select_campaignmap.run<FullscreenMenuBase::MenuTarget>() == FullscreenMenuBase::MenuTarget::kOk) {
			filename = select_campaignmap.get_map();
		}
	try {
		// Load selected tutorial-map-file
		if (filename.size())
			game.run_splayer_scenario_direct(filename.c_str(), "");
	} catch (const std::exception & e) {
		log("Fatal exception: %s\n", e.what());
		emergency_save(game);
		throw;
	}
}



/**
 * Run the singleplayer menu
 */
void WLApplication::mainmenu_singleplayer()
{
	//  This is the code returned by UI::Panel::run<Returncode>() when the panel is dying.
	//  Make sure that the program exits when the window manager says so.
	static_assert
		(static_cast<int>(FullscreenMenuBase::MenuTarget::kBack)
		 == static_cast<int>(UI::Panel::Returncodes::kBack),
		 "Panel should be dying.");

	for (;;) {
		FullscreenMenuSinglePlayer single_player_menu;
		switch (single_player_menu.run<FullscreenMenuBase::MenuTarget>()) {
		case FullscreenMenuBase::MenuTarget::kBack:
			return;
		case FullscreenMenuBase::MenuTarget::kNewGame:
			if (new_game())
				return;
			break;
		case FullscreenMenuBase::MenuTarget::kLoadGame:
			if (load_game())
				return;
			break;
		case FullscreenMenuBase::MenuTarget::kCampaign:
			if (campaign_game())
				return;
			break;
		default:
			assert(false);
			break;
		}
	}

}

/**
 * Run the multiplayer menu
 */
void WLApplication::mainmenu_multiplayer()
{
	FullscreenMenuBase::MenuTarget menu_result = FullscreenMenuBase::MenuTarget::kJoingame; // dummy init;
	for (;;) { // stay in menu until player clicks "back" button
		bool internet = false;
		FullscreenMenuMultiPlayer mp;
		switch (mp.run<FullscreenMenuBase::MenuTarget>()) {
			case FullscreenMenuBase::MenuTarget::kBack:
				return;
			case FullscreenMenuBase::MenuTarget::kMetaserver:
				internet = true;
				break;
			case FullscreenMenuBase::MenuTarget::kLan:
				break;
			default:
				assert(false);
				break;
		}

		if (internet) {
			std::string playername = mp.get_nickname();
			std::string password(mp.get_password());
			bool registered = mp.registered();

			Section & s = g_options.pull_section("global");
			s.set_string("nickname", playername);
			// Only change the password if we use a registered account
			if (registered)
				s.set_string("password", password);

			// reinitalise in every run, else graphics look strange
			FullscreenMenuInternetLobby ns(playername.c_str(), password.c_str(), registered);
			ns.run<FullscreenMenuBase::MenuTarget>();

			if (InternetGaming::ref().logged_in())
				// logout of the metaserver
				InternetGaming::ref().logout();
			else
				// Reset InternetGaming for clean login
				InternetGaming::ref().reset();
		} else {
			// reinitalise in every run, else graphics look strange
			FullscreenMenuNetSetupLAN ns;
			menu_result = ns.run<FullscreenMenuBase::MenuTarget>();
			std::string playername = ns.get_playername();
			uint32_t addr;
			uint16_t port;
			bool const host_address = ns.get_host_address(addr, port);

			switch (menu_result) {
				case FullscreenMenuBase::MenuTarget::kHostgame: {
					NetHost netgame(playername);
					netgame.run();
					break;
				}
				case FullscreenMenuBase::MenuTarget::kJoingame: {
					IPaddress peer;

					if (!host_address)
						throw WLWarning
							("Invalid Address", "%s",
							 "The address of the game server is invalid");

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
	FullscreenMenuLaunchSPG lgm(&sp);
	const FullscreenMenuBase::MenuTarget code = lgm.run<FullscreenMenuBase::MenuTarget>();
	Widelands::Game game;

	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		return false;
	}
	if (code == FullscreenMenuBase::MenuTarget::kScenarioGame) { // scenario
		try {
			game.run_splayer_scenario_direct(sp.get_map().c_str(), "");
		} catch (const std::exception & e) {
			log("Fatal exception: %s\n", e.what());
			emergency_save(game);
			throw;
		}
	} else { // normal singleplayer
		uint8_t const pn = sp.settings().playernum + 1;
		try {
			// Game controller needs the ibase pointer to init
			// the chat
			game.set_ibase
				(new InteractivePlayer
					(game, g_options.pull_section("global"), pn, false));
			std::unique_ptr<GameController> ctrl
				(new SinglePlayerGameController(game, true, pn));
			UI::ProgressWindow loaderUI;
			std::vector<std::string> tipstext;
			tipstext.push_back("general_game");
			tipstext.push_back("singleplayer");
			try {
				tipstext.push_back(sp.get_players_tribe());
			} catch (GameSettingsProvider::NoTribe) {
			}
			GameTips tips (loaderUI, tipstext);

			loaderUI.step(_("Preparing game"));

			game.set_game_controller(ctrl.get());
			game.init_newgame(&loaderUI, sp.settings());
			game.run(&loaderUI, Widelands::Game::NewNonScenario, "", false);
		} catch (const std::exception & e) {
			log("Fatal exception: %s\n", e.what());
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

	SinglePlayerGameSettingsProvider sp;
	FullscreenMenuLoadGame ssg(game, &sp, nullptr);

	if (ssg.run<FullscreenMenuBase::MenuTarget>() == FullscreenMenuBase::MenuTarget::kOk)
		filename = ssg.filename();
	else
		return false;

	try {
		if (game.run_load_game(filename, ""))
			return true;
	} catch (const std::exception & e) {
		log("Fatal exception: %s\n", e.what());
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
		{ //  First start UI for selecting the campaign.
			FullscreenMenuCampaignSelect select_campaign;
			if (select_campaign.run<FullscreenMenuBase::MenuTarget>() == FullscreenMenuBase::MenuTarget::kOk)
				campaign = select_campaign.get_campaign();
			else { //  back was pressed
				filename = "";
				break;
			}
		}
		//  Then start UI for the selected campaign.
		FullscreenMenuCampaignMapSelect select_campaignmap;
		select_campaignmap.set_campaign(campaign);
		if (select_campaignmap.run<FullscreenMenuBase::MenuTarget>() == FullscreenMenuBase::MenuTarget::kOk) {
			filename = select_campaignmap.get_map();
			break;
		}
	}
	try {
		// Load selected campaign-map-file
		if (filename.size())
			return game.run_splayer_scenario_direct(filename.c_str(), "");
	} catch (const std::exception & e) {
		log("Fatal exception: %s\n", e.what());
		emergency_save(game);
		throw;
	}
	return false;
}

/**
 * Show the replay menu and play a replay.
 */
void WLApplication::replay()
{
	Widelands::Game game;
	if (m_filename.empty()) {
		SinglePlayerGameSettingsProvider sp;
		FullscreenMenuLoadGame rm(game, &sp, nullptr, true);
		if (rm.run<FullscreenMenuBase::MenuTarget>() == FullscreenMenuBase::MenuTarget::kBack)
			return;

		m_filename = rm.filename();
	}

	try {
		UI::ProgressWindow loaderUI;
		std::vector<std::string> tipstext;
		tipstext.push_back("general_game");
		GameTips tips (loaderUI, tipstext);

		loaderUI.step(_("Loading..."));

		game.set_ibase
			(new InteractiveSpectator(game, g_options.pull_section("global")));
		game.set_write_replay(false);
		ReplayGameController rgc(game, m_filename);

		game.save_handler().set_allow_saving(false);

		game.run(&loaderUI, Widelands::Game::Loaded, "", true);
	} catch (const std::exception & e) {
		log("Fatal Exception: %s\n", e.what());
		emergency_save(game);
		m_filename.clear();
		throw;
	}
	m_filename.clear();
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

/**
 * Delete the syncstream (.wss) files in the replay directory on startup
 * Delete old replay files on startup
 */
void WLApplication::cleanup_replays()
{
	FilenameSet files;

	Section & s = g_options.pull_section("global");

	if (s.get_bool("remove_syncstreams", true)) {
		files =
			filter(g_fs->list_directory(REPLAY_DIR),
		          [](const std::string& fn) {return boost::ends_with(fn, REPLAY_SUFFIX ".wss");});

		for
			(FilenameSet::iterator filename = files.begin();
			 filename != files.end();
			 ++filename)
		{
			log("Delete syncstream %s\n", filename->c_str());
			g_fs->fs_unlink(*filename);
		}
	}

	time_t tnow = time(nullptr);

	if (s.get_int("remove_replays", 0)) {
		files =
			filter(g_fs->list_directory(REPLAY_DIR),
		          [](const std::string& fn) {return boost::ends_with(fn, REPLAY_SUFFIX);});

		for
			(FilenameSet::iterator filename = files.begin();
			 filename != files.end();
			 ++filename)
		{
			std::string file = g_fs->fs_filename(filename->c_str());
			std::string timestr = file.substr(0, file.find(' '));

			if (19 != timestr.size())
				continue;

			tm tfile;
			memset(&tfile, 0, sizeof(tm));

			tfile.tm_mday = atoi(timestr.substr(8, 2).c_str());
			tfile.tm_mon = atoi(timestr.substr(5, 2).c_str()) - 1;
			tfile.tm_year = atoi(timestr.substr(0, 4).c_str()) - 1900;

			double tdiff = std::difftime(tnow, mktime(&tfile)) / 86400;

			if (tdiff > s.get_int("remove_replays")) {
				log("Delete replay %s\n", file.c_str());

				g_fs->fs_unlink(*filename);
				g_fs->fs_unlink(*filename + ".wgf");
			}
		}
	}
}

bool WLApplication::redirect_output(std::string path)
{
	if (path.empty()) {
#ifdef _WIN32
		char module_name[MAX_PATH];
		GetModuleFileName(nullptr, module_name, MAX_PATH);
		path = module_name;
		size_t pos = path.find_last_of("/\\");
		if (pos == std::string::npos) return false;
		path.resize(pos);
#else
		path = ".";
#endif
	}
	std::string stdoutfile = path + "/stdout.txt";
	/* Redirect standard output */
	FILE * newfp = freopen(stdoutfile.c_str(), "w", stdout);
	if (!newfp) return false;
	/* Redirect standard error */
	std::string stderrfile = path + "/stderr.txt";
	newfp = freopen(stderrfile.c_str(), "w", stderr);
	if (!newfp) return false;

	/* Line buffered */
	setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);

	/* No buffering */
	setbuf(stderr, nullptr);

	m_redirected_stdio = true;
	return true;
}
