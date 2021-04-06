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

using std::endl;

static size_t kIndent = 23;

#ifndef _WIN32
static std::string kDefaultHomedir = "~/.widelands";
#else
static std::string kDefaultHomedir = "%USERPROFILE%\\.widelands";
#endif

/// Command line help
/// Title: unindented text in the line above
/// Key: the actual parameter
/// Hint: text after =
/// Help: Full text help
/// Verbose: Filter some config options (--help vs. --help-all)
/// Note that the _() markup is only for extraction. Since no textdomain is available in
/// static context these should be marked again on usage.
static std::vector<Parameter> parameters = {
   {_("\nUsage:"), _("\twidelands <option0>=<value0> ... <optionN>=<valueN>"), "--", "", false},
   {"", _("\twidelands <save.wgf>/<replay.wrpl>"), "--", "", false},
   {_("\nOptions:"), _("<config-entry-name>"), _("value"),
    _("value overwrites any config file setting\nNote: New value will be written to config file"),
    false},
   /// Paths
   {" ", "datadir", _("DIRNAME"), _("Use specified directory for the widelands\ndata files"),
    false},
   {"", "homedir", _("DIRNAME"),
    _("Use specified directory for widelands config\nfiles, savegames and replays\nDefault is ") +
       kDefaultHomedir,
    false},
   {"", "localedir", _("DIRNAME"), _("Use specified directory for the widelands\nlocale files"),
    false},
   {" ", "language", _("[de_DE|sv_SE|...]"), _("The locale to use"), false},
   /// Game setup
   {" ", "new_game_from_template", _("FILENAME"),
    _("Directly create a new singleplayer game\nconfigured in the given file. An example can\nbe "
      "found in `data/templates/new_game_template`"),
    false},
   {"", "scenario", _("FILENAME"), _("Directly starts the map FILENAME as scenario map"), false},
   {"", "loadgame", _("FILENAME"), _("Directly loads the savegame FILENAME"), false},
   {"", "replay", _("FILENAME"), _("Directly loads the replay FILENAME"), false},
   {"", "script", _("FILENAME"),
    _("Run the given Lua script after initialization.\nOnly valid with --scenario, --loadgame, or "
      "--editor"),
    false},
   {"", "editor", "",
    _("Directly starts the Widelands editor.\nYou can add a =FILENAME to directly load\nthe map "
      "FILENAME in editor"),
    false},
   /// Misc
   {" ", "nosound", "", _("Starts the game with sound disabled"), false},
   {" ", "fail-on-lua-error", "", _("Force Widelands to crash when a Lua error occurs"), false},
   {"", "ai_training", "",
    _("Enables AI training mode. See\nhttps://www.widelands.org/wiki/Ai%20Training/\nfor a full "
      "description of the AI training logic"),
    false},
   {"", "auto_speed", "",
    _("In multiplayer games only, this will keep\nadjusting the game speed "
      "automatically,\ndepending on FPS. Useful in conjunction with\n--ai_training"),
    false},
   /// Saving options
   {"", "autosave", _("n"), _("Automatically save each n minutes"), false},
   {"", "rolling_autosave", _("n"), _("Use n files for rolling autosaves"), true},
   {"", "nozip", "", _("Do not save files as binary zip archives"), false},
   {"", "display_replay_filenames", _("[true*|false]"), _("Show filenames in replay screen"), true},
   {"", "editor_player_menu_warn_too_many_players", _("[true*|false]"),
    _("Enable verbose debug messages"), true},
   /// Game options
   {"", "auto_roadbuild_mode", _("[true*|false]"), _("Start building road after placing a flag"),
    true},
   {"", "display_flags", _("[...]"), _("Display flags to set for new games"), true},
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
    _("[true|false*]"), _("Scroll when the mouse cursor is near the screen edge"), true},
   {"", "numpad_diagonalscrolling", _("[true|false*]"),
    _("Allow diagonal scrolling with the numeric keypad"), true},
   {"", "game_clock", _("[true|false*]"), _("Display system time in the info panel"), true},
   {"", "ctrl_zoom", _("[true|false*]"), _("Zoom only when Ctrl is pressed"), true},
   {"", "single_watchwin", _("[true|false*]"), _("Use single watchwindow mode"), true},
   {"", "transparent_chat", _("[true*|false]"), _("Show in-game chat with transparent background"),
    true},
   {"", "toolbar_pos", _("[...]"), _("Sets the toolbar location and mode"), true},
   /// Networking
   {"\nNetworking:", "write_syncstreams",
    /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but */
    /** TRANSLATORS: it HAS TO BE CONSISTENT with the translation in the widelands textdomain. */
    _("[true*|false]"),
    /** TRANSLATORS: A syncstream is a synchronization stream. Syncstreams are used in multiplayer
     */
    /** TRANSLATORS: games to make sure that there is no mismatch between the players. */
    _("Create syncstream dump files to help debug network games"), false},
   {"", "metaserver", _("URI"), _("Connect to a different metaserver for internet gaming"), false},
   {"", "metaserverport", _("n"), _("Port number n of the metaserver for internet gaming"), false},
   {"", "servername", _("[...]"), _("The name of the last hosted game"), true},
   {"", "realname", _("[...]"), _("The nickname used for LAN and online games"), true},
   {"", "nickname", _("[...]"), _("Name of map author"), true},
   {"", "addon_server", _("URI"),
    _("Connect to a different github repository\nand branch from the add-ons manager"), false},
   /// Interface options
   {_("\nGraphic options:"), "fullscreen", _("[true|false*]"),
    _("Whether to use the whole display for the\ngame screen"), false},
   {"", "maximized", _("[true|false*]"), _("Whether to use start the game in a maximized window"),
    false},
   {"", "xres", _("x"), _("Width x of the window in pixel"), false},
   {"", "yres", _("y"), _("Height y of the window in pixel"), false},
   {"", "inputgrab", _("[true|false*]"), _("Whether to grab the mouse input"), true},
   {"", "sdl_cursor", _("[true*|false]"), _("Whether to use the mouse cursor provided by SDL"),
    true},
   {"", "tooltip_accessibility_mode", _("[true|false*]"), _("Whether to use sticky tooltips"),
    true},
   {"", "maxfps", _("n"), _("Maximal optical framerate n of the game"), true},
   {"", "theme", _("[...]"), _("The active UI theme"), false},
   /// Window options
   {_("\nOptions for the internal window manager:"), "animate_map_panning", _("[true*|false]"),
    _("Should automatic map movements be animated"), true},
   {"", "border_snap_distance", _("n"),
    _("Move a window to the edge of the screen\nwhen the edge of the window comes within\na "
      "distance n from the edge of the screen"),
    true},
   {"", "dock_windows_to_edges", _("[true|false*]"),
    _("Eliminate a window’s border towards the\nedge of the screen when the edge of the\nwindow is "
      "next to the edge of the screen"),
    true},
   {"", "panel_snap_distance", _("n"),
    _("Move a window to the edge of the panel when\nthe edge of the window comes within "
      "a\ndistance of n from the edge of the panel"),
    true},
   {"", "snap_windows_only_when_overlapping", _("[true|false*]"),
    _("Only move a window to the edge of a panel\nif the window is overlapping with the panel"),
    true},
   /// Others
   {"\nOthers:", "verbose", "", _("Enable verbose debug messages"), false},
   {"", "version", "", _("Only print version and exit"), false},
   {"", "help", "", _("Show this help"), false},
   {"", "help-all", "", _("Show this help with all available config options"), false},
   {" ", _("<save.wgf>/<replay.wrpl>"), "--",
    _("Directly loads the given savegame or replay. Useful for\n.wgf/.wrpl file extension "
      "association. Does not work with\nother options. Also see --loadgame/--replay"),
    false}};

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
	i18n::Textdomain textdomain("widelands_console");  //  uses system standard language

	std::cout << std::string(60, '=')
	          << endl
	          /** TRANSLATORS: %s = version information */
	          << (boost::format(_("This is Widelands Build %s")) %
	              (boost::format("%s(%s)") % build_id % build_type).str())
	                .str()
	          << endl;

	if (verbosity != CmdLineVerbosity::None) {
		std::string indent_string = std::string(kIndent, ' ');
		for (const Parameter& param : parameters) {
			if (verbosity != CmdLineVerbosity::All && param.is_verbose_) {
				continue;
			}

			if (!param.title_.empty()) {
				// Duplicate gettext markup to ensure being in the correct textdomain
				std::cout << _(param.title_) << endl;
			}

			std::string column = " ";
			if (param.hint_ == "--") {
				// option without dashes
				column += _(param.key_);
			} else {
				column += std::string("--") + _(param.key_);
				if (!param.hint_.empty()) {
					column += std::string("=") + _(param.hint_);
				}
			}

			std::cout << column;
			if (param.help_.empty()) {
				std::cout << endl;
				continue;
			}

			if (column.size() >= kIndent) {
				std::cout << endl << indent_string;
			} else {
				std::cout << std::string(kIndent - column.size(), ' ');
			}

			std::string help =
			   std::regex_replace(_(param.help_), std::regex("\\n"), "\n" + indent_string);
			std::cout << help << endl;
		}
	}

	std::cout << endl
	          << _("Bug reports? Suggestions? Check out the project website:\n"
	               "        https://www.widelands.org/\n\n"
	               "Hope you enjoy this game!")
	          << endl;
}
