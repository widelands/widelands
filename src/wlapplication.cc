/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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
#include <cstdlib>
#include <iostream>
#include <memory>
#include <regex>

#include <SDL.h>
#include <SDL_ttf.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/multithreading.h"
#include "base/random.h"
#include "base/time_string.h"
#include "base/wexception.h"
#include "build_info.h"
#include "config.h"
#include "editor/editorinteractive.h"
#include "editor/ui_menus/main_menu_random_map.h"
#include "graphic/default_resolution.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/mouse_cursor.h"
#include "graphic/text/font_set.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/addons.h"
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
#include "network/gameclient.h"
#include "network/gamehost.h"
#include "network/host_game_settings_provider.h"
#include "network/internet_gaming.h"
#include "sound/sound_handler.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/about.h"
#include "ui_fsmenu/launch_spg.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/main.h"
#include "ui_fsmenu/mapselect.h"
#include "ui_fsmenu/options.h"
#include "wlapplication_messages.h"
#include "wlapplication_mousewheel_options.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"

std::string get_executable_directory(const bool logdir) {
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
	if (logdir) {
		log_info("Widelands executable directory: %s\n", executabledir.c_str());
	}
	return executabledir;
}

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

bool is_absolute_path(const std::string& path) {
	std::regex re("^/|\\w:");
	return std::regex_search(path.c_str(), re);
}

// Returns the absolute path of 'path' which might be relative.
std::string absolute_path_if_not_windows(const std::string& path) {
#ifndef _WIN32
	char buffer[PATH_MAX];
	// https://pubs.opengroup.org/onlinepubs/009695399/functions/realpath.html
	char* rp = realpath(path.c_str(), buffer);
	log_info("Realpath: %s\n", rp);
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
bool to_long(const std::string& text, int64_t* val) {
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

	int64_t day, month, year;
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

// A simple wrapper around the game logic thread. It ensures the thread is
// always joined when this class goes out of scope. This is important when an
// exception is thrown within the game anywhere, because without this, we will
// not join the thread and this means the exception will never be caught and
// printed, making debugging hard.
// See
// https://stackoverflow.com/questions/25397874/deleting-stdthread-pointer-raises-exception-libcabi-dylib-terminating
// TODO(klaus-halfman/Noordfrees): move this to base/multithreading.h as generic concept.
class GameLogicThread {
public:
	explicit GameLogicThread(bool* should_die)
	   : should_die_(should_die), thread_(&UI::Panel::logic_thread) {
	}

	DISALLOW_COPY_AND_ASSIGN(GameLogicThread);

	~GameLogicThread() {
		*should_die_ = true;
		thread_.join();
	}

private:
	bool* should_die_;  // signals using object that thread is dead.
	std::thread thread_;
};

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
		log_info("Set home directory: %s\n", homedir_.c_str());

		// Create directory structure
		g_fs->ensure_directory_exists("save");
		g_fs->ensure_directory_exists("replays");
		g_fs->ensure_directory_exists("maps/My_Maps");
		g_fs->ensure_directory_exists("maps/Downloaded");
	}

#ifdef USE_XDG
	set_config_directory(userconfigdir_);
#else
	set_config_directory(homedir_);
#endif

	i18n::set_homedir(homedir_);
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
	if (the_singleton == nullptr) {
		the_singleton = new WLApplication(argc, argv);
	}
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
     game_type_(GameType::kNone),
     mouse_swapped_(false),
     faking_middle_mouse_button_(false),
     mouse_position_(Vector2i::zero()),
     mouse_locked_(false),
     mouse_compensate_warp_(Vector2i::zero()),
     handle_key_enabled_(true),
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

	set_initializer_thread();

	log_info("Adding directory: %s\n", datadir_.c_str());
	g_fs->add_file_system(&FileSystem::create(datadir_));

	if (!datadir_for_testing_.empty()) {
		log_info("Adding directory: %s\n", datadir_for_testing_.c_str());
		g_fs->add_file_system(&FileSystem::create(datadir_for_testing_));
	}

	init_language();  // search paths must already be set up
	changedir_on_mac();
	cleanup_replays();
	cleanup_ai_files();
	cleanup_temp_files();
	cleanup_temp_backups();

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	log_info("Byte order: little-endian\n");
#else
	log_info("Byte order: big-endian\n");
#endif

	// Start the SDL core
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		// We sometimes run into a missing video driver in our CI environment, so we exit 0 to prevent
		// too frequent failures
		log_err("Failed to initialize SDL, no valid video driver: %s", SDL_GetError());
		exit(2);
	}

	g_gr = new Graphic();

	if (TTF_Init() == -1) {
		log_err("True Type library did not initialize: %s\n", TTF_GetError());
		exit(2);
	}

	UI::g_fh = UI::create_fonthandler(
	   g_image_cache, i18n::get_locale());  // This will create the fontset, so loading it first.

	g_gr->initialize(
	   get_config_bool("debug_gl_trace", false) ? Graphic::TraceGl::kYes : Graphic::TraceGl::kNo,
	   get_config_int("xres", kDefaultResolutionW), get_config_int("yres", kDefaultResolutionH),
	   get_config_bool("fullscreen", false), get_config_bool("maximized", false));

	g_mouse_cursor = new MouseCursor();
	g_mouse_cursor->initialize(get_config_bool("sdl_cursor", true));

	g_sh = new SoundHandler();

	g_sh->register_songs("music", "intro");
	g_sh->register_songs("music", "menu");
	g_sh->register_songs("music", "ingame");

	initialize_g_addons();

	// Register the click sound for UI::Panel.
	// We do it here to ensure that the sound handler has been created first, and we only want to
	// register it once.
	UI::Panel::register_click();

	set_input_grab(get_config_bool("inputgrab", false));

	// seed random number generator used for random tribe selection
	std::srand(time(nullptr));

	// Make sure we didn't forget to read any global option
	check_config_used();

	// Save configuration now. Otherwise, the UUID and sound options
	// are not saved, when the game crashes
	write_config();
}

