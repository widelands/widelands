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

#include <cstring>
#include <iostream>

#include <boost/format.hpp>

#include "base/i18n.h"

constexpr int kIndent = 22;
constexpr int kDescriptionWidth = 50;
constexpr int kMinPadding = 2;
static void print_help_option(const char* name, const char* description) {
	std::cout << name;

	const int name_length = strlen(name);
	bool needs_indent = false;
	if (name_length + kMinPadding > kIndent) {
		needs_indent = true;
	} else {
		for (int i = name_length; i < kIndent; ++i) {
			std::cout << ' ';
		}
	}

	for (;;) {
		for (;;) {
			if (*description == ' ') {
				++description;
			} else {
				break;
			}
		}
		if (*description == '\0') {
			break;
		}

		if (needs_indent) {
			std::cout << std::endl;
			for (int i = 0; i < kIndent; ++i) {
				std::cout << ' ';
			}
		}

		const int len = strlen(description);
		if (len <= kDescriptionWidth) {
			std::cout << description;
			break;
		}

		int whitespace_pos = -1;
		for (int i = 0; i < kDescriptionWidth; ++i) {
			if (description[i] == ' ') {
				whitespace_pos = i;
			}
		}
		if (whitespace_pos < 0) {
			whitespace_pos = len;
			for (int i = kDescriptionWidth; i < len; ++i) {
				if (description[i] == ' ') {
					whitespace_pos = i;
					break;
				}
			}
		}

		for (int i = 0; i < whitespace_pos; ++i, ++description) {
			std::cout << description[0];
		}
		needs_indent = true;
	}
	std::cout << std::endl;
}

/**
 * Print usage information
 */
