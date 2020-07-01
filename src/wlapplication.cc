/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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

#include <cassert>
#ifndef _WIN32
#include <csignal>
#endif
#include <iostream>
#include <memory>

#include <SDL.h>
#include <SDL_ttf.h>
#include <boost/regex.hpp>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/random.h"
#include "base/time_string.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "build_info.h"
#include "config.h"
#include "editor/editorinteractive.h"
#include "graphic/default_resolution.h"
#include "graphic/font_handler.h"
#include "graphic/mouse_cursor.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/game_settings.h"
#include "logic/map.h"
#include "logic/replay.h"
#include "logic/replay_game_controller.h"
#include "logic/single_player_game_controller.h"
#include "logic/single_player_game_settings_provider.h"
#include "map_io/map_loader.h"
#include "network/crypto.h"
#include "network/gameclient.h"
#include "network/gamehost.h"
#include "network/internet_gaming.h"
#include "sound/sound_handler.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/about.h"
#include "ui_fsmenu/campaign_select.h"
#include "ui_fsmenu/campaigns.h"
#include "ui_fsmenu/internet_lobby.h"
#include "ui_fsmenu/intro.h"
#include "ui_fsmenu/launch_spg.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/main.h"
#include "ui_fsmenu/mapselect.h"
#include "ui_fsmenu/multiplayer.h"
#include "ui_fsmenu/netsetup_lan.h"
#include "ui_fsmenu/options.h"
#include "ui_fsmenu/scenario_select.h"
#include "ui_fsmenu/singleplayer.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"

