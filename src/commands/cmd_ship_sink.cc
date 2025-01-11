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

#include "commands/cmd_ship_sink.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_ShipSink ***/
CmdShipSink::CmdShipSink(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
}

void CmdShipSink::execute(Game& game) {
	upcast(Ship, ship, game.objects().get_object(serial));
	if ((ship != nullptr) && ship->get_owner()->player_number() == sender()) {
		ship->sink_ship(game);
	}
}

void CmdShipSink::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionCmdShipSink = 1;

void CmdShipSink::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdShipSink) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError(
			   "CmdShipSink", packet_version, kCurrentPacketVersionCmdShipSink);
		}
	} catch (const WException& e) {
		throw GameDataError("Ship explore: %s", e.what());
	}
}
void CmdShipSink::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdShipSink);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

}  // namespace Widelands
