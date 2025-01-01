/*
 * Copyright (C) 2004-2025 by the Widelands Development Team
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

#include "commands/cmd_destroy_map_object.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

CmdDestroyMapObject::CmdDestroyMapObject(const Time& t, MapObject& o)
   : GameLogicCommand(t), obj_serial(o.serial()) {
}

void CmdDestroyMapObject::execute(Game& game) {
	game.syncstream().unsigned_8(SyncEntry::kDestroyObject);
	game.syncstream().unsigned_32(obj_serial);

	if (MapObject* obj = game.objects().get_object(obj_serial)) {
		obj->destroy(game);
	}
}

constexpr uint16_t kCurrentPacketVersionDestroyMapObject = 1;

void CmdDestroyMapObject::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionDestroyMapObject) {
			GameLogicCommand::read(fr, egbase, mol);
			if (Serial const serial = fr.unsigned_32()) {
				try {
					obj_serial = mol.get<MapObject>(serial).serial();
				} catch (const WException& e) {
					throw GameDataError("%u: %s", serial, e.what());
				}
			} else {
				obj_serial = 0;
			}
		} else {
			throw UnhandledVersionError(
			   "CmdDestroyMapObject", packet_version, kCurrentPacketVersionDestroyMapObject);
		}
	} catch (const WException& e) {
		throw GameDataError("destroy map object: %s", e.what());
	}
}
void CmdDestroyMapObject::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionDestroyMapObject);

	// Write base classes
	GameLogicCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(obj_serial)));
}

}  // namespace Widelands
