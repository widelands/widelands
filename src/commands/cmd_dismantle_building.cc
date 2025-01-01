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

#include "commands/cmd_dismantle_building.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_DismantleBuilding ***/
CmdDismantleBuilding::CmdDismantleBuilding(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	keep_wares_ = (des.unsigned_8() != 0u);
}

void CmdDismantleBuilding::execute(Game& game) {
	if (upcast(Building, building, game.objects().get_object(serial_))) {
		game.get_player(sender())->dismantle_building(building, keep_wares_);
	}
}

void CmdDismantleBuilding::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_8(keep_wares_ ? 1 : 0);
}

constexpr uint16_t kCurrentPacketVersionDismantleBuilding = 2;

void CmdDismantleBuilding::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionDismantleBuilding) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			keep_wares_ = (fr.unsigned_8() != 0u);
		} else {
			throw UnhandledVersionError(
			   "CmdDismantleBuilding", packet_version, kCurrentPacketVersionDismantleBuilding);
		}
	} catch (const WException& e) {
		throw GameDataError("dismantle building: %s", e.what());
	}
}
void CmdDismantleBuilding::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionDismantleBuilding);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_8(keep_wares_ ? 1 : 0);
}

}  // namespace Widelands
