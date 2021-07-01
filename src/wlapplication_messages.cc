/*
 * Copyright (C) 2012-2021 by the Widelands Development Team
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

#include "wlapplication_messages.h"

#include <algorithm>
#include <iostream>
#include <regex>

#include <boost/format.hpp>

#include "base/i18n.h"

constexpr size_t kIndent = 23;
constexpr size_t kTextWidth = 50;

#ifndef _WIN32
static const std::string kDefaultHomedir = "~/.widelands";
#else
static const std::string kDefaultHomedir = "%USERPROFILE%\\.widelands";
#endif

/// Command line help
/// Title: unindented text in the line above
/// Key: the actual parameter
/// Hint: text after =
/// Help: Full text help
/// Verbose: Filter some config options (--help vs. --help-all)
static std::vector<Parameter> parameters;
void fill_parameter_vector() {
	i18n::Textdomain textdomain("widelands_console");
	parameters =
	{ {_("Usage:"), _("widelands <option0>=<value0> ... <optionN>=<valueN>"), "--", "", false},
	  {"", _("widelands <save.wgf>/<replay.wrpl>"), "--", "", false},
	  /// Paths
	  {_("Options:"), "datadir", _("DIRNAME"),
		_("Use specified directory for the widelands data files"), false},
	  {"", "homedir", _("DIRNAME"),
		_("Use specified directory for widelands config files, savegames and replays. Default is ") +
		   kDefaultHomedir,
		false},
	  {"", "localedir", _("DIRNAME"), _("Use specified directory for the widelands locale files"),
		false},
	  {"", "language",
		/** TRANSLATORS: The … is not used on purpose to increase readability on monospaced terminals
		 */
		_("[de_DE|sv_SE|...]"), _("The locale to use"), false},
	  {"", "skip_check_datadir_version", "",
		_("Do not check whether the data directory to use is compatible with this Widelands version"),
		true},
	  /// Game setup
	  {"", "new_game_from_template", _("FILENAME"),
		_("Directly create a new singleplayer game configured in the given file. An example can be "
		  "found in `data/templates/new_game_template`"),
		false},
	  {"", "scenario", _("FILENAME"), _("Directly starts the map `FILENAME` as scenario map"),
		false},
	  {"", "loadgame", _("FILENAME"), _("Directly loads the savegame `FILENAME`"), false},
	  {"", "replay", _("FILENAME"), _("Directly loads the replay `FILENAME`"), false},
	  {"", "script", _("FILENAME"),
		_("Run the given Lua script after initialization. Only valid with --scenario, --loadgame, or "
		  "--editor"),
		false},
	  {"", "editor", "",
		_("Directly starts the Widelands editor. You can add a =FILENAME to directly load the map "
		  "`FILENAME` in editor"),
		false},
	  /// Misc
	  {"", "nosound", "", _("Starts the game with sound disabled"), false},
	  {"", "fail-on-lua-error", "", _("Force Widelands to crash when a Lua error occurs"), false},
	  {"", "ai_training", "",
		_("Enables AI training mode. See https://www.widelands.org/wiki/Ai%20Training/ for a full "
		  "description of the AI training logic"),
		false},
	  {"", "auto_speed", "",
		_("In multiplayer games only, this will keep adjusting the game speed "
		  "automatically, depending on FPS. Useful in conjunction with --ai_training"),
		false},
	  /// Saving options
	  {_("Game options:"), _("Note: New values will be written to config file"), "--", "", false},
	  {"", "autosave",
		/** TRANSLATORS: A placeholder for a numerical value */
		_("n"),
		/** TRANSLATORS: `n` references a numerical placeholder */
		_("Automatically save each `n` minutes"), false},
	  {"", "rolling_autosave", _("n"),
		/** TRANSLATORS: `n` references a numerical placeholder */
		_("Use `n` files for rolling autosaves"), true},
	  {"", "nozip", "", _("Do not save files as binary zip archives"), false},
	  {"", "display_replay_filenames", _("[true*|false]"), _("Show filenames in replay screen"),
		true},
	  {"", "editor_player_menu_warn_too_many_players", _("[true*|false]"),
		_("Whether a warning should be shown in the editor if there are too many players"), true},
	  /// Game options
	  {"", "auto_roadbuild_mode", _("[true*|false]"), _("Start building road after placing a flag"),
		true},
	  {"", "display_flags",
		/** TRANSLATORS: The … character is not used on purpose to increase readability on monospaced
		   terminals */
		_("[...]"), _("Bitmask of display flags to set for new games"), true},
