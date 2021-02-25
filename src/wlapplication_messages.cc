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

#include <iostream>

#include <boost/format.hpp>

#include "base/i18n.h"

using std::endl;

/**
 * Print usage information
 */
void show_usage(const std::string& build_id, const std::string& build_type) {
	i18n::Textdomain textdomain("widelands_console");  //  uses system standard language

	/** TRANSLATORS: %s = version information */
	std::cout << (boost::format(_("This is Widelands Build %s")) %
	              (boost::format("%s(%s)") % build_id % build_type).str())
	                .str()
	          << endl
	          << endl;
	std::cout << _("Usage: widelands <option0>=<value0> ... <optionN>=<valueN>") << endl;
	std::cout << _("       widelands <save.wgf>/<replay.wrpl>") << endl << endl;
	std::cout << _("Options:") << endl << endl;
	std::cout << _(" --<config-entry-name>=value overwrites any config file setting") << endl
	          << _("                      Note: New value will be written to config file") << endl
	          << endl
	          << _(" --datadir=DIRNAME    Use specified directory for the widelands\n"
	               "                      data files")
	          << endl
	          << _(" --homedir=DIRNAME    Use specified directory for widelands config\n"
	               "                      files, savegames and replays")
	          << endl
#ifndef _WIN32
	          << _("                      Default is ~/.widelands") << endl
#else
	          << _("                      Default is %USERPROFILE%\\.widelands") << endl
#endif
	          << _(" --localedir=DIRNAME  Use specified directory for the widelands\n"
	               "                      locale files")
	          << endl
	          << endl
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but */
	          /** TRANSLATORS: it HAS TO BE CONSISTENT with the translation in the widelands
	             textdomain */
	          << _(" --coredump=[true|false]\n"
	               "                      Generates a core dump on segfaults instead of\n"
	               "                      using the SDL")
	          << endl
	          << _(" --language=[de_DE|sv_SE|...]\n"
	               "                      The locale to use.")
	          << endl
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but */
	          /** TRANSLATORS: it HAS TO BE CONSISTENT with the translation in the widelands
	             textdomain. */
	          /** TRANSLATORS: A syncstream is a synchronization stream. Syncstreams are used in
	             multiplayer */
	          /** TRANSLATORS: games to make sure that there is no mismatch between the players. */
	          << _(" --write_syncstreams=[true|false]\n"
	               "                      Create syncstream dump files to help debug network games.")
	          << endl
	          << _(" --autosave=[...]     Automatically save each n minutes") << endl
	          << _(" --rolling_autosave=[...]\n"
	               "                      Use this many files for rolling autosaves")
	          << endl
	          << _(" --metaserver=[...]\n"
	               "                      Connect to a different metaserver for internet gaming.")
	          << endl
	          << _(" --metaserverport=[...]\n"
	               "                      Port number of the metaserver for internet gaming.")
	          << endl
	          << _(" --addon_server_ip=[...]\n"
	               " --addon_server_port=[...]\n"
	               "                      Connect to a different server or port\n"
	               "                      from the add-ons manager.") << endl
	          << endl
	          << endl
	          << _(" --nosound            Starts the game with sound disabled.") << endl
	          << endl
	          << _(" --fail-on-lua-error  Force Widelands to crash when a Lua error occurs.") << endl
	          << endl
	          << _(" --nozip              Do not save files as binary zip archives.") << endl
	          << endl
	          << _(" --editor             Directly starts the Widelands editor.\n"
	               "                      You can add a =FILENAME to directly load\n"
	               "                      the map FILENAME in editor.")
	          << endl
	          << _(" --ai_training        Enables AI training mode. See\n"
	               "                      https://www.widelands.org/wiki/Ai%20Training/\n"
	               "                      for a full description of the AI training logic.")
	          << endl
	          << _(" --auto_speed         In multiplayer games only, this will keep\n"
	               "                      adjusting the game speed automatically,\n"
	               "                      depending on FPS. Useful in conjunction with\n"
	               "                      --ai_training.")
	          << endl
	          << _(" --new_game_from_template=FILENAME\n"
	               "                      Directly create a new singleplayer game\n"
	               "                      configured in the given file. An example can\n"
	               "                      be found in `data/templates/new_game_template`.")
	          << endl
	          << endl
	          << _(" --scenario=FILENAME  Directly starts the map FILENAME as scenario\n"
	               "                      map.")
	          << endl
	          << _(" --loadgame=FILENAME  Directly loads the savegame FILENAME.") << endl
	          << _(" --replay=FILENAME    Directly loads the replay FILENAME.") << endl
	          << _(" --script=FILENAME    Run the given Lua script after initialization.\n"
	               "                      Only valid with --scenario, --loadgame, or --editor.")
	          << endl
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but */
	          /** TRANSLATORS: it HAS TO BE CONSISTENT with the translation in the widelands
	             textdomain */
	          << _(" --auto_roadbuild_mode=[true|false]\n"
	               "                      Whether to enter roadbuilding mode\n"
	               "                      automatically after placing a flag that is\n"
	               "                      not connected to a road.")
	          << endl
	          << endl
	          << _("Graphic options:")
	          << endl
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but */
	          /** TRANSLATORS: it HAS TO BE CONSISTENT with the translation in the widelands
	             textdomain */
	          << _(" --fullscreen=[true|false]\n"
	               "                      Whether to use the whole display for the\n"
	               "                      game screen.")
	          << endl
	          << _(" --xres=[...]         Width of the window in pixel.") << endl
	          << _(" --yres=[...]         Height of the window in pixel.") << endl
	          << _(" --maxfps=[5 ...]     Maximal optical framerate of the game.") << endl
	          << endl
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but */
	          /** TRANSLATORS: it HAS TO BE CONSISTENT with the translation in the widelands
	             textdomain */
	          << _("Options for the internal window manager:") << endl
	          << _(" --animate_map_panning=[yes|no]\n"
	               "                      Should automatic map movements be animated.")
	          << endl
	          << _(" --border_snap_distance=[0 ...]\n"
	               "                      Move a window to the edge of the screen\n"
	               "                      when the edge of the window comes within\n"
	               "                      this distance from the edge of the screen.")
	          << endl
	          /** TRANSLATORS: You may translate true/false, also as on/off or yes/no, but */
	          /** TRANSLATORS: it HAS TO BE CONSISTENT with the translation in the widelands
	             textdomain */
	          << _(" --dock_windows_to_edges=[true|false]\n"
	               "                      Eliminate a window's border towards the\n"
	               "                      edge of the screen when the edge of the\n"
	               "                      window is next to the edge of the screen.")
	          << endl
	          << _(" --panel_snap_distance=[0 ...]\n"
	               "                      Move a window to the edge of the panel when\n"
	               "                      the edge of the window comes within this\n"
	               "                      distance from the edge of the panel.")
	          << endl
	          << _(" --snap_windows_only_when_overlapping=[yes|no]\n"
	               "                      Only move a window to the edge of a panel\n"
	               "                      if the window is overlapping with the\n"
	               "                      panel.")
	          << endl
	          << endl;
	std::cout << _(" --verbose            Enable verbose debug messages") << endl << endl;
	std::cout << _(" --help               Show this help") << endl << endl;
	std::cout
	   << _(" <save.wgf>/<replay.wrpl> \n"
	        "                      Directly loads the given savegame or replay. Useful for\n"
	        "                      .wgf/.wrpl file extension association. Does not work with\n"
	        "                      other options. Also see --loadgame/--replay.")
	   << endl
	   << endl;
	std::cout << _("Bug reports? Suggestions? Check out the project website:\n"
	               "        https://www.widelands.org/\n\n"
	               "Hope you enjoy this game!")
	          << endl;
}