/**
 * Shut down all subsystems in an orderly manner
 */
// TODO(unknown): Handle errors that happen here!
WLApplication::~WLApplication() {
	// Do use the opposite order of WLApplication::init()

	if (!g_gr->fullscreen() && !g_gr->maximized()) {
		set_config_int("xres", g_gr->get_xres());
		set_config_int("yres", g_gr->get_yres());
	}
	set_config_bool("maximized", g_gr->maximized());

	shutdown_hardware();
	shutdown_settings();

	assert(UI::g_fh);
	delete UI::g_fh;
	UI::g_fh = nullptr;

	TTF_Quit();  // TODO(unknown): not here

	delete g_fs;
	g_fs = nullptr;

	if (redirected_stdio_) {
		std::cout.flush();
		fclose(stdout);
		std::cerr.flush();
		fclose(stderr);
	}

	SDL_Quit();
}

void WLApplication::initialize_g_addons() {
	AddOns::g_addons.clear();
	if (g_fs->is_directory(kAddOnDir)) {
		std::set<std::string> found;
		for (std::string desired_addons = get_config_string("addons", ""); !desired_addons.empty();) {
			const size_t commapos = desired_addons.find(',');
			const std::string substring = desired_addons.substr(0, commapos);
			const size_t colonpos = desired_addons.find(':');
			if (colonpos == std::string::npos) {
				log_warn("Ignoring malformed add-ons config substring '%s'\n", substring.c_str());
			} else {
				const std::string name = substring.substr(0, colonpos);
				if (name.find(kAddOnExtension) != name.length() - kAddOnExtension.length()) {
					log_warn("Not loading add-on '%s' (wrong file name extension)\n", name.c_str());
				} else {
					std::string path(kAddOnDir);
					path += FileSystem::file_separator();
					path += name;
					if (g_fs->file_exists(path)) {
						try {
							found.insert(name);
							AddOns::g_addons.push_back(std::make_pair(
							   AddOns::preload_addon(name), substring.substr(colonpos) == ":true"));
						} catch (const std::exception& e) {
							log_warn("Not loading add-on '%s' (%s)", name.c_str(), e.what());
						}
					} else {
						log_warn("Not loading add-on '%s' (not found)\n", name.c_str());
					}
				}
			}
			if (commapos == std::string::npos) {
				break;
			}
			desired_addons = desired_addons.substr(commapos + 1);
		}
		for (const std::string& name : g_fs->list_directory(kAddOnDir)) {
			std::string addon_name(FileSystem::fs_filename(name.c_str()));
			if (!found.count(addon_name) &&
			    addon_name.find(kAddOnExtension) == addon_name.length() - kAddOnExtension.length()) {
				try {
					AddOns::g_addons.push_back(std::make_pair(AddOns::preload_addon(addon_name), false));
				} catch (const std::exception& e) {
					log_warn("Not loading add-on '%s' (%s)", addon_name.c_str(), e.what());
				}
			}
		}
	}
	AddOns::update_ui_theme(AddOns::UpdateThemeAction::kLoadFromConfig);
}

