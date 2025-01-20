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

#include "commands/cmd_fleet_targets.h"

#include "economy/ferry_fleet.h"
#include "economy/ship_fleet.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

// CmdFleetTargets
void CmdFleetTargets::execute(Game& game) {
	MapObject* mo = game.objects().get_object(interface_);
	if (mo == nullptr) {
		return;
	}

	switch (mo->descr().type()) {
	case MapObjectType::SHIP_FLEET_YARD_INTERFACE:
		dynamic_cast<ShipFleetYardInterface*>(mo)->get_fleet()->set_ships_target(game, target_);
		return;

	case MapObjectType::FERRY_FLEET_YARD_INTERFACE:
		dynamic_cast<FerryFleetYardInterface*>(mo)->get_fleet()->set_idle_ferries_target(
		   game, target_);
		return;

	default:
		throw wexception("CmdFleetTargets for object %u of type %s", interface_,
		                 to_string(mo->descr().type()).c_str());
	}
}

CmdFleetTargets::CmdFleetTargets(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	interface_ = des.unsigned_32();
	target_ = des.unsigned_32();
}

void CmdFleetTargets::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(interface_);
	ser.unsigned_32(target_);
}

constexpr uint8_t kCurrentPacketVersionCmdFleetTargets = 1;

void CmdFleetTargets::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdFleetTargets) {
			PlayerCommand::read(fr, egbase, mol);
			interface_ = fr.unsigned_32();
			target_ = fr.unsigned_32();
		} else {
			throw UnhandledVersionError(
			   "CmdFleetTargets", packet_version, kCurrentPacketVersionCmdFleetTargets);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_FleetTargets: %s", e.what());
	}
}

void CmdFleetTargets::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdFleetTargets);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(interface_)));
	fw.unsigned_32(target_);
}

}  // namespace Widelands