void show_usage() {
	i18n::Textdomain textdomain("widelands_console");

	std::cout << std::endl << _("Usage:") << std::endl <<
			"       " << _("widelands <option0>=<value0> ... <optionN>=<valueN>") << std::endl <<
			"       " << _("widelands <save.wgf>/<replay.wrpl>") << std::endl << std::endl;

	std::cout << _("General options:") << std::endl << std::endl;
	print_help_option(_("--<config-entry-name>=value"), _("Overwrites any config file setting. Note: The new value will be written to the config file."));
#ifndef _WIN32
	print_help_option(_("--homedir=DIRNAME"), _("Use specified directory for Widelands config files, savegames and replays. Default is ~/.widelands"));
#else
	print_help_option(_("--homedir=DIRNAME"), _("Use specified directory for Widelands config files, savegames and replays. Default is %USERPROFILE%\\.widelands"));
#endif

	print_help_option(_("--localedir=DIRNAME"), _("Use specified directory for the Widelands locale files."));
	print_help_option(
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but it HAS TO BE CONSISTENT with the translation in the widelands textdomain */
	_("--coredump=[true|false]"),
		_("Generates a core dump on segfaults instead of using the SDL."));
	print_help_option(_("--language=[de_DE|sv_SE|...]"), _("The locale to use."));
	print_help_option(
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but it HAS TO BE CONSISTENT with the translation in the widelands textdomain. */
	          /** TRANSLATORS: A syncstream is a synchronization stream. Syncstreams are used in multiplayer games to make sure that there is no mismatch between the players. */
	_("--write_syncstreams=[true|false]"), _("Create syncstream dump files to help debug network games."));
	print_help_option(_("--autosave=[...]"), _("Automatically save each n minutes."));
	print_help_option(_("--rolling_autosave=[...]"), _("Use this many files for rolling autosaves."));
	print_help_option(_("--metaserver=[...]"), _("Connect to a different metaserver for internet gaming."));
	print_help_option(_("--metaserverport=[...]"), _("Port number of the metaserver for internet gaming."));
	print_help_option(_("--addon_server=[...]"), _("Connect to a different github repository and branch from the add-ons manager."));
	print_help_option(_("--nosound"), _("Starts the game with sound disabled."));
	print_help_option(_("--fail-on-lua-error"), _("Force Widelands to crash when a Lua error occurs."));
	print_help_option(_("--nozip"), _("Do not save files as binary zip archives."));
	print_help_option(_("--editor"), _("Directly starts the Widelands editor."));
	print_help_option(_("--editor=FILENAME"), _("Directly starts the Widelands editor and loads the map FILENAME."));
	print_help_option(_("--ai_training"), _("Enables AI training mode. See https://www.widelands.org/wiki/Ai%20Training/ for a full description of the AI training logic."));
	print_help_option(_("--auto_speed"), _("In multiplayer games only, this will keep adjusting the game speed automatically, depending on FPS. Useful in conjunction with --ai_training."));
	print_help_option(_("--new_game_from_template=FILENAME"), _("Directly create a new singleplayer game configured in the given file. An example can be found in `data/templates/new_game_template`."));
	print_help_option(_("--scenario=FILENAME"), _("Directly starts the map FILENAME as scenario map."));
	print_help_option(_("--loadgame=FILENAME"), _("Directly loads the savegame FILENAME."));
	print_help_option(_("--replay=FILENAME"), _("Directly loads the replay FILENAME."));
	print_help_option(_("--script=FILENAME"), _("Run the given Lua script after initialization. Only valid with --scenario, --loadgame, or --editor."));
	print_help_option(
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but it HAS TO BE CONSISTENT with the translation in the widelands textdomain */
	_("--auto_roadbuild_mode=[true|false]"), _("Whether to enter roadbuilding mode automatically after placing a flag that is not connected to a road."));

	std::cout << std::endl << _("Graphic options:") << std::endl << std::endl;
	print_help_option(
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but it HAS TO BE CONSISTENT with the translation in the widelands textdomain */
	_("--fullscreen=[true|false]"), _("Whether to use the whole display for the game screen."));
	print_help_option(_("--xres=[...]"), _("Width of the window in pixels."));
	print_help_option(_("--yres=[...]"), _("Height of the window in pixels."));
	print_help_option(_("--maxfps=[5 ...]"), _("Maximal optical framerate of the game."));

	std::cout << std::endl << _("Options for the internal window manager:") << std::endl << std::endl;
	print_help_option(
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but it HAS TO BE CONSISTENT with the translation in the widelands textdomain */
	_("--animate_map_panning=[true|false]"), _("Should automatic map movements be animated."));
	print_help_option(_("--border_snap_distance=[0 ...]"), _("Move a window to the edge of the screen when the edge of the window comes within this distance from the edge of the screen."));
	print_help_option(
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but it HAS TO BE CONSISTENT with the translation in the widelands textdomain */
	_("--dock_windows_to_edges=[true|false]"), _("Eliminate a window’s border towards the edge of the screen when the edge of the window is next to the edge of the screen."));
	print_help_option(_("--panel_snap_distance=[0 ...]"), _("Move a window to the edge of the panel when the edge of the window comes within this distance from the edge of the panel."));
	print_help_option(
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but it HAS TO BE CONSISTENT with the translation in the widelands textdomain */
	_("--snap_windows_only_when_overlapping=[true|false]"), _("Only move a window to the edge of a panel if the window is overlapping with the panel."));

	std::cout << std::endl << _("Other options:") << std::endl << std::endl;
	print_help_option(_("--verbose"), _("Enable verbose debug messages."));
	print_help_option(_("--quiet"), _("Suppress all output."));
	print_help_option(_("--version"), _("Show the version and exit."));
	print_help_option(_("--help"), _("Show this help and exit."));
	print_help_option(_("<save.wgf>/<replay.wrpl>"), _("Directly loads the given savegame or replay. Useful for .wgf/.wrpl file extension association. Does not work with other options. Also see --loadgame/--replay."));

	std::cout << std::endl << _("Bug reports? Suggestions? Check out the project website:")
	               << std::endl << "        https://www.widelands.org/" << std::endl
	               << _("Hope you enjoy this game!") << std::endl;
}