static void init_one_player_from_template(unsigned p,
                                          bool human,
                                          std::unique_ptr<GameSettingsProvider>& settings,
                                          Section& player_section,
                                          const Widelands::Map& map) {
	if (player_section.get_bool("closed", false)) {
		if (human) {
			throw wexception("Cannot close interactive player slot.");
		}
		settings->set_player_state(p, PlayerSettings::State::kClosed);
		return;  // No need to configure closed player
	} else if (human) {
		settings->set_player_state(p, PlayerSettings::State::kHuman);
	} else {
		std::string ai = player_section.get_string("ai", "normal");
		bool random = ai == "random";
		settings->set_player_ai(p, ai, random);
		settings->set_player_state(p, PlayerSettings::State::kComputer);
	}

	settings->set_player_team(p, player_section.get_natural("team", 0));
	if (player_section.has_val("playercolor")) {
		std::string colorstr = player_section.get_safe_string("playercolor");
		char* color;
		RGBColor result;
		result.r = std::strtol(colorstr.c_str(), &color, 10);
		++color;
		result.g = std::strtol(color, &color, 10);
		++color;
		result.b = std::strtol(color, &color, 10);
		settings->set_player_color(p, result);
	} else {
		settings->set_player_color(p, kPlayerColors[p]);
	}

	std::string tribe =
	   player_section.get_string("tribe", map.get_scenario_player_tribe(p + 1).c_str());
	settings->set_player_tribe(p, tribe, tribe.empty());
	tribe = settings->settings().players[p].tribe;

	const std::string& init_script_name = player_section.get_string("init", "headquarters.lua");
	std::string addon;
	if (FileSystem::filename_ext(init_script_name) == kAddOnExtension) {
		addon = kAddOnDir;
		addon += FileSystem::file_separator();
		addon += init_script_name;
		addon += FileSystem::file_separator();
		addon += tribe;
		addon += ".lua";
	}
	bool found_init = false;
	const Widelands::TribeBasicInfo t = settings->settings().get_tribeinfo(tribe);
	for (unsigned i = 0; i < t.initializations.size(); ++i) {
		if (addon.empty() ?
             init_script_name == FileSystem::fs_filename(t.initializations[i].script.c_str()) :
             addon == t.initializations[i].script) {
			settings->set_player_init(p, i);
			found_init = true;
			break;
		}
	}
	if (!found_init) {
		throw wexception(
		   "Invalid starting condition '%s' for player %d", init_script_name.c_str(), p + 1);
	}
}

void WLApplication::init_and_run_game_from_template() {
	AddOns::AddOnsGuard ag;

	Profile profile(filename_.c_str());
	Section& section = profile.get_safe_section("global");
	const bool multiplayer = section.get_bool("multiplayer", false);

	std::vector<AddOns::AddOnState> new_g_addons;
	for (std::string addons = section.get_string("addons", ""); !addons.empty();) {
		const size_t commapos = addons.find(',');
		std::string name;
		if (commapos == std::string::npos) {
			name = addons;
			addons = "";
		} else {
			name = addons.substr(0, commapos);
			addons = addons.substr(commapos + 1);
		}
		bool found = false;
		for (const auto& pair : AddOns::g_addons) {
			if (pair.first->internal_name == name) {
				found = true;
				new_g_addons.push_back(std::make_pair(pair.first, true));
				break;
			}
		}
		if (!found) {
			log_err("Add-on '%s' not found", name.c_str());
			return;
		}
	}
	AddOns::g_addons = new_g_addons;

	const int playernumber = section.get_natural("interactive_player", 1);
	if (playernumber == 0 && !multiplayer) {
		log_err("interactive_player must be > 0 for singleplayer games.");
		return;
	}

	std::unique_ptr<GameSettingsProvider> settings;
	std::unique_ptr<GameHost> host;
	if (multiplayer) {
		std::shared_ptr<GameController> ctrl(nullptr);
		host.reset(new GameHost(nullptr, ctrl, get_config_string("nickname", _("nobody")),
		                        Widelands::get_all_tribeinfos(nullptr), false));
		settings.reset(new HostGameSettingsProvider(host.get()));
	} else {
		settings.reset(new SinglePlayerGameSettingsProvider());
	}

	settings->set_peaceful_mode(section.get_bool("peaceful", false));
	settings->set_custom_starting_positions(section.get_bool("custom_starting_positions", false));

	{
		std::string wc_name = section.get_string("win_condition", "endless_game.lua");
		std::string script;
		if (FileSystem::filename_ext(wc_name) == kAddOnExtension) {
			script = kAddOnDir;
			script += FileSystem::file_separator();
			script += wc_name;
			script += FileSystem::file_separator();
			script += "init.lua";
		} else {
			script = "scripting/win_conditions/";
			script += wc_name;
		}
		settings->set_win_condition_script(script);
	}

	{
		const std::string mapfile = section.get_safe_string("map");
		Widelands::Map map;
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(mapfile);
		if (!ml) {
			log_err("Invalid map file '%s'", mapfile.c_str());
			return;
		}
		ml->preload_map(true, nullptr);
		const int nr_players = map.get_nrplayers();
		settings->set_scenario((map.scenario_types() & Widelands::Map::SP_SCENARIO) != 0);
		settings->set_map(map.get_name(), mapfile, map.get_background_theme(), map.get_background(),
		                  nr_players, false);
		settings->set_player_number(playernumber == 0 ? UserSettings::none() : playernumber - 1);
		for (int p = 0; p < nr_players; ++p) {
			std::string key = "player_";
			key += std::to_string(p + 1);
			bool human = p == playernumber - 1;
			try {
				init_one_player_from_template(
				   p, human, settings, profile.pull_section(key.c_str()), map);
			} catch (const WException& e) {
				log_err("%s", e.what());
				return;
			}
		}
	}

	if (!settings->can_launch()) {
		log_err("Inconsistent game setup configuration. Cannot launch.");
		return;
	}

	if (multiplayer) {
		host->run_direct();
		return;
	}

	Widelands::Game game;
	std::vector<std::string> tipstexts{"general_game", "singleplayer"};
	if (settings->has_players_tribe()) {
		tipstexts.push_back(settings->get_players_tribe());
	}
	game.create_loader_ui(
	   tipstexts, true, settings->settings().map_theme, settings->settings().map_background);
	Notifications::publish(UI::NoteLoadingMessage(_("Preparing game…")));

	game.set_ibase(new InteractivePlayer(game, get_config_section(), playernumber, false));

	game.set_game_controller(std::make_shared<SinglePlayerGameController>(game, true, playernumber));
	game.init_newgame(settings->settings());
	try {
		game.run(Widelands::Game::StartGameType::kMap, "", false, "single_player");
	} catch (const Widelands::GameDataError& e) {
		log_err("Game not started: Game data error: %s\n", e.what());
	} catch (const std::exception& e) {
		log_err("Fatal exception: %s\n", e.what());
		emergency_save(nullptr, game, e.what());
	}
}

