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

#include "commands/cmd_ship_set_destination.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_ShipSetDestination ***/
CmdShipSetDestination::CmdShipSetDestination(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	destination_object_ = des.unsigned_32();
	destination_coords_ = des.unsigned_32();
}

void CmdShipSetDestination::execute(Game& game) {
	upcast(Ship, ship, game.objects().get_object(serial_));
	if (ship != nullptr && ship->get_owner()->player_number() == sender()) {
		if (destination_coords_ != 0U) {
			ship->set_destination(
			   game, ship->owner().get_detected_port_space(destination_coords_), true);
		} else {
			ship->set_destination(game, game.objects().get_object(destination_object_), true);
		}
	}
}

void CmdShipSetDestination::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_32(destination_object_);
	ser.unsigned_32(destination_coords_);
}

constexpr uint16_t kCurrentPacketVersionShipSetDestination = 2;

void CmdShipSetDestination::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		// TODO(Nordfriese): Savegame compatibility v1.1
		if (packet_version >= 1 && packet_version <= kCurrentPacketVersionShipSetDestination) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			destination_object_ = get_object_serial_or_zero<MapObject>(fr.unsigned_32(), mol);
			destination_coords_ = packet_version >= 2 ? fr.unsigned_32() : 0;
		} else {
			throw UnhandledVersionError(
			   "CmdShipSetDestination", packet_version, kCurrentPacketVersionShipSetDestination);
		}
	} catch (const WException& e) {
		throw GameDataError("Ship set destination: %s", e.what());
	}
}
void CmdShipSetDestination::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionShipSetDestination);
	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_32(
	   mos.get_object_file_index_or_zero(egbase.objects().get_object(destination_object_)));
	fw.unsigned_32(destination_coords_);
}

}  // namespace Widelands
