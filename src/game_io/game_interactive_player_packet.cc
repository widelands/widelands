/*
 * Copyright (C) 2002-2026 by the Widelands Development Team
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

#include "game_io/game_interactive_player_packet.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

namespace {

/* Changelog:
 * 6: v1.1
 * 7: Changed landmark handling
 * 8: Naval Warfare: Removed expedition port spaces
 */
constexpr uint16_t kCurrentPacketVersion = 8;

}  // namespace

void GameInteractivePlayerPacket::read(FileSystem& fs, Game& game, MapObjectLoader* mol) {
	try {
		FileRead fr;
		fr.open(fs, "binary/interactive_player");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			PlayerNumber player_number = fr.unsigned_8();
			if (player_number < 1 || player_number > game.map().get_nrplayers()) {
				throw GameDataError("Invalid player number: %i.", player_number);
			}

			if (game.get_player(player_number) == nullptr) {
				// This happens if the player, that saved the game, was a spectator
				// and the slot for player 1 was not used in the game.
				// So now we try to create an InteractivePlayer object for another
				// player instead.
				const PlayerNumber max = game.map().get_nrplayers();
				for (player_number = 1; player_number <= max; ++player_number) {
					if (game.get_player(player_number) != nullptr) {
						break;
					}
				}
				if (player_number > max) {
					throw GameDataError("The game has no players!");
				}
			}

			IGameInterface* iginterface = game.get_game_interface();
			IGameInterface::SaveloadingInformation info;
			info.player_number = player_number;

			info.mapview_center.x = fr.float_32();
			info.mapview_center.y = fr.float_32();
			info.display_flags = fr.unsigned_32();

			info.landmarks.resize(fr.unsigned_32());
			for (auto& lm : info.landmarks) {
				lm.set = fr.unsigned_8() != 0;
				lm.view_x = fr.float_32();
				lm.view_y = fr.float_32();
				lm.zoom = fr.float_32();
				lm.name = fr.string();
			}

			if (iginterface != nullptr) {
				iginterface->restore_from_saveloading_information(info);

				if (fr.unsigned_8() != 0u && info.should_saveload_windows) {
					iginterface->load_windows(fr, *mol);
				}
			}
		} else {
			throw UnhandledVersionError(
			   "GameInteractivePlayerPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("interactive player: %s", e.what());
	}
}

/*
 * Write Function
 */
void GameInteractivePlayerPacket::write(FileSystem& fs, Game& game, MapObjectSaver* const mos) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	IGameInterface* iginterface = game.get_game_interface();
	IGameInterface::SaveloadingInformation info;
	if (iginterface != nullptr) {
		iginterface->gather_saveloading_information(info);
	}

	// Player number
	fw.unsigned_8(info.player_number);

	fw.float_32(info.mapview_center.x);
	fw.float_32(info.mapview_center.y);

	// Display flags
	fw.unsigned_32(info.display_flags);

	// Map landmarks
	fw.unsigned_32(info.landmarks.size());
	for (const auto& lm : info.landmarks) {
		fw.unsigned_8(lm.set ? 1 : 0);
		fw.float_32(lm.view_x);
		fw.float_32(lm.view_y);
		fw.float_32(lm.zoom);
		fw.string(lm.name);
	}

	if (info.should_saveload_windows) {
		fw.unsigned_8(1);
		iginterface->save_windows(fw, *mos);
	} else {
		fw.unsigned_8(0);
	}

	/*
	 * Important:
	 * The number of bytes written by `iplayer->save_windows` can not be
	 * determined in advance, so any loading code that does not wish to
	 * load the windows will not be able to access any data saved beyond
	 * this call. Therefore, do not save any further data after this
	 * function call which is not intended to be read exclusively by
	 * loading callers that also read the window data.
	 */

	fw.write(fs, "binary/interactive_player");
}
}  // namespace Widelands