/**
 * The main loop. Plain and Simple.
 */
// TODO(unknown): Refactor the whole mainloop out of class \ref UI::Panel into here.
// In the future: push the first event on the event queue, then keep
// dispatching events until it is time to quit.
void WLApplication::run() {
	GameLogicThread game_logic_thread(&should_die_);

	if (game_type_ == GameType::kEditor) {
		g_sh->change_music("ingame");
		if (filename_.empty()) {
			EditorInteractive::run_editor(nullptr, EditorInteractive::Init::kDefault);
		} else {
			EditorInteractive::run_editor(
			   nullptr, EditorInteractive::Init::kLoadMapDirectly, filename_, script_to_run_);
		}
	} else if (game_type_ == GameType::kReplay || game_type_ == GameType::kLoadGame) {
		Widelands::Game game;
		std::string title, message;
		try {
			if (game_type_ == GameType::kReplay) {
				std::string map_theme, map_bg;
				game.create_loader_ui({"general_game"}, true, map_theme, map_bg);
				game.set_ibase(new InteractiveSpectator(game, get_config_section()));
				game.set_write_replay(false);
				new ReplayGameController(game, filename_);
				game.save_handler().set_allow_saving(false);
				game.run(Widelands::Game::StartGameType::kSaveGame, "", true, "replay");
			} else {
				game.set_ai_training_mode(get_config_bool("ai_training", false));
				game.run_load_game(filename_, script_to_run_);
			}
		} catch (const Widelands::GameDataError& e) {
			message = (boost::format(_("Widelands could not load the file \"%s\". The file format "
			                           "seems to be incompatible.")) %
			           filename_.c_str())
			             .str();
			message = message + "\n\n" + _("Error message:") + "\n" + e.what();
			title = _("Game data error");
		} catch (const FileNotFoundError& e) {
			message =
			   (boost::format(_("Widelands could not find the file \"%s\".")) % filename_.c_str())
			      .str();
			message = message + "\n\n" + _("Error message:") + "\n" + e.what();
			title = _("File system error");
		} catch (const std::exception& e) {
			emergency_save(nullptr, game, e.what());
			message = e.what();
			title = _("Error message:");
		}
		if (!message.empty()) {
			g_sh->change_music("menu");
			FsMenu::MainMenu m(true);
			m.show_messagebox(title, message);
			log_err("%s\n", message.c_str());
			m.run<int>();
		}
	} else if (game_type_ == GameType::kScenario) {
		Widelands::Game game;
		try {
			game.run_splayer_scenario_direct({filename_}, script_to_run_);
		} catch (const Widelands::GameDataError& e) {
			log_err("Scenario not started: Game data error: %s\n", e.what());
		} catch (const std::exception& e) {
			log_err("Fatal exception: %s\n", e.what());
			emergency_save(nullptr, game, e.what());
		}
	} else if (game_type_ == GameType::kFromTemplate) {
		init_and_run_game_from_template();
	} else {
		g_sh->change_music("intro");

		g_sh->change_music("menu", 1000);

		FsMenu::MainMenu m;
		m.run<int>();
	}

	g_sh->stop_music(500);

	should_die_ = true;
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

bool WLApplication::handle_key(bool down, const SDL_Keycode& keycode, const int modifiers) {
	if (!down || !handle_key_enabled_) {
		return false;
	}

	if (matches_shortcut(KeyboardShortcut::kCommonScreenshot, keycode, modifiers)) {
		if (g_fs->disk_space() < kMinimumDiskSpace) {
			log_warn("Omitting screenshot because diskspace is lower than %lluMiB\n",
			         kMinimumDiskSpace / (1024 * 1024));
		} else {
			g_fs->ensure_directory_exists(kScreenshotsDir);
			for (uint32_t nr = 0; nr < 10000; ++nr) {
				const std::string filename =
				   (boost::format("%s/shot%04u.png") % kScreenshotsDir % nr).str();
				if (g_fs->file_exists(filename)) {
					continue;
				}
				g_gr->screenshot(filename);
				return true;
			}
			log_warn("Omitting screenshot because 10000 screenshots are already present");
		}
		// Screenshot not taken
		return false;
	}

	if (matches_shortcut(KeyboardShortcut::kCommonFullscreen, keycode, modifiers)) {
		const uint32_t time = SDL_GetTicks();
		if ((time - last_resolution_change_ > 250)) {
			last_resolution_change_ = time;
			const bool value = !g_gr->fullscreen();
			g_gr->set_fullscreen(value);
			set_config_bool("fullscreen", value);
		}
		return true;
	}

	return false;
}

void WLApplication::handle_input(InputCallback const* cb) {
	// Container for keyboard events using the Alt key.
	// <sym, mod>, type.
	std::map<std::pair<SDL_Keycode, uint16_t>, unsigned> alt_events;

	SDL_Event ev;
	while (poll_event(ev)) {
		switch (ev.type) {
		case SDL_KEYUP:
		case SDL_KEYDOWN: {
			normalize_numpad(ev.key.keysym);
			bool handled = false;
			// Workaround for duplicate triggering of the Alt key in Ubuntu:
			// Don't accept the same key twice, so we use a map to squash them and handle them later.
			if (ev.key.keysym.mod & KMOD_ALT) {
				alt_events.insert(
				   std::make_pair(std::make_pair(ev.key.keysym.sym, ev.key.keysym.mod), ev.type));
				handled = true;
			}
			if (!handled) {
				handled = handle_key(ev.type == SDL_KEYDOWN, ev.key.keysym.sym, ev.key.keysym.mod);
			}
			if (!handled && cb && cb->key) {
				cb->key(ev.type == SDL_KEYDOWN, ev.key.keysym);
			}
			break;
		}
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
				cb->mouse_wheel(ev.wheel.x, ev.wheel.y, SDL_GetModState());
			}
			break;
		case SDL_MOUSEMOTION:
			mouse_position_ = Vector2i(ev.motion.x, ev.motion.y);

			if ((ev.motion.xrel || ev.motion.yrel) && cb && cb->mouse_move) {
				cb->mouse_move(
				   ev.motion.state, ev.motion.x, ev.motion.y, ev.motion.xrel, ev.motion.yrel);
			}
			break;
		case SDL_WINDOWEVENT:
			switch (ev.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
				// Do not save the new size to config at this point to avoid saving sizes that
				// result from maximization etc. Save at shutdown instead.
				if (!g_gr->fullscreen()) {
					g_gr->change_resolution(ev.window.data1, ev.window.data2, false);
				}
				break;
			case SDL_WINDOWEVENT_MAXIMIZED:
				set_config_bool("maximized", true);
				break;
			case SDL_WINDOWEVENT_RESTORED:
				set_config_bool("maximized", g_gr->maximized());
				break;
			}
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

	if (ev.type == SDL_MOUSEBUTTONDOWN && cb && cb->mouse_press) {
		cb->mouse_press(ev.button.button, ev.button.x, ev.button.y);
	} else if (ev.type == SDL_MOUSEBUTTONUP) {
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

	// Without this the config options get dropped by check_used().
	for (const std::string& conf : get_all_parameters()) {
		get_config_string(conf, "");
	}

	// Keyboard shortcuts
	init_shortcuts();

	// Mousewheel options
	update_mousewheel_settings();

	int64_t last_start = get_config_int("last_start", 0);
	int64_t now = time(nullptr);
	if (last_start + 12 * 60 * 60 < now || get_config_string("uuid", "").empty()) {
		// First start of the game or not started for 12 hours. Create a (new) UUID.
		// For the use of the UUID, see network/internet_gaming_protocol.h
		set_config_string("uuid", generate_random_uuid());
	}
	set_config_int("last_start", now);

	return true;
}

/**
 * Initialize language settings
 */
void WLApplication::init_language() {
	// Set the locale dir
	if (!localedir_.empty()) {
		i18n::set_localedir(g_fs->canonicalize_name(localedir_));
	} else {
		i18n::set_localedir(g_fs->canonicalize_name(datadir_ + "/locale"));
	}

	// If locale dir is not a directory, barf. We can handle it not being there tough.
	if (g_fs->file_exists(i18n::get_localedir()) && !g_fs->is_directory(i18n::get_localedir())) {
		SDL_ShowSimpleMessageBox(
		   SDL_MESSAGEBOX_ERROR, "'locale' directory not valid",
		   std::string(i18n::get_localedir() + "\nis not a directory. Please fix this.").c_str(),
		   nullptr);
		log_err("%s is not a directory. Please fix this.\n", i18n::get_localedir().c_str());
		exit(1);
	}

	if (!g_fs->is_directory(i18n::get_localedir()) ||
	    g_fs->list_directory(i18n::get_localedir()).empty()) {
		log_warn("No locale translations found in %s\n", i18n::get_localedir().c_str());
	}

	// Initialize locale and grab "widelands" textdomain
	i18n::init_locale();
	i18n::grab_textdomain("widelands", i18n::get_localedir().c_str());

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
		if (opt.size() < 2 || opt.compare(0, 2, "--")) {
			if (argc == 2) {
				// Special case of opening a savegame or replay from file browser
				if (opt.size() > kSavegameExtension.size() &&
				    0 == opt.compare(opt.size() - kSavegameExtension.size(), kSavegameExtension.size(),
				                     kSavegameExtension)) {
					commandline_["loadgame"] = opt;
					continue;
				} else if (opt.size() > kReplayExtension.size() &&
				           0 == opt.compare(opt.size() - kReplayExtension.size(),
				                            kReplayExtension.size(), kReplayExtension)) {
					commandline_["replay"] = opt;
					continue;
				}
			}
			commandline_["error"] = opt;
			break;
		} else {
			opt.erase(0, 2);  //  yes. remove the leading "--", just for cosmetics
		}

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
 * \throw a ParameterError if there were errors during parsing \e or if "--help"
 */
void WLApplication::handle_commandline_parameters() {
	auto throw_empty_value = [](const std::string& opt) {
		throw ParameterError(
		   CmdLineVerbosity::None,
		   (boost::format(_("Empty value of command line parameter: %s")) % opt).str());
	};

	auto throw_exclusive = [](const std::string& opt) {
		throw ParameterError(
		   CmdLineVerbosity::None,
		   (boost::format(_("%s can not be combined with other actions")) % opt).str());
	};

	if (commandline_.count("nosound")) {
		SoundHandler::disable_backend();
		commandline_.erase("nosound");
	}
	if (commandline_.count("verbose-i18n")) {
		i18n::enable_verbose_i18n();
		commandline_.erase("verbose-i18n");
	}
	if (commandline_.count("fail-on-lua-error")) {
		g_fail_on_lua_error = true;
		commandline_.erase("fail-on-lua-error");
	}
	if (commandline_.count("nozip")) {
		set_config_bool("nozip", true);
		commandline_.erase("nozip");
	}
	if (commandline_.count("localedir")) {
		localedir_ = commandline_["localedir"];
		commandline_.erase("localedir");
	}

	const bool skip_check_datadir_version = commandline_.count("skip_check_datadir_version");
	commandline_.erase("skip_check_datadir_version");
	auto checkdatadirversion = [skip_check_datadir_version](const std::string& dd) {
		if (skip_check_datadir_version) {
			return std::string();
		}
		try {
			std::unique_ptr<FileSystem> fs(&FileSystem::create(dd));
			if (!fs.get()) {
				return std::string("Unable to allocate filesystem");
			}

			size_t len;
			void* textptr = fs->load("datadirversion", len);
			std::string text(static_cast<char*>(textptr), len);
			free(textptr);

			size_t sep_pos = text.find_first_of("\n\r");
			if (sep_pos == std::string::npos) {
				return std::string("Malformed one-liner version string");
			}

			if (sep_pos != build_id().size() || 0 != text.compare(0, sep_pos, build_id())) {
				return std::string("Incorrect version string part");
			}

			text = text.substr(sep_pos + (text.at(sep_pos) == '\r' ? 2 : 1));
			sep_pos = text.find_first_of("\n\r");
			if (sep_pos == std::string::npos) {
				return std::string("Malformed two-liner version string");
			}

			if (sep_pos != build_type().size() || 0 != text.compare(0, sep_pos, build_type())) {
				return std::string("Incorrect type string part");
			}
		} catch (const std::exception& e) {
			return std::string(e.what());
		}
		return std::string();
	};
	if (commandline_.count("datadir")) {
		datadir_ = commandline_["datadir"];
		commandline_.erase("datadir");

		const std::string err = checkdatadirversion(datadir_);
		if (!err.empty()) {
			log_err("Invalid explicit datadir '%s': %s", datadir_.c_str(), err.c_str());
			exit(2);
		}
	} else {
		bool found = false;
		std::vector<std::pair<std::string, std::string>> wrong_candidates;

		// Try absolute path first.
		if (is_absolute_path(INSTALL_DATADIR)) {
			datadir_ = INSTALL_DATADIR;
			const std::string err = checkdatadirversion(datadir_);
			if (err.empty()) {
				found = true;
			} else {
				wrong_candidates.emplace_back(datadir_, err);
			}
		}

		// Next, pick the first applicable XDG path.
#ifdef USE_XDG
		if (!found) {
			for (const auto& datadir : FileSystem::get_xdgdatadirs()) {
				RealFSImpl dir(datadir);
				if (dir.is_directory(datadir + "/widelands")) {
					datadir_ = datadir + "/widelands";

					const std::string err = checkdatadirversion(datadir_);
					if (err.empty()) {
						found = true;
						break;
					} else {
						wrong_candidates.emplace_back(datadir_, err);
					}
				}
			}
		}
#endif

		// Finally, try a relative datadir.
		if (!found) {
			datadir_ = get_executable_directory() + FileSystem::file_separator() + INSTALL_DATADIR;
			const std::string err = checkdatadirversion(datadir_);
			if (err.empty()) {
				found = true;
			} else {
				wrong_candidates.emplace_back(datadir_, err);
			}
		}

		if (!found) {
			log_err("Unable to detect the datadir. Please specify a datadir explicitly\n"
			        "with the --datadir command line option. Tried the following %d path(s):",
			        static_cast<int>(wrong_candidates.size()));
			for (const auto& pair : wrong_candidates) {
				log_err(" · '%s': %s", pair.first.c_str(), pair.second.c_str());
			}
			exit(2);
		}
	}
	if (!is_absolute_path(datadir_)) {
		try {
			datadir_ = absolute_path_if_not_windows(FileSystem::get_working_directory() +
			                                        FileSystem::file_separator() + datadir_);
		} catch (const WException& e) {
			log_err("Error parsing datadir: %s\n", e.what());
			exit(2);
		}
	}

	if (commandline_.count("language")) {
		const std::string& lang = commandline_["language"];
		if (!lang.empty()) {
			set_config_string("language", lang);
		} else {
			init_language();
			throw_empty_value("--language");
		}
	}
	init_language();  // do this now to have translated command line help
	fill_parameter_vector();

	if (commandline_.count("error")) {
		throw ParameterError(
		   CmdLineVerbosity::Normal,
		   (boost::format(_("Unknown command line parameter: %s\nMaybe a '=' is missing?")) %
		    commandline_["error"])
		      .str());
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
		if (!filename_.empty() && *filename_.rbegin() == '/') {
			filename_.erase(filename_.size() - 1);
		}
		game_type_ = GameType::kEditor;
		commandline_.erase("editor");
	}

	if (commandline_.count("replay")) {
		if (game_type_ != GameType::kNone) {
			throw_exclusive("replay");
		}
		filename_ = commandline_["replay"];
		if (filename_.empty()) {
			throw_empty_value("--replay");
		}
		if (*filename_.rbegin() == '/') {
			filename_.erase(filename_.size() - 1);
		}
		game_type_ = GameType::kReplay;
		commandline_.erase("replay");
	}

	if (commandline_.count("new_game_from_template")) {
		if (game_type_ != GameType::kNone) {
		}
		filename_ = commandline_["new_game_from_template"];
		if (filename_.empty()) {
			throw_empty_value("--new_game_from_template");
		}
		game_type_ = GameType::kFromTemplate;
		commandline_.erase("new_game_from_template");
	}

	if (commandline_.count("loadgame")) {
		if (game_type_ != GameType::kNone) {
			throw_exclusive("loadgame");
		}
		filename_ = commandline_["loadgame"];
		if (filename_.empty()) {
			throw_empty_value("--loadgame");
		}
		if (*filename_.rbegin() == '/') {
			filename_.erase(filename_.size() - 1);
		}
		game_type_ = GameType::kLoadGame;
		commandline_.erase("loadgame");
	}

	if (commandline_.count("scenario")) {
		if (game_type_ != GameType::kNone) {
			throw_exclusive("scenario");
		}
		filename_ = commandline_["scenario"];
		if (filename_.empty()) {
			throw_empty_value("--scenario");
		}
		if (*filename_.rbegin() == '/') {
			filename_.erase(filename_.size() - 1);
		}
		game_type_ = GameType::kScenario;
		commandline_.erase("scenario");
	}
	if (commandline_.count("script")) {
		script_to_run_ = commandline_["script"];
		if (script_to_run_.empty()) {
			throw_empty_value("--script");
		}
		if (*script_to_run_.rbegin() == '/') {
			script_to_run_.erase(script_to_run_.size() - 1);
		}
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

	if (commandline_.count("version")) {
		throw ParameterError(CmdLineVerbosity::None);  // No message on purpose
	}

	if (commandline_.count("help-all")) {
		throw ParameterError(CmdLineVerbosity::All);  // No message on purpose
	}

	if (commandline_.count("help")) {
		throw ParameterError(CmdLineVerbosity::Normal);  // No message on purpose
	}

	// Override maximized and fullscreen settings for window options
	uint8_t exclusives = commandline_.count("xres") + commandline_.count("yres") +
	                     2 * commandline_.count("maximized") + 2 * commandline_.count("fullscreen");
	if (exclusives > 2) {
		throw ParameterError(CmdLineVerbosity::None,
		                     _("--xres/--yres, --maximized and --fullscreen can not be combined"));
	} else if (exclusives > 0) {
		set_config_bool("maximized", false);
		set_config_bool("fullscreen", false);
	}

	// If it hasn't been handled yet it's probably an attempt to
	// override a conffile setting
	for (const auto& pair : commandline_) {
		if (is_parameter(pair.first)) {
			if (!pair.second.empty()) {
				set_config_string(pair.first, pair.second);
			} else {
				throw_empty_value(pair.first);
			}
		} else {
			throw ParameterError(
			   CmdLineVerbosity::Normal,
			   (boost::format(_("Unknown command line parameter: %s")) % pair.first).str());
		}
	}
}

/**
 * Try to save the game instance if possible
 */
void WLApplication::emergency_save(UI::Panel* panel,
                                   Widelands::Game& game,
                                   const std::string& error,
                                   const uint8_t playernumber,
                                   const bool replace_ctrl,
                                   const bool ask_for_bug_report) {
	log_err("##############################\n"
	        "  FATAL EXCEPTION: %s\n"
	        "##############################\n",
	        error.c_str());
	if (ask_for_bug_report) {
		log_err("  Please report this problem to help us improve Widelands.\n"
		        "  You will find related messages in the standard output (stdout.txt on Windows).\n"
		        "  You are using build %s (%s).\n"
		        "  Please add this information to your report.\n",
		        build_id().c_str(), build_type().c_str());
	}
	log_err("  If desired, Widelands attempts to create an emergency savegame.\n"
	        "  It is often – though not always – possible to load it and continue playing.\n"
	        "##############################");
	if (!game.is_loaded()) {
		if (!ask_for_bug_report) {
			return;
		}
		UI::WLMessageBox m(
		   panel, UI::WindowStyle::kFsMenu, _("Error"),
		   (boost::format(
		       _("An error has occured. The error message is:\n\n%1$s\n\nPlease report "
		         "this problem to help us improve Widelands. You will find related messages in the "
		         "standard output (stdout.txt on Windows). You are using build %2$s "
		         "(%3$s).\nPlease add this information to your report.")) %
		    error % build_id() % build_type())
		      .str(),
		   UI::WLMessageBox::MBoxType::kOk);
		m.run<UI::Panel::Returncodes>();
		return;
	}

	if (panel) {
		UI::WLMessageBox m(
		   panel, UI::WindowStyle::kFsMenu,
		   ask_for_bug_report ? _("Unexpected error during the game") : _("Game ended unexpectedly"),
		   ask_for_bug_report ?
            (boost::format(_(
		          "An error occured during the game. The error message is:\n\n%1$s\n\nPlease report "
		          "this problem to help us improve Widelands. You will find related messages in the "
		          "standard output (stdout.txt on Windows). You are using build %2$s "
		          "(%3$s).\n\nPlease add this information to your report.\n\nWould you like "
		          "Widelands "
		          "to attempt to create an emergency savegame? It is often – though not always – "
		          "possible to load it and continue playing.")) %
		       error % build_id() % build_type())
		         .str() :
            (boost::format(
		          _("The game ended unexpectedly for the following reason:\n\n%s\n\nWould you like "
		            "Widelands to attempt to create an emergency savegame? It is often – though not "
		            "always – possible to load it and continue playing.")) %
		       error)
		         .str(),
		   UI::WLMessageBox::MBoxType::kOkCancel);
		if (m.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}
	}

	try {
		if (replace_ctrl) {
			game.set_game_controller(
			   std::make_shared<SinglePlayerGameController>(game, true, playernumber));
		}

		SaveHandler& save_handler = game.save_handler();
		std::string e;
		if (!save_handler.save_game(
		       game, save_handler.create_file_name(kSaveDir, timestring()), &e)) {
			throw wexception("Save handler returned error: %s", e.c_str());
		}
	} catch (const std::exception& e) {
		log_err("Emergency save failed because: %s", e.what());
		if (panel) {
			UI::WLMessageBox m(
			   panel, UI::WindowStyle::kFsMenu, _("Emergency save failed"),
			   (boost::format(_("We are sorry, but Widelands was unable to create an emergency "
			                    "savegame for the following reason:\n\n%s")) %
			    e.what())
			      .str(),
			   UI::WLMessageBox::MBoxType::kOk);
			m.run<UI::Panel::Returncodes>();
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
			log_info("Delete syncstream or replay %s\n", filename.c_str());
			try {
				g_fs->fs_unlink(filename);
			} catch (const FileError& e) {
				log_warn("WLApplication::cleanup_replays: File %s couldn't be deleted: %s\n",
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
			log_info("Deleting generated ai file: %s\n", filename.c_str());
			try {
				g_fs->fs_unlink(filename);
			} catch (const FileError& e) {
				log_warn("WLApplication::cleanup_ai_files: File %s couldn't be deleted: %s\n",
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
			log_info("Deleting old temp file: %s\n", filename.c_str());
			try {
				g_fs->fs_unlink(filename);
			} catch (const FileError& e) {
				log_warn("WLApplication::cleanup_temp_files: File %s couldn't be deleted: %s\n",
				         filename.c_str(), e.what());
			}
		}
	}
}

/**
 * Recursively delete temporary backup files in a given directory
 */
void WLApplication::cleanup_temp_backups(const std::string& dir) {
	for (const std::string& filename : g_fs->filter_directory(
	        dir, [](const std::string& fn) { return boost::ends_with(fn, kTempBackupExtension); })) {
		if (is_autogenerated_and_expired(filename, kTempBackupsKeepAroundTime)) {
			log_info("Deleting old temp backup file: %s\n", filename.c_str());
			try {
				g_fs->fs_unlink(filename);
			} catch (const FileError& e) {
				log_warn("WLApplication::cleanup_temp_backups: File %s couldn't be deleted: %s\n",
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
		if (pos == std::string::npos) {
			return false;
		}
		path.resize(pos);
#else
		path = ".";
#endif
	}
	std::string stdoutfile = path + "/stdout.txt";
	/* Redirect standard output */
	FILE* newfp = freopen(stdoutfile.c_str(), "w", stdout);
	if (!newfp) {
		return false;
	}
	/* Redirect standard error */
	std::string stderrfile = path + "/stderr.txt";
	newfp = freopen(stderrfile.c_str(), "w", stderr);
	if (!newfp) {
		return false;
	}

	/* Line buffered */
	setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);

	/* No buffering */
	setbuf(stderr, nullptr);

	redirected_stdio_ = true;
	return true;
}
