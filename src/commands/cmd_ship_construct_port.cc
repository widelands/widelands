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

#include "commands/cmd_ship_construct_port.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_ShipConstructPort ***/
CmdShipConstructPort::CmdShipConstructPort(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	coords = read_coords_32(&des);
}

void CmdShipConstructPort::execute(Game& game) {
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship != nullptr && ship->get_owner()->player_number() == sender()) {
		if (ship->get_ship_state() != Widelands::ShipStates::kExpeditionPortspaceFound) {
			log_warn_time(game.get_gametime(),
			              " %1d:ship on %3dx%3d received build port command but "
			              "not in kExpeditionPortspaceFound status (expedition: %s), ignoring...\n",
			              ship->get_owner()->player_number(), ship->get_position().x,
			              ship->get_position().y, (ship->state_is_expedition()) ? "Y" : "N");
			return;
		}
		ship->exp_construct_port(game, coords);
	}
}

void CmdShipConstructPort::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	write_coords_32(&ser, coords);
}

constexpr uint16_t kCurrentPacketVersionShipConstructPort = 1;

void CmdShipConstructPort::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipConstructPort) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			// Coords
			coords = read_coords_32(&fr);
		} else {
			throw UnhandledVersionError(
			   "CmdShipConstructPort", packet_version, kCurrentPacketVersionShipConstructPort);
		}
	} catch (const WException& e) {
		throw GameDataError("Ship construct port: %s", e.what());
	}
}
void CmdShipConstructPort::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionShipConstructPort);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Coords
	write_coords_32(&fw, coords);
}

}  // namespace Widelands
