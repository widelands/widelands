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

#include "commands/cmd_start_stop_building.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_StartStopBuilding ***/

CmdStartStopBuilding::CmdStartStopBuilding(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
}

void CmdStartStopBuilding::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial);
	if (upcast(ConstructionSite, cs, mo)) {
		if (upcast(ProductionsiteSettings, s, cs->get_settings())) {
			s->stopped = !s->stopped;
		}
	} else if (upcast(Building, building, mo)) {
		game.get_player(sender())->start_stop_building(*building);
	}
}

void CmdStartStopBuilding::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionCmdStartStopBuilding = 1;

void CmdStartStopBuilding::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdStartStopBuilding) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError(
			   "CmdStartStopBuilding", packet_version, kCurrentPacketVersionCmdStartStopBuilding);
		}
	} catch (const WException& e) {
		throw GameDataError("start/stop building: %s", e.what());
	}
}
void CmdStartStopBuilding::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdStartStopBuilding);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

}  // namespace Widelands
