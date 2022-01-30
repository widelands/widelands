/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "game_io/game_map_packet.h"

#include <memory>

#include "io/filesystem/filesystem.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "map_io/map_saver.h"
#include "map_io/widelands_map_loader.h"

namespace Widelands {

GameMapPacket::~GameMapPacket() {
	delete wms_;
	delete wml_;
}

void GameMapPacket::read(FileSystem& fs, Game& game, MapObjectLoader* const) {
	if (!fs.file_exists("map") || !fs.is_directory("map")) {
		throw GameDataError("no map");
	}

	//  Now Load the map as it would be a normal map saving.
	delete wml_;

	wml_ = new WidelandsMapLoader(fs.make_sub_file_system("map"), game.mutable_map());

	wml_->preload_map(
	   true, nullptr /* add-ons should already have been loaded by GamePreloadPacket */);

	//  DONE, mapfs gets deleted by WidelandsMapLoader.
}

void GameMapPacket::read_complete(Game& game) {
	wml_->load_map_complete(game, MapLoader::LoadType::kScenario);
	mol_ = wml_->get_map_object_loader();
}

void GameMapPacket::write(FileSystem& fs, Game& game, MapObjectSaver* const) {

	std::unique_ptr<FileSystem> mapfs(fs.create_sub_file_system("map", FileSystem::DIR));

	//  Now Write the map as it would be a normal map saving.
	delete wms_;
	wms_ = new MapSaver(*mapfs, game);
	wms_->save();
	mos_ = wms_->get_map_object_saver();
}
}  // namespace Widelands