#if 0  // TODO(matthiakl): Re-add training wheels code after v1.0
	{"",
	 "training_wheels",
	 _("[true*|false]"),
	 "",
	 true
	},
#endif
	  {"", "edge_scrolling",
		/** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but */
		/** TRANSLATORS: it HAS TO BE CONSISTENT with the translation in the widelands textdomain. */
		/** TRANSLATORS: * marks the default value */
		_("[true|false*]"), _("Scroll when the mouse cursor is near the screen edge"), true},
	  {"", "invert_movement", _("[true|false*]"), _("Invert click-and-drag map movement direction"),
		true},
	  {"", "numpad_diagonalscrolling", _("[true|false*]"),
		_("Allow diagonal scrolling with the numeric keypad"), true},
	  {"", "game_clock", _("[true|false*]"), _("Display system time in the info panel"), true},
	  {"", "ctrl_zoom", _("[true|false*]"), _("Zoom only when Ctrl is pressed"), true},
	  {"", "single_watchwin", _("[true|false*]"), _("Use single watchwindow mode"), true},
	  {"", "transparent_chat", _("[true*|false]"),
		_("Show in-game chat with transparent background"), true},
	  {"", "toolbar_pos", _("[...]"), _("Bitmask to set the toolbar location and mode"), true},
	  /// Networking
	  {_("Networking:"), "write_syncstreams",
		/** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but */
		/** TRANSLATORS: it HAS TO BE CONSISTENT with the translation in the widelands textdomain. */
		/** TRANSLATORS: * marks the default value */
		_("[true*|false]"),
		/** TRANSLATORS: A syncstream is a synchronization stream. Syncstreams are used in multiplayer
		 */
		/** TRANSLATORS: games to make sure that there is no mismatch between the players. */
		_("Create syncstream dump files to help debug network games"), false},
	  {"", "metaserver", _("URI"), _("Connect to a different metaserver for internet gaming"),
		false},
	  {"", "metaserverport", _("n"),
		/** TRANSLATORS: `n` references a numerical placeholder */
		_("Port number `n` of the metaserver for internet gaming"), false},
	  {"", "servername", _("[...]"), _("The name of the last hosted game"), true},
	  {"", "nickname", _("[...]"), _("The nickname used for LAN and online games"), true},
	  {"", "realname", _("[...]"), _("Name of map author"), true},
	  {"", "lasthost", _("[...]"), _("The last host connected to"), true},
	  {"", "registered", _("[true|false*]"),
		_("Whether the used metaserver login is for a registered user"), true},
	  {"", "password_sha1", _("[...]"), _("The hashed password for online logins"), true},
	  {"", "addon_server", _("URI"),
		_("Connect to a different github repository and branch from the add-ons manager"), false},
	  /// Interface options
	  {_("Graphic options:"), "fullscreen", _("[true|false*]"),
		_("Whether to use the whole display for the game screen"), false},
	  {"", "maximized", _("[true|false*]"), _("Whether to start the game in a maximized window"),
		false},
	  {"", "xres",
		/** TRANSLATORS: A placeholder for window width */
		_("x"),
		/** TRANSLATORS: `x` references a window width placeholder */
		_("Width `x` of the window in pixel"), false},
	  {"", "yres",
		/** TRANSLATORS: A placeholder for window height */
		_("y"),
		/** TRANSLATORS: `y` references a window height placeholder */
		_("Height `y` of the window in pixel"), false},
	  {"", "inputgrab", _("[true|false*]"), _("Whether to grab the mouse input"), true},
	  {"", "sdl_cursor", _("[true*|false]"), _("Whether to use the mouse cursor provided by SDL"),
		true},
	  {"", "tooltip_accessibility_mode", _("[true|false*]"), _("Whether to use sticky tooltips"),
		true},
	  {"", "maxfps", _("n"),
		/** TRANSLATORS: `n` references a numerical placeholder */
		_("Maximal optical framerate `n` of the game"), true},
	  {"", "theme", _("DIRNAME"), _("The path to the active UI theme relative to the homedir"),
		false},
	  /// Window options
	  {_("Options for the internal window manager:"), "animate_map_panning", _("[true*|false]"),
		_("Should automatic map movements be animated"), true},
	  {"", "border_snap_distance", _("n"),
		/** TRANSLATORS: `n` references a numerical placeholder */
		_("Move a window to the edge of the screen when the edge of the window comes within a "
		  "distance `n` from the edge of the screen"),
		true},
	  {"", "dock_windows_to_edges", _("[true|false*]"),
		_("Eliminate a window’s border towards the edge of the screen when the edge of the window is "
		  "next to the edge of the screen"),
		true},
	  {"", "panel_snap_distance", _("n"),
		/** TRANSLATORS: `n` references a numerical placeholder */
		_("Move a window to the edge of the panel when the edge of the window comes within "
		  "a distance of `n` from the edge of the panel"),
		true},
	  {"", "snap_windows_only_when_overlapping", _("[true|false*]"),
		_("Only move a window to the edge of a panel if the window is overlapping with the panel"),
		true},
	  /// Others
	  {_("Others:"), "verbose", "", _("Enable verbose debug messages"), false},
	  {"", "version", "", _("Only print version and exit"), false},
	  {"", "help", "", _("Show this help"), false},
	  {"", "help-all", "", _("Show this help with all available config options"), false},
	  {"", _("<save.wgf>/<replay.wrpl>"), "--",
		_("Directly loads the given savegame or replay. Useful for .wgf/.wrpl file extension "
		  "association. Does not work with other options. Also see --loadgame/--replay"),
		false} };
}

