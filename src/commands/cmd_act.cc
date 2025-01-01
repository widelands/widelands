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

#include "commands/cmd_act.h"

#include "economy/ferry_fleet.h"
#include "economy/ship_fleet.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

CmdAct::CmdAct(const Time& t, MapObject& o, int32_t const a)
   : GameLogicCommand(t), obj_serial(o.serial()), arg(a) {
}

void CmdAct::execute(Game& game) {
	game.syncstream().unsigned_8(SyncEntry::kCmdAct);
	game.syncstream().unsigned_32(obj_serial);

	if (MapObject* const obj = game.objects().get_object(obj_serial)) {
		game.syncstream().unsigned_8(static_cast<uint8_t>(obj->descr().type()));
		obj->act(game, arg);
	} else {
		game.syncstream().unsigned_8(static_cast<uint8_t>(MapObjectType::MAPOBJECT));
	}
	// the object must queue the next CMD_ACT itself if necessary
}

constexpr uint16_t kCurrentPacketVersionCmdAct = 1;

void CmdAct::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdAct) {
			GameLogicCommand::read(fr, egbase, mol);
			if (Serial const object_serial = fr.unsigned_32()) {
				try {
					obj_serial = mol.get<MapObject>(object_serial).serial();
				} catch (const WException& e) {
					throw GameDataError("object %u: %s", object_serial, e.what());
				}
			} else {
				obj_serial = 0;
			}
			arg = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("CmdAct", packet_version, kCurrentPacketVersionCmdAct);
		}
	} catch (const WException& e) {
		throw wexception("act: %s", e.what());
	}
}
void CmdAct::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdAct);

	// Write base classes
	GameLogicCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(obj_serial)));

	// And arg
	fw.unsigned_32(arg);
}

}  // namespace Widelands
