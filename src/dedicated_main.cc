/*
 * Copyright (C) 2006-2016 by the Widelands Development Team
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

#include <iostream>
#include <memory>
#include <string>

#include "base/log.h"
#include "config.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "network/internet_gaming.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		log("wl_dedicated_main <mapname>\n");
		return 1;
	}

	const std::string map_name = argv[1];

	g_fs = new LayeredFileSystem();
	g_fs->add_file_system(&FileSystem::create(INSTALL_DATADIR));

	// Widelands::Game game;
	// NOCOM(#sirver): kill dedicatedlog
	// NOCOM(#sirver): make this configurable
	if (!InternetGaming::ref().login(
	       "dedicated", "", false, INTERNET_GAMING_METASERVER, INTERNET_GAMING_PORT)) {
		log("ERROR: Could not connect to metaserver (reason above)!\n");
		return;
	}

	// try {
	// // setup some details of the dedicated server
	// Section & s = g_options.pull_section      ("global");
	// const std::string & meta   = s.get_string ("metaserver", INTERNET_GAMING_METASERVER.c_str());
	// uint32_t            port   = s.get_natural("metaserverport", INTERNET_GAMING_PORT);
	// const std::string & name   = s.get_string ("nickname",       "dedicated");
	// const std::string & server = s.get_string ("servername",     name.c_str());
	// const bool registered      = s.get_bool   ("registered",     false);
	// const std::string & pwd    = s.get_string ("password",       "");
	// for (;;) { // endless loop
	// std::string realservername(server);
	// bool name_valid = false;
	// while (!name_valid) {
	// name_valid = true;
	// const std::vector<InternetGame> & hosts = InternetGaming::ref().games();
	// for (uint32_t i = 0; i < hosts.size(); ++i) {
	// if (hosts.at(i).name == realservername)
	// name_valid = false;
	// }
	// if (!name_valid)
	// realservername += "*";
	// }

	// InternetGaming::ref().set_local_servername(realservername);

	// NetHost netgame(name, true);

	// // Load the requested map
	// Widelands::Map map;
	// map.set_filename(filename_);
	// std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(filename_);
	// if (!ml) {
	// throw WLWarning
	// ("Unsupported format",
	// "Widelands could not load the file \"%s\". The file format seems to be incompatible.",
	// filename_.c_str());
	// }
	// ml->preload_map(true);

	// // set the map
	// netgame.set_map(map.get_name(), map.get_filename(), map.get_nrplayers());

	// // run the network game
	// // -> autostarts when a player sends "/start" as pm to the server.
	// netgame.run(true);

	// InternetGaming::ref().logout();
	// }
	// } catch (const std::exception & e) {

	return 0;
}