const std::vector<std::string> get_all_parameters() {
	std::vector<std::string> result(parameters.size());
	std::transform(parameters.begin(), parameters.end(), result.begin(),
	               [](const Parameter& p) { return p.key_; });
	return result;
}

bool is_parameter(const std::string& name) {
	auto result = std::find_if(
	   parameters.begin(), parameters.end(), [name](const Parameter& p) { return p.key_ == name; });
	return result != parameters.end();
}

/**
 * Print usage information
 */
void show_usage(const std::string& build_id,
                const std::string& build_type,
                CmdLineVerbosity verbosity) {
	i18n::Textdomain textdomain("widelands_console");

	std::cout << std::string(kIndent + kTextWidth, '=')
	          << std::endl
	          /** TRANSLATORS: %s = version information */
	          << (boost::format(_("This is Widelands Version %s")) %
	              (boost::format("%s(%s)") % build_id % build_type).str())
	                .str()
	          << std::endl;

	if (verbosity != CmdLineVerbosity::None) {
		std::string indent_string = std::string(kIndent, ' ');
		bool multiline = true;
		for (const Parameter& param : parameters) {
			if (verbosity != CmdLineVerbosity::All && param.is_verbose_) {
				continue;
			}

			if (!param.title_.empty()) {
				std::cout << std::endl << param.title_ << std::endl;
			} else if (!multiline) {
				// Space out single line entries
				std::cout << std::endl;
			}

			std::string column = " ";
			if (param.hint_ == "--") {
				// option without dashes
				column += param.key_;
			} else {
				column += std::string("--") + param.key_;
				if (!param.hint_.empty()) {
					column += std::string("=") + param.hint_;
				}
			}

			std::cout << column;
			if (param.help_.empty()) {
				std::cout << std::endl;
				continue;
			}

			multiline = column.size() >= kIndent;
			if (multiline) {
				std::cout << std::endl << indent_string;
			} else {
				std::cout << std::string(kIndent - column.size(), ' ');
			}

			std::string help = param.help_;
			multiline |= help.size() > kTextWidth;
			while (help.size() > kTextWidth) {
				// Auto wrap lines wider than text width
				size_t space_idx = help.rfind(' ', kTextWidth);
				if (space_idx != std::string::npos) {
					std::cout << help.substr(0, space_idx) << std::endl << indent_string;
					help = help.substr(space_idx + 1);
				} else {
					break;
				}
			}
			std::cout << help << std::endl;
		}
	}

	std::cout << std::endl
	          << _("Bug reports? Suggestions? Check out the project website:\n"
	               "        https://www.widelands.org/\n\n"
	               "Hope you enjoy this game!")
	          << std::endl;
}
