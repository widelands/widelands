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

#include "commands/cmd_toggle_infinite_production.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_ToggleInfiniteProduction ***/

CmdToggleInfiniteProduction::CmdToggleInfiniteProduction(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
}

void CmdToggleInfiniteProduction::execute(Game& game) {
	if (upcast(ProductionSite, ps, game.objects().get_object(serial_))) {
		ps->set_infinite_production(!ps->infinite_production());
	}
}

void CmdToggleInfiniteProduction::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
}

constexpr uint16_t kCurrentPacketVersionCmdToggleInfiniteProduction = 1;

void CmdToggleInfiniteProduction::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdToggleInfiniteProduction) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdToggleInfiniteProduction", packet_version,
			                            kCurrentPacketVersionCmdToggleInfiniteProduction);
		}
	} catch (const WException& e) {
		throw GameDataError("toggle infinite production: %s", e.what());
	}
}
void CmdToggleInfiniteProduction::write(FileWrite& fw,
                                        EditorGameBase& egbase,
                                        MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionCmdToggleInfiniteProduction);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
}

}  // namespace Widelands