namespace {

/**
 * Shut the hardware down: stop graphics mode, stop sound handler
 */
#ifndef _WIN32
void terminate(int) {
	// The logger can already be shut down, so we use cout
	std::cout
	   << "Waited 5 seconds to close audio. There are some problems here, so killing Widelands."
	      " Update your sound driver and/or SDL to fix this problem\n";
	raise(SIGKILL);
}
#endif

/**
 * Returns the widelands executable path.
 */
std::string get_executable_directory() {
	std::string executabledir;
#ifdef __APPLE__
	uint32_t buffersize = 0;
	_NSGetExecutablePath(nullptr, &buffersize);
	std::unique_ptr<char[]> buffer(new char[buffersize]);
	int32_t check = _NSGetExecutablePath(buffer.get(), &buffersize);
	if (check != 0) {
		throw wexception("could not find the path of the main executable");
	}
	executabledir = std::string(buffer.get());
	executabledir.resize(executabledir.rfind('/') + 1);
#endif
#ifdef __linux__
	char buffer[PATH_MAX];
	size_t size = readlink("/proc/self/exe", buffer, PATH_MAX);
	if (size <= 0) {
		throw wexception("could not find the path of the main executable");
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
std::string absolute_path_if_not_windows(const std::string& path) {
#ifndef _WIN32
	char buffer[PATH_MAX];
	// https://pubs.opengroup.org/onlinepubs/009695399/functions/realpath.html
	char* rp = realpath(path.c_str(), buffer);
	log("Realpath: %s\n", rp);
	if (!rp) {
		throw wexception("Unable to get absolute path for %s", path.c_str());
	}
	return std::string(rp);
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

// Extracts a long from 'text' into 'val' returning true if all of the string
// was valid. If not, the content of 'val' is undefined.
bool to_long(const std::string& text, long* val) {
	const char* start = text.c_str();
	char* end;
	*val = strtol(start, &end, 10);
	return *end == '\0';
}

// Extracts the creation date from 'path' which is expected to
// match "YYYY-MM-DD*". Returns false if no date could be extracted.
bool extract_creation_day(const std::string& path, tm* tfile) {
	const std::string filename = FileSystem::fs_filename(path.c_str());
	memset(tfile, 0, sizeof(tm));

	long day, month, year;
	if (!to_long(filename.substr(8, 2), &day)) {
		return false;
	}
	if (!to_long(filename.substr(5, 2), &month)) {
		return false;
	}
	if (!to_long(filename.substr(0, 4), &year)) {
		return false;
	}

	tfile->tm_mday = day;
	tfile->tm_mon = month - 1;
	tfile->tm_year = year - 1900;
	return tfile;
}

// Returns true if 'filename' was autogenerated, i.e. if 'extract_creation_day'
// can return a date and it is old enough to be deleted.
bool is_autogenerated_and_expired(const std::string& filename,
                                  const double keep_time = kReplayKeepAroundTime) {
	tm tfile;
	if (!extract_creation_day(filename, &tfile)) {
		return false;
	}
	return std::difftime(time(nullptr), mktime(&tfile)) > keep_time;
}

}  // namespace

// Set up the homedir. Exit 1 if the homedir is illegal or the logger couldn't be initialized for
// Windows.
// Also sets the config directory. This defaults to $XDG_CONFIG_HOME/widelands/config on Unix.
// Defaults to homedir/config everywhere else, if homedir is set manually or if
// built without XDG-support.
void WLApplication::setup_homedir() {
	// Check if we have a command line override
	if (commandline_.count("homedir")) {
		// We don't have a destination dir for the logger yet
		std::cout << "Adding home directory: " << commandline_["homedir"].c_str() << std::endl;
		homedir_ = commandline_["homedir"];
#ifdef USE_XDG
		// This commandline option should probably also affect the
		// configuration file.
		userconfigdir_ = commandline_["homedir"];
#endif
		commandline_.erase("homedir");
	}

#ifdef REDIRECT_OUTPUT
	if (!redirect_output())
		redirect_output(homedir_);
#endif

	// If we don't have a home directory, we exit with an error
	if (homedir_.empty()) {
		std::cout << "Unable to start Widelands, because the given homedir is empty" << std::endl;
		delete g_fs;
		exit(1);
	} else {
		try {
			std::unique_ptr<FileSystem> home(new RealFSImpl(homedir_));
			home->ensure_directory_exists(".");
			g_fs->set_home_file_system(home.release());
		} catch (const std::exception& e) {
			std::cout
			   << "Unable to start Widelands, because we were unable to add the home directory: "
			   << e.what() << std::endl;
			delete g_fs;
			exit(1);
		}
#ifdef _WIN32
		// Initialize the logger for Windows. Exit on failure.
		if (!set_logging_dir(homedir_)) {
			delete g_fs;
			exit(1);
		}
#endif
		// Homedir is ready, so we can log normally from now on
		log("Set home directory: %s\n", homedir_.c_str());
	}

#ifdef USE_XDG
	set_config_directory(userconfigdir_);
#else
	set_config_directory(homedir_);
#endif
}

WLApplication* WLApplication::the_singleton = nullptr;

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
WLApplication* WLApplication::get(int const argc, char const** argv) {
	if (the_singleton == nullptr)
		the_singleton = new WLApplication(argc, argv);
	return the_singleton;
}

/**
 * Initialize an instance of WLApplication.
 *
 * Exits with code 2 if the SDL/TTF system is not available.
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
WLApplication::WLApplication(int const argc, char const* const* const argv)
   : commandline_(std::map<std::string, std::string>()),
     game_type_(NONE),
     mouse_swapped_(false),
     faking_middle_mouse_button_(false),
     mouse_position_(Vector2i::zero()),
     mouse_locked_(0),
     mouse_compensate_warp_(Vector2i::zero()),
     should_die_(false),
#ifdef _WIN32
     homedir_(FileSystem::get_homedir() + "\\.widelands"),
#elif defined USE_XDG
     // To enable backwards compatibility, the program name is passed with the
     // path.
     homedir_(FileSystem::get_userdatadir()),
     userconfigdir_(FileSystem::get_userconfigdir()),
#else
     homedir_(FileSystem::get_homedir() + "/.widelands"),
#endif
     redirected_stdio_(false),
     last_resolution_change_(0) {
	g_fs = new LayeredFileSystem();

	parse_commandline(argc, argv);  // throws ParameterError, handled by main.cc

	setup_homedir();
	init_settings();
	datadir_ = g_fs->canonicalize_name(datadir_);
	datadir_for_testing_ = g_fs->canonicalize_name(datadir_for_testing_);

	log("Adding directory: %s\n", datadir_.c_str());
	g_fs->add_file_system(&FileSystem::create(datadir_));

	if (!datadir_for_testing_.empty()) {
		log("Adding directory: %s\n", datadir_for_testing_.c_str());
		g_fs->add_file_system(&FileSystem::create(datadir_for_testing_));
	}

	init_language();  // search paths must already be set up
	changedir_on_mac();
	cleanup_replays();
	cleanup_ai_files();
	cleanup_temp_files();
	cleanup_temp_backups();

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	log("Byte order: little-endian\n");
#else
	log("Byte order: big-endian\n");
#endif

	// Start the SDL core
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		// We sometimes run into a missing video driver in our CI environment, so we exit 0 to prevent
		// too frequent failures
		log("Failed to initialize SDL, no valid video driver: %s", SDL_GetError());
		exit(2);
	}

	g_gr = new Graphic();

	if (TTF_Init() == -1) {
		log("True Type library did not initialize: %s\n", TTF_GetError());
		exit(2);
	}

	UI::g_fh = UI::create_fonthandler(
	   &g_gr->images(), i18n::get_locale());  // This will create the fontset, so loading it first.

	g_gr->initialize(
	   get_config_bool("debug_gl_trace", false) ? Graphic::TraceGl::kYes : Graphic::TraceGl::kNo,
	   get_config_int("xres", DEFAULT_RESOLUTION_W), get_config_int("yres", DEFAULT_RESOLUTION_H),
	   get_config_bool("fullscreen", false));

	g_mouse_cursor = new MouseCursor();
	g_mouse_cursor->initialize(get_config_bool("sdl_cursor", true));

	g_sh = new SoundHandler();

	g_sh->register_songs("music", "intro");
	g_sh->register_songs("music", "menu");
	g_sh->register_songs("music", "ingame");

	// Register the click sound for UI::Panel.
	// We do it here to ensure that the sound handler has been created first, and we only want to
	// register it once.
	UI::Panel::register_click();

	// This might grab the input.
	refresh_graphics();

	// seed random number generator used for random tribe selection
	std::srand(time(nullptr));

	// Make sure we didn't forget to read any global option
	check_config_used();
}

/**
 * Shut down all subsystems in an orderly manner
 */
// TODO(unknown): Handle errors that happen here!
WLApplication::~WLApplication() {
	// Do use the opposite order of WLApplication::init()

	shutdown_hardware();
	shutdown_settings();

	assert(UI::g_fh);
	delete UI::g_fh;
	UI::g_fh = nullptr;

	TTF_Quit();  // TODO(unknown): not here

	if (g_fs) {
		delete g_fs;
	}
	g_fs = nullptr;

	if (redirected_stdio_) {
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
void WLApplication::run() {
	// This also grabs the mouse cursor if so desired.
	refresh_graphics();

	if (game_type_ == EDITOR) {
		g_sh->change_music("ingame");
		EditorInteractive::run_editor(filename_, script_to_run_);
	} else if (game_type_ == REPLAY) {
		replay();
	} else if (game_type_ == LOADGAME) {
		Widelands::Game game;
		game.set_ai_training_mode(get_config_bool("ai_training", false));
		try {
			game.run_load_game(filename_, script_to_run_);
		} catch (const Widelands::GameDataError& e) {
			log("Game not loaded: Game data error: %s\n", e.what());
		} catch (const std::exception& e) {
			log("Fatal exception: %s\n", e.what());
			emergency_save(game);
			throw;
		}
	} else if (game_type_ == SCENARIO) {
		Widelands::Game game;
		try {
			game.run_splayer_scenario_direct(filename_.c_str(), script_to_run_);
		} catch (const Widelands::GameDataError& e) {
			log("Scenario not started: Game data error: %s\n", e.what());
		} catch (const std::exception& e) {
			log("Fatal exception: %s\n", e.what());
			emergency_save(game);
			throw;
		}
	} else {
		g_sh->change_music("intro");

		{
			FullscreenMenuIntro intro;
			intro.run<FullscreenMenuBase::MenuTarget>();
		}

		g_sh->change_music("menu", 1000);
		mainmenu();
	}

	g_sh->stop_music(500);

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
		ev.motion.xrel += mouse_compensate_warp_.x;
		ev.motion.yrel += mouse_compensate_warp_.y;
		mouse_compensate_warp_ = Vector2i::zero();

		if (mouse_locked_) {
			warp_mouse(mouse_position_);

			ev.motion.x = mouse_position_.x;
			ev.motion.y = mouse_position_.y;
		}
		break;

	case SDL_USEREVENT: {
		if (ev.user.code == CHANGE_MUSIC) {
			/* Notofication from the SoundHandler that a song has finished playing.
			 * Usually, another song from the same songset will be started.
			 * There is a special case for the intro screen's music: only one song will be
			 * played. If the user has not clicked the mouse or pressed escape when the song
			 * finishes, Widelands will automatically go on to the main menu.
			 */
			assert(!SoundHandler::is_backend_disabled());
			if (g_sh->current_songset() == "intro") {
				// Special case for splashscreen: there, only one song is ever played
				SDL_Event new_event;
				new_event.type = SDL_KEYDOWN;
				new_event.key.state = SDL_PRESSED;
				new_event.key.keysym.sym = SDLK_ESCAPE;
				SDL_PushEvent(&new_event);
			} else {
				g_sh->change_music();
			}
		}
	} break;

	default:
		break;
	}
	return true;
}

bool WLApplication::handle_key(bool down, const SDL_Keycode& keycode, int modifiers) {
	if (down) {
		const bool ctrl = (modifiers & KMOD_LCTRL) || (modifiers & KMOD_RCTRL);
		switch (keycode) {
		case SDLK_F11:
			// Takes a screenshot.
			if (ctrl) {
				if (g_fs->disk_space() < kMinimumDiskSpace) {
					log("Omitting screenshot because diskspace is lower than %lluMB\n",
					    kMinimumDiskSpace / (1000 * 1000));
					break;
				}
				g_fs->ensure_directory_exists(kScreenshotsDir);
				for (uint32_t nr = 0; nr < 10000; ++nr) {
					const std::string filename =
					   (boost::format("%s/shot%04u.png") % kScreenshotsDir % nr).str();
					if (g_fs->file_exists(filename)) {
						continue;
					}
					g_gr->screenshot(filename);
					break;
				}
			}
			return true;

		case SDLK_f: {
			// Toggle fullscreen
			const uint32_t time = SDL_GetTicks();
			if ((time - last_resolution_change_ > 250) && (ctrl)) {
				last_resolution_change_ = time;
				bool value = !g_gr->fullscreen();
				g_gr->set_fullscreen(value);
				set_config_bool("fullscreen", value);
			}
			return true;
		}

		default:
			break;
		}
	}
	return false;
}

void WLApplication::handle_input(InputCallback const* cb) {
	// Container for keyboard events using the Alt key.
	// <sym, mod>, type.
	std::map<std::pair<int32_t, uint16_t>, uint32_t> alt_events;

	SDL_Event ev;
	while (poll_event(ev)) {
		switch (ev.type) {
		case SDL_KEYUP:
		case SDL_KEYDOWN: {
			bool handled = false;
			// Workaround for duplicate triggering of the Alt key in Ubuntu:
			// Don't accept the same key twice, so we use a map to squash them and handle them later.
			if (ev.key.keysym.mod & KMOD_ALT) {
				alt_events.insert(std::make_pair<std::pair<int32_t, uint16_t>, uint32_t>(
				   std::make_pair<int32_t, uint16_t>(static_cast<int32_t>(ev.key.keysym.sym),
				                                     static_cast<uint16_t>(ev.key.keysym.mod)),
				   static_cast<uint32_t>(ev.type)));
				handled = true;
			}
			if (!handled && cb && cb->key) {
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
			handle_mousebutton(ev, cb);
			break;
		case SDL_MOUSEWHEEL:
			if (cb && cb->mouse_wheel) {
				cb->mouse_wheel(ev.wheel.which, ev.wheel.x, ev.wheel.y);
			}
			break;
		case SDL_MOUSEMOTION:
			mouse_position_ = Vector2i(ev.motion.x, ev.motion.y);

			if ((ev.motion.xrel || ev.motion.yrel) && cb && cb->mouse_move)
				cb->mouse_move(
				   ev.motion.state, ev.motion.x, ev.motion.y, ev.motion.xrel, ev.motion.yrel);
			break;
		case SDL_QUIT:
			should_die_ = true;
			break;
		default:;
		}
	}

	// Now constructing the events for the Alt key from the container and handling them.
	for (const auto& event : alt_events) {
		ev.type = event.second;
		ev.key.keysym.sym = event.first.first;
		ev.key.keysym.mod = event.first.second;
		bool handled = false;
		if (cb && cb->key) {
			handled = cb->key(ev.type == SDL_KEYDOWN, ev.key.keysym);
		}
		if (!handled) {
			handle_key(ev.type == SDL_KEYDOWN, ev.key.keysym.sym, ev.key.keysym.mod);
		}
	}
}

/*
 * Capsule repetitive code for mouse buttons
 */
void WLApplication::handle_mousebutton(SDL_Event& ev, InputCallback const* cb) {
	if (mouse_swapped_) {
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
	if (ev.button.button == SDL_BUTTON_MIDDLE &&
	    (get_key_state(SDL_SCANCODE_LALT) || get_key_state(SDL_SCANCODE_RALT))) {
		ev.button.button = SDL_BUTTON_LEFT;
		faking_middle_mouse_button_ = true;
	}
#endif

	if (ev.type == SDL_MOUSEBUTTONDOWN && cb && cb->mouse_press)
		cb->mouse_press(ev.button.button, ev.button.x, ev.button.y);
	else if (ev.type == SDL_MOUSEBUTTONUP) {
		if (cb && cb->mouse_release) {
			if (ev.button.button == SDL_BUTTON_MIDDLE && faking_middle_mouse_button_) {
				cb->mouse_release(SDL_BUTTON_LEFT, ev.button.x, ev.button.y);
				faking_middle_mouse_button_ = false;
			}
			cb->mouse_release(ev.button.button, ev.button.x, ev.button.y);
		}
	}
}

/// Instantaneously move the mouse cursor without creating a motion event.
///
/// SDL_WarpMouseInWindow() *will* create a mousemotion event, which we do not want.
/// As a workaround, we store the delta in mouse_compensate_warp_ and use that to
/// eliminate the motion event in poll_event()
///
/// \param position The new mouse position
void WLApplication::warp_mouse(const Vector2i position) {
	mouse_position_ = position;

	Vector2i cur_position = Vector2i::zero();
	SDL_GetMouseState(&cur_position.x, &cur_position.y);
	if (cur_position != position) {
		mouse_compensate_warp_ += cur_position - position;
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
void WLApplication::set_input_grab(bool grab) {
	if (!g_gr) {
		return;
	}
	SDL_Window* sdl_window = g_gr->get_sdlwindow();
	if (grab) {
		if (sdl_window) {
			SDL_SetWindowGrab(sdl_window, SDL_TRUE);
		}
	} else {
		if (sdl_window) {
			SDL_SetWindowGrab(sdl_window, SDL_FALSE);
		}
		warp_mouse(mouse_position_);  // TODO(unknown): is this redundant?
	}
}

void WLApplication::set_mouse_lock(const bool locked) {
	mouse_locked_ = locked;

	// If we use the SDL cursor then it needs to be hidden when locked
	// otherwise it'll jerk around which looks ugly
	if (g_mouse_cursor->is_using_sdl()) {
		g_mouse_cursor->set_visible(!mouse_locked_);
	}
}

void WLApplication::refresh_graphics() {
	g_gr->change_resolution(
	   get_config_int("xres", DEFAULT_RESOLUTION_W), get_config_int("yres", DEFAULT_RESOLUTION_H));
	g_gr->set_fullscreen(get_config_bool("fullscreen", false));

	// does only work with a window
	set_input_grab(get_config_bool("inputgrab", false));
}

/**
 * Read the config file, parse the commandline and give all other internal
 * parameters sensible default values
 */
bool WLApplication::init_settings() {

	// Read in the configuration file
	read_config();

	// Then parse the commandline - overwrites conffile settings
	handle_commandline_parameters();

	set_mouse_swap(get_config_bool("swapmouse", false));

	// TODO(unknown): KLUDGE!
	// Without this the following config options get dropped by check_used().
	// Profile needs support for a Syntax definition to solve this in a
	// sensible way

	// Some of the options listed here are documented in wlapplication_messages.cc
	get_config_bool("ai_training", false);
	get_config_bool("auto_roadbuild_mode", false);
	get_config_bool("auto_speed", false);
	get_config_bool("dock_windows_to_edges", false);
	get_config_bool("fullscreen", false);
	get_config_bool("sdl_cursor", true);
	get_config_bool("snap_windows_only_when_overlapping", false);
	get_config_bool("animate_map_panning", false);
	get_config_bool("write_syncstreams", false);
	get_config_bool("nozip", false);
	get_config_int("xres", 0);
	get_config_int("yres", 0);
	get_config_int("border_snap_distance", 0);
	get_config_int("maxfps", 0);
	get_config_int("panel_snap_distance", 0);
	get_config_int("autosave", 0);
	get_config_int("rolling_autosave", 0);
	get_config_string("language", "");
	get_config_string("metaserver", "");
	get_config_natural("metaserverport", 0);
	// Undocumented on command line, appears in game options
	get_config_bool("single_watchwin", false);
	get_config_bool("ctrl_zoom", false);
	get_config_bool("game_clock", true);
	get_config_bool("inputgrab", false);
	get_config_bool("transparent_chat", false);
	// Undocumented. Unique ID used to allow the metaserver to recognize players
	get_config_string("uuid", "");
	// Undocumented, appears in online login box
	// Whether the used metaserver login is for a registered user
	get_config_string("registered", "");
	// Undocumented, appears in online login box and LAN lobby
	// The nickname used for LAN and online games
	get_config_string("nickname", "");
	// Undocumented, appears in online login box. The hashed password for online logins
	get_config_string("password_sha1", "");
	// Undocumented, appears in online login box. Whether to automatically use the stored login
	get_config_string("auto_log", "");
	// Undocumented, appears in LAN lobby. The last host connected to
	get_config_string("lasthost", "");
	// Undocumented, appears in online lobby. The name of the last hosted game
	get_config_string("servername", "");
	// Undocumented, appears in editor. Name of map author
	get_config_string("realname", "");
	// Undocumented, checkbox appears on "Watch Replay" screen
	get_config_bool("display_replay_filenames", false);
	get_config_bool("editor_player_menu_warn_too_many_players", false);
	// Undocumented, on command line, appears in game options
	get_config_bool("sound", "enable_ambient", true);
	get_config_bool("sound", "enable_chat", true);
	get_config_bool("sound", "enable_message", true);
	get_config_bool("sound", "enable_music", true);
	get_config_bool("sound", "enable_ui", true);
	get_config_int("sound", "volume_ambient", 128);
	get_config_int("sound", "volume_chat", 128);
	get_config_int("sound", "volume_message", 128);
	get_config_int("sound", "volume_music", 64);
	get_config_int("sound", "volume_ui", 128);
	// KLUDGE!

	long int last_start = get_config_int("last_start", 0);
	if (last_start + 12 * 60 * 60 < time(nullptr) || !get_config_string("uuid", "").empty()) {
		// First start of the game or not started for 12 hours. Create a (new) UUID.
		// For the use of the UUID, see network/internet_gaming_protocol.h
		get_config_string("uuid", generate_random_uuid().c_str());
	}
	get_config_int("last_start", time(nullptr));

	// Save configuration now. Otherwise, the UUID is not saved
	// when the game crashes, losing part of its advantage
	write_config();

	return true;
}

/**
 * Initialize language settings
 */
void WLApplication::init_language() {
	// Set the locale dir
	i18n::set_localedir(g_fs->canonicalize_name(datadir_ + "/locale"));

	// If locale dir is not a directory, barf. We can handle it not being there tough.
	if (g_fs->file_exists(i18n::get_localedir()) && !g_fs->is_directory(i18n::get_localedir())) {
		SDL_ShowSimpleMessageBox(
		   SDL_MESSAGEBOX_ERROR, "'locale' directory not valid",
		   std::string(i18n::get_localedir() + "\nis not a directory. Please fix this.").c_str(),
		   NULL);
		log("ERROR: %s is not a directory. Please fix this.\n", i18n::get_localedir().c_str());
		exit(1);
	}

	if (!g_fs->is_directory(i18n::get_localedir()) ||
	    g_fs->list_directory(i18n::get_localedir()).empty()) {
		log("WARNING: No locale translations found in %s\n", i18n::get_localedir().c_str());
	}

	// Initialize locale and grab "widelands" textdomain
	i18n::init_locale();
	i18n::grab_textdomain("widelands");

	// Set locale corresponding to selected language
	std::string language = get_config_string("language", "");
	i18n::set_locale(language);
}

/**
 * Remember the last settings: write them into the config file
 */
void WLApplication::shutdown_settings() {
	// To be proper, release our textdomain
	i18n::release_textdomain();
	write_config();
}

void WLApplication::shutdown_hardware() {
	delete g_mouse_cursor;
	g_mouse_cursor = nullptr;

	delete g_gr;
	g_gr = nullptr;

// SOUND can lock up with buggy SDL/drivers. we try to do the right thing
// but if it doesn't happen we will kill widelands anyway in 5 seconds.
#ifndef _WIN32
	signal(SIGALRM, terminate);
	// TODO(GunChleoc): alarm is a POSIX function. If we found a Windows equivalent, we could call
	// terminate in Windows as well.
	alarm(5);
#endif

	delete g_sh;
	g_sh = nullptr;

	SDL_QuitSubSystem(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
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
void WLApplication::parse_commandline(int const argc, char const* const* const argv) {
	for (int i = 1; i < argc; ++i) {
		std::string opt = argv[i];
		std::string value;

		if (!opt.compare(0, 5, "-psn_")) {
			// Mac OS passes this on the commandline when launched from finder.
			// SDL1 removed it for us (apparently), but SDL2 does no longer, so we
			// have to do this ourselves.
			continue;
		}

		// Are we looking at an option at all?
		if (opt.compare(0, 2, "--"))
			throw ParameterError();
		else
			opt.erase(0, 2);  //  yes. remove the leading "--", just for cosmetics

		// Look if this option has a value
		std::string::size_type const pos = opt.find('=');

		if (pos == std::string::npos) {  //  if no equals sign found
			value = "";
		} else {
			// Extract option value
			value = opt.substr(pos + 1);

			// Remove value from option name
			opt.erase(pos, opt.size() - pos);
		}

		commandline_[opt] = value;
	}
}

/**
 * Parse the command line given in commandline_
 *
 * \return false if there were errors during parsing \e or if "--help"
 * was given,
 * true otherwise.
 */
void WLApplication::handle_commandline_parameters() {
	if (commandline_.count("nosound")) {
		SoundHandler::disable_backend();
		commandline_.erase("nosound");
	}
	if (commandline_.count("nozip")) {
		set_config_bool("nozip", true);
		commandline_.erase("nozip");
	}

	if (commandline_.count("datadir")) {
		datadir_ = commandline_["datadir"];
		commandline_.erase("datadir");
	} else {
		datadir_ = is_absolute_path(INSTALL_DATADIR) ?
		              INSTALL_DATADIR :
		              get_executable_directory() + FileSystem::file_separator() + INSTALL_DATADIR;
#ifdef USE_XDG
		// Overwrite with first folder found in XDG_DATA_DIRS
		for (const auto& datadir : FileSystem::get_xdgdatadirs()) {
			RealFSImpl dir(datadir);
			if (dir.is_directory(datadir + "/widelands")) {
				datadir_ = datadir + "/widelands";
				break;
			}
		}
#endif
	}
	if (!is_absolute_path(datadir_)) {
		try {
			datadir_ = absolute_path_if_not_windows(FileSystem::get_working_directory() +
			                                        FileSystem::file_separator() + datadir_);
		} catch (const WException& e) {
			log("Error parsing datadir: %s\n", e.what());
			exit(1);
		}
	}
	if (commandline_.count("datadir_for_testing")) {
		datadir_for_testing_ = commandline_["datadir_for_testing"];
		commandline_.erase("datadir_for_testing");
	}

	if (commandline_.count("verbose")) {
		g_verbose = true;

		commandline_.erase("verbose");
	}

	if (commandline_.count("editor")) {
		filename_ = commandline_["editor"];
		if (filename_.size() && *filename_.rbegin() == '/')
			filename_.erase(filename_.size() - 1);
		game_type_ = EDITOR;
		commandline_.erase("editor");
	}

	if (commandline_.count("replay")) {
		if (game_type_ != NONE)
			throw wexception("replay can not be combined with other actions");
		filename_ = commandline_["replay"];
		if (filename_.size() && *filename_.rbegin() == '/')
			filename_.erase(filename_.size() - 1);
		game_type_ = REPLAY;
		commandline_.erase("replay");
	}

	if (commandline_.count("loadgame")) {
		if (game_type_ != NONE)
			throw wexception("loadgame can not be combined with other actions");
		filename_ = commandline_["loadgame"];
		if (filename_.empty())
			throw wexception("empty value of command line parameter --loadgame");
		if (*filename_.rbegin() == '/')
			filename_.erase(filename_.size() - 1);
		game_type_ = LOADGAME;
		commandline_.erase("loadgame");
	}

	if (commandline_.count("scenario")) {
		if (game_type_ != NONE)
			throw wexception("scenario can not be combined with other actions");
		filename_ = commandline_["scenario"];
		if (filename_.empty())
			throw wexception("empty value of command line parameter --scenario");
		if (*filename_.rbegin() == '/')
			filename_.erase(filename_.size() - 1);
		game_type_ = SCENARIO;
		commandline_.erase("scenario");
	}
	if (commandline_.count("script")) {
		script_to_run_ = commandline_["script"];
		if (script_to_run_.empty())
			throw wexception("empty value of command line parameter --script");
		if (*script_to_run_.rbegin() == '/')
			script_to_run_.erase(script_to_run_.size() - 1);
		commandline_.erase("script");
	}

	// Following is used for training of AI
	if (commandline_.count("ai_training")) {
		set_config_bool("ai_training", true);
		commandline_.erase("ai_training");
	} else {
		set_config_bool("ai_training", false);
	}

	if (commandline_.count("auto_speed")) {
		set_config_bool("auto_speed", true);
		commandline_.erase("auto_speed");
	} else {
		set_config_bool("auto_speed", false);
	}

	// If it hasn't been handled yet it's probably an attempt to
	// override a conffile setting
	// With typos, this will create invalid config settings. They
	// will be taken care of (==ignored) when saving the options

	const std::map<std::string, std::string>::const_iterator commandline_end = commandline_.end();
	for (std::map<std::string, std::string>::const_iterator it = commandline_.begin();
	     it != commandline_end; ++it) {
		// TODO(unknown): barf here on unknown option; the list of known options
		// needs to be centralized

		set_config_string(it->first.c_str(), it->second.c_str());
	}

	if (commandline_.count("help") || commandline_.count("version")) {
		init_language();
		throw ParameterError();  // No message on purpose
	}
}

/**
 * Run the main menu
 */
void WLApplication::mainmenu() {
	std::string messagetitle;
	std::string message;

	for (;;) {
		// Refresh graphics system in case we just changed resolution.
		refresh_graphics();

		FullscreenMenuMain mm;

		if (message.size()) {
			log("\n%s\n%s\n", messagetitle.c_str(), message.c_str());

			UI::WLMessageBox mmb(&mm, messagetitle, richtext_escape(message),
			                     UI::WLMessageBox::MBoxType::kOk, UI::Align::kLeft);
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
				Section& s = get_config_section();
				OptionsCtrl om(s);
				break;
			}
			case FullscreenMenuBase::MenuTarget::kAbout: {
				FullscreenMenuAbout ff;
				ff.run<FullscreenMenuBase::MenuTarget>();
				break;
			}
			case FullscreenMenuBase::MenuTarget::kEditor:
				EditorInteractive::run_editor(filename_, script_to_run_);
				break;
			case FullscreenMenuBase::MenuTarget::kExit:
			default:
				return;
			}
		} catch (const WLWarning& e) {
			messagetitle = (boost::format("Warning: %s") % e.title()).str();
			message = e.what();
		} catch (const Widelands::GameDataError& e) {
			messagetitle = _("Game data error");
			message = e.what();
		}
#ifdef NDEBUG
		catch (const std::exception& e) {
			messagetitle = "Unexpected error during the game";
			message = e.what();
			message += "\n\n";
			message += (boost::format(_("Please report this problem to help us improve Widelands. "
			                            "You will find related messages in the standard output "
			                            "(stdout.txt on Windows). You are using build %1$s (%2$s).")) %
			            build_id().c_str() % build_type().c_str())
			              .str();

			message = (boost::format("%s\n\n%s") % message %
			           _("Please add this information to your report.\n\n"
			             "Widelands attempts to create a savegame when errors occur "
			             "during the game. It is often – though not always – possible "
			             "to load it and continue playing."))
			             .str();
		}
#endif
	}
}

/**
 * Handle the "Play Tutorial" menu option:
 * Show tutorial UI, let player select tutorial and run it.
 */
void WLApplication::mainmenu_tutorial() {
	Widelands::Game game;
	std::string filename;
	//  Start UI for the tutorials.
	FullscreenMenuScenarioSelect select_campaignmap;
	if (select_campaignmap.run<FullscreenMenuBase::MenuTarget>() ==
	    FullscreenMenuBase::MenuTarget::kOk) {
		filename = select_campaignmap.get_map();
	}
	try {
		// Load selected tutorial-map-file
		if (filename.size())
			game.run_splayer_scenario_direct(filename.c_str(), "");
	} catch (const std::exception& e) {
		log("Fatal exception: %s\n", e.what());
		emergency_save(game);
		throw;
	}
}

/**
 * Run the singleplayer menu
 */
void WLApplication::mainmenu_singleplayer() {
	//  This is the code returned by UI::Panel::run<Returncode>() when the panel is dying.
	//  Make sure that the program exits when the window manager says so.
	static_assert(static_cast<int>(FullscreenMenuBase::MenuTarget::kBack) ==
	                 static_cast<int>(UI::Panel::Returncodes::kBack),
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
			NEVER_HERE();
		}
	}
}

/**
 * Run the multiplayer menu
 */
void WLApplication::mainmenu_multiplayer() {
	FullscreenMenuBase::MenuTarget menu_result =
	   FullscreenMenuBase::MenuTarget::kJoingame;  // dummy init;
	for (;;) {                                     // stay in menu until player clicks "back" button
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
			NEVER_HERE();
		}

		g_sh->change_music("ingame", 1000);

		if (internet) {
			std::string playername = mp.get_nickname();
			std::string password(mp.get_password());
			bool registered = mp.registered();

			get_config_string("nickname", playername);
			// Only change the password if we use a registered account
			if (registered) {
				get_config_string("password_sha1", password);
			}

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

			switch (menu_result) {
			case FullscreenMenuBase::MenuTarget::kHostgame: {
				GameHost netgame(playername);
				netgame.run();
				break;
			}
			case FullscreenMenuBase::MenuTarget::kJoingame: {
				NetAddress addr;
				if (!ns.get_host_address(&addr)) {
					UI::WLMessageBox mmb(
					   &ns, _("Invalid Address"),
					   _("The entered hostname or address is invalid and can’t be connected to."),
					   UI::WLMessageBox::MBoxType::kOk);
					mmb.run<UI::Panel::Returncodes>();
					break;
				}

				GameClient netgame(std::make_pair(addr, NetAddress()), playername);
				netgame.run();
				break;
			}
			default:
				break;
			}
		}
		g_sh->change_music("menu", 1000);
	}
}

/**
 * Handle the "New game" menu option: Configure a single player game and
 * run it.
 *
 * \return @c true if a game was played, @c false if the player pressed Back
 * or aborted the game setup via some other means.
 */
bool WLApplication::new_game() {
	SinglePlayerGameSettingsProvider sp;
	FullscreenMenuLaunchSPG lgm(&sp);
	const FullscreenMenuBase::MenuTarget code = lgm.run<FullscreenMenuBase::MenuTarget>();

	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		return false;
	}

	Widelands::Game game;

	game.set_ai_training_mode(get_config_bool("ai_training", false));

	if (code == FullscreenMenuBase::MenuTarget::kScenarioGame) {  // scenario
		try {
			game.run_splayer_scenario_direct(sp.get_map().c_str(), "");
		} catch (const std::exception& e) {
			log("Fatal exception: %s\n", e.what());
			emergency_save(game);
			throw;
		}
	} else {  // normal singleplayer
		uint8_t const pn = sp.settings().playernum + 1;
		try {
			// Game controller needs the ibase pointer to init
			// the chat
			game.set_ibase(new InteractivePlayer(game, get_config_section(), pn, false));
			std::unique_ptr<GameController> ctrl(new SinglePlayerGameController(game, true, pn));

			std::vector<std::string> tipstexts{"general_game", "singleplayer"};
			if (sp.has_players_tribe()) {
				tipstexts.push_back(sp.get_players_tribe());
			}
			game.create_loader_ui(tipstexts, false);

			game.step_loader_ui(_("Preparing game"));

			game.set_game_controller(ctrl.get());
			game.init_newgame(sp.settings());
			game.run(Widelands::Game::NewNonScenario, "", false, "single_player");
		} catch (const std::exception& e) {
			log("Fatal exception: %s\n", e.what());
			std::unique_ptr<GameController> ctrl(new SinglePlayerGameController(game, true, pn));
			game.set_game_controller(ctrl.get());
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
bool WLApplication::load_game() {
	Widelands::Game game;
	std::string filename;

	game.set_ai_training_mode(get_config_bool("ai_training", false));
	SinglePlayerGameSettingsProvider sp;
	FullscreenMenuLoadGame ssg(game, &sp);

	if (ssg.run<FullscreenMenuBase::MenuTarget>() == FullscreenMenuBase::MenuTarget::kOk)
		filename = ssg.filename();
	else
		return false;

	try {
		if (game.run_load_game(filename, ""))
			return true;
	} catch (const std::exception& e) {
		log("Fatal exception: %s\n", e.what());
		emergency_save(game);
		throw;
	}
	return false;  // keep compiler silent.
}

/**
 * Handle the "Campaign" menu option:
 * Show campaign UI, let player select scenario and run it.
 *
 * \return @c true if a scenario was played, @c false if the player pressed Back
 * or aborted the game setup via some other means.
 */
bool WLApplication::campaign_game() {
	Widelands::Game game;
	std::string filename;
	for (;;) {  // Campaign UI - Loop
		std::unique_ptr<Campaigns> campaign_visibility(new Campaigns());

		size_t campaign_index;
		{  //  First start UI for selecting the campaign.
			FullscreenMenuCampaignSelect select_campaign(campaign_visibility.get());
			if (select_campaign.run<FullscreenMenuBase::MenuTarget>() ==
			    FullscreenMenuBase::MenuTarget::kOk) {
				campaign_index = select_campaign.get_campaign_index();
			} else {  //  back was pressed
				filename = "";
				break;
			}
		}
		//  Then start UI for the selected campaign.
		CampaignData* campaign_data = campaign_visibility->get_campaign(campaign_index);
		FullscreenMenuScenarioSelect select_campaignmap(campaign_data);
		if (select_campaignmap.run<FullscreenMenuBase::MenuTarget>() ==
		    FullscreenMenuBase::MenuTarget::kOk) {
			filename = select_campaignmap.get_map();
			game.set_scenario_difficulty(select_campaignmap.get_difficulty());
			break;
		}
	}
	try {
		// Load selected campaign-map-file
		if (filename.size())
			return game.run_splayer_scenario_direct(filename.c_str(), "");
	} catch (const std::exception& e) {
		log("Fatal exception: %s\n", e.what());
		emergency_save(game);
		throw;
	}
	return false;
}

/**
 * Show the replay menu and play a replay.
 */
void WLApplication::replay() {
	Widelands::Game game;
	if (filename_.empty()) {
		SinglePlayerGameSettingsProvider sp;
		FullscreenMenuLoadGame rm(game, &sp, true);
		if (rm.run<FullscreenMenuBase::MenuTarget>() == FullscreenMenuBase::MenuTarget::kBack)
			return;

		filename_ = rm.filename();
	}

	try {
		game.create_loader_ui({"general_game"}, true);
		game.step_loader_ui(_("Loading…"));

		game.set_ibase(new InteractiveSpectator(game, get_config_section()));
		game.set_write_replay(false);
		ReplayGameController rgc(game, filename_);

		game.save_handler().set_allow_saving(false);

		game.run(Widelands::Game::Loaded, "", true, "replay");
	} catch (const std::exception& e) {
		log("Fatal Exception: %s\n", e.what());
		emergency_save(game);
		filename_.clear();
		throw;
	}
	filename_.clear();
}

/**
 * Try to save the game instance if possible
 */
void WLApplication::emergency_save(Widelands::Game& game) {
	log("FATAL ERROR - game crashed. Attempting emergency save.\n");
	if (game.is_loaded()) {
		try {
			SaveHandler& save_handler = game.save_handler();
			std::string error;
			if (!save_handler.save_game(
			       game, save_handler.create_file_name(kSaveDir, timestring()), &error)) {
				log("Emergency save failed: %s\n", error.c_str());
			}
		} catch (...) {
			log("Emergency save failed");
			throw;
		}
	}
}

/**
 * Delete old syncstream (.wss) files in the replay directory on startup
 * Delete old replay files on startup
 */
void WLApplication::cleanup_replays() {
	for (const std::string& filename : g_fs->filter_directory(kReplayDir, [](const std::string& fn) {
		     return boost::ends_with(
		        fn, (boost::format("%s%s") % kReplayExtension % kSyncstreamExtension).str());
		  })) {
		if (is_autogenerated_and_expired(filename, kReplayKeepAroundTime)) {
			log("Delete syncstream or replay %s\n", filename.c_str());
			try {
				g_fs->fs_unlink(filename);
			} catch (const FileError& e) {
				log("WLApplication::cleanup_replays: File %s couldn't be deleted: %s\n",
				    filename.c_str(), e.what());
			}
		}
	}
}

/**
 * Delete old ai dna files generated during AI initialization
 */
void WLApplication::cleanup_ai_files() {
	for (const std::string& filename : g_fs->filter_directory(kAiDir, [](const std::string& fn) {
		     return boost::ends_with(fn, kAiExtension) || boost::contains(fn, "ai_player");
		  })) {
		if (is_autogenerated_and_expired(filename, kAIFilesKeepAroundTime)) {
			log("Deleting generated ai file: %s\n", filename.c_str());
			try {
				g_fs->fs_unlink(filename);
			} catch (const FileError& e) {
				log("WLApplication::cleanup_ai_files: File %s couldn't be deleted: %s\n",
				    filename.c_str(), e.what());
			}
		}
	}
}

/**
 * Delete old temp files that might still lurk around (game crashes etc.)
 */
void WLApplication::cleanup_temp_files() {
	for (const std::string& filename : g_fs->filter_directory(
	        kTempFileDir,
	        [](const std::string& fn) { return boost::ends_with(fn, kTempFileExtension); })) {
		if (is_autogenerated_and_expired(filename, kTempFilesKeepAroundTime)) {
			log("Deleting old temp file: %s\n", filename.c_str());
			try {
				g_fs->fs_unlink(filename);
			} catch (const FileError& e) {
				log("WLApplication::cleanup_temp_files: File %s couldn't be deleted: %s\n",
				    filename.c_str(), e.what());
			}
		}
	}
}

/**
 * Recursively delete temporary backup files in a given directory
 */
void WLApplication::cleanup_temp_backups(std::string dir) {
	for (const std::string& filename : g_fs->filter_directory(
	        dir, [](const std::string& fn) { return boost::ends_with(fn, kTempBackupExtension); })) {
		if (is_autogenerated_and_expired(filename, kTempBackupsKeepAroundTime)) {
			log("Deleting old temp backup file: %s\n", filename.c_str());
			try {
				g_fs->fs_unlink(filename);
			} catch (const FileError& e) {
				log("WLApplication::cleanup_temp_backups: File %s couldn't be deleted: %s\n",
				    filename.c_str(), e.what());
			}
		}
	}
	// recursively delete in subdirs
	for (const std::string& dirname : g_fs->filter_directory(dir, [](const std::string& fn) {
		     return g_fs->is_directory(fn) &&
		            // avoid searching within savegames/maps/backups that were created
		            // as directories instead of zipfiles
		            !boost::ends_with(fn, kSavegameExtension) &&
		            !boost::ends_with(fn, kWidelandsMapExtension) &&
		            !boost::ends_with(fn, kTempBackupExtension);
		  })) {
		cleanup_temp_backups(dirname);
	}
}

/**
 * Delete old temporary backup files that might still lurk around (game crashes etc.)
 */
void WLApplication::cleanup_temp_backups() {
	cleanup_temp_backups(kSaveDir);
	cleanup_temp_backups(kMapsDir);
}

bool WLApplication::redirect_output(std::string path) {
	if (path.empty()) {
#ifdef _WIN32
		char module_name[MAX_PATH];
		GetModuleFileName(nullptr, module_name, MAX_PATH);
		path = module_name;
		size_t pos = path.find_last_of("/\\");
		if (pos == std::string::npos)
			return false;
		path.resize(pos);
#else
		path = ".";
#endif
	}
	std::string stdoutfile = path + "/stdout.txt";
	/* Redirect standard output */
	FILE* newfp = freopen(stdoutfile.c_str(), "w", stdout);
	if (!newfp)
		return false;
	/* Redirect standard error */
	std::string stderrfile = path + "/stderr.txt";
	newfp = freopen(stderrfile.c_str(), "w", stderr);
	if (!newfp)
		return false;

	/* Line buffered */
	setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);

	/* No buffering */
	setbuf(stderr, nullptr);

	redirected_stdio_ = true;
	return true;
}
