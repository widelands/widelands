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
#include "base/warning.h"
#include "base/wexception.h"
#include "config.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "map_io/map_loader.h"
#include "network/internet_gaming.h"
#include "network/nethost.h"

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
	// if (!InternetGaming::ref().login(
			 // "dedicated", "", false, INTERNET_GAMING_METASERVER, INTERNET_GAMING_PORT)) {
		// log("ERROR: Could not connect to metaserver (reason above)!\n");
		// return 1;
	// }

	// try {
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

	InternetGaming::ref().set_local_servername("dedicated_testing");

	NetHost netgame("dedicated", true);

	// // Load the requested map
	Widelands::Map map;
	map.set_filename(map_name);
	std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(map_name);
	if (!ml) {
		throw WLWarning(
		   "Unsupported format",
		   "Widelands could not load the file \"%s\". The file format seems to be incompatible.",
		   map_name.c_str());
	}
	ml->preload_map(true);

	// set the map
	netgame.set_map(map.get_name(), map.get_filename(), map.get_nrplayers());

	// // run the network game
	// // -> autostarts when a player sends "/start" as pm to the server.
	netgame.run(true);

	// }
	// } catch (const std::exception & e) {

	// InternetGaming::ref().logout();

	return 0;
}
