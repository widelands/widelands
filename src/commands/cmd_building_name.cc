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

#include "commands/cmd_building_name.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

// CmdBuildingName
void CmdBuildingName::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial_);
	if (mo == nullptr) {
		return;
	}

	switch (mo->descr().type()) {
	case MapObjectType::SHIP:
		dynamic_cast<Ship&>(*mo).set_shipname(name_);
		return;

	case MapObjectType::WAREHOUSE:
		dynamic_cast<Warehouse&>(*mo).set_warehouse_name(name_);
		return;

	default:
		throw wexception("CmdBuildingName for object %u of type %s", serial_,
		                 to_string(mo->descr().type()).c_str());
	}
}

CmdBuildingName::CmdBuildingName(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	name_ = des.string();
}

void CmdBuildingName::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.string(name_);
}

constexpr uint8_t kCurrentPacketVersionCmdBuildingName = 1;

void CmdBuildingName::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdBuildingName) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = fr.unsigned_32();
			name_ = fr.string();
		} else {
			throw UnhandledVersionError(
			   "CmdBuildingName", packet_version, kCurrentPacketVersionCmdBuildingName);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_BuildingName: %s", e.what());
	}
}

void CmdBuildingName::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdBuildingName);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.string(name_);
}

}  // namespace Widelands
