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

#include "commands/cmd_change_soldier_capacity.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_ChangeSoldierCapacity ***/

CmdChangeSoldierCapacity::CmdChangeSoldierCapacity(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	val = des.signed_16();
}

void CmdChangeSoldierCapacity::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial);
	if (upcast(ConstructionSite, cs, mo)) {
		// Clamp the capacity between the minimum and maximum value because the player may
		// have sent multiple decrease/increase commands at the same time (bug #5006).
		if (upcast(MilitarysiteSettings, ms, cs->get_settings())) {
			ms->desired_capacity = std::max(1, std::min<int32_t>(ms->max_capacity, val));
		} else if (upcast(TrainingsiteSettings, ts, cs->get_settings())) {
			ts->desired_capacity = std::max(0, std::min<int32_t>(ts->max_capacity, val));
		} else if (upcast(WarehouseSettings, ws, cs->get_settings())) {
			ws->desired_capacity = std::max(0, std::min<int32_t>(ws->max_garrison, val));
		}
	} else if (upcast(Building, building, mo)) {
		if (building->get_owner() == game.get_player(sender()) &&
		    building->soldier_control() != nullptr) {
			SoldierControl* soldier_control = building->mutable_soldier_control();
			Widelands::Quantity const old_capacity = soldier_control->soldier_capacity();
			Widelands::Quantity const new_capacity =
			   std::min(static_cast<Widelands::Quantity>(
			               std::max(static_cast<int32_t>(old_capacity) + val,
			                        static_cast<int32_t>(soldier_control->min_soldier_capacity()))),
			            soldier_control->max_soldier_capacity());
			if (old_capacity != new_capacity) {
				soldier_control->set_soldier_capacity(new_capacity);
			}
		}
	}
}

void CmdChangeSoldierCapacity::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.signed_16(val);
}

constexpr uint16_t kCurrentPacketVersionChangeSoldierCapacity = 1;

void CmdChangeSoldierCapacity::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionChangeSoldierCapacity) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			val = fr.signed_16();
		} else {
			throw UnhandledVersionError(
			   "CmdChangeSoldierCapacity", packet_version, kCurrentPacketVersionChangeSoldierCapacity);
		}
	} catch (const WException& e) {
		throw GameDataError("change soldier capacity: %s", e.what());
	}
}

void CmdChangeSoldierCapacity::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionChangeSoldierCapacity);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Now capacity
	fw.signed_16(val);
}

}  // namespace Widelands
