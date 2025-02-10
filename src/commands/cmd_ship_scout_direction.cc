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

#include "commands/cmd_ship_scout_direction.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_ShipScoutDirection ***/
CmdShipScoutDirection::CmdShipScoutDirection(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	dir = static_cast<WalkingDir>(des.unsigned_8());
}

void CmdShipScoutDirection::execute(Game& game) {
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship != nullptr && ship->get_owner()->player_number() == sender()) {
		if (ship->get_ship_state() != Widelands::ShipStates::kExpeditionWaiting &&
		    ship->get_ship_state() != Widelands::ShipStates::kExpeditionPortspaceFound &&
		    ship->get_ship_state() != Widelands::ShipStates::kExpeditionScouting) {
			log_warn_time(
			   game.get_gametime(),
			   " %1d:ship on %3dx%3d received scout command but not in "
			   "kExpeditionWaiting or kExpeditionPortspaceFound or kExpeditionScouting status "
			   "(expedition: %s), ignoring...\n",
			   ship->get_owner()->player_number(), ship->get_position().x, ship->get_position().y,
			   (ship->state_is_expedition()) ? "Y" : "N");
			return;
		}
		ship->exp_scouting_direction(game, dir);
	}
}

void CmdShipScoutDirection::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_8(static_cast<uint8_t>(dir));
}

constexpr uint16_t kCurrentPacketVersionShipScoutDirection = 1;

void CmdShipScoutDirection::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipScoutDirection) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			// direction
			dir = static_cast<WalkingDir>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError(
			   "CmdShipScoutDirection", packet_version, kCurrentPacketVersionShipScoutDirection);
		}
	} catch (const WException& e) {
		throw GameDataError("Ship scout: %s", e.what());
	}
}
void CmdShipScoutDirection::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionShipScoutDirection);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// direction
	fw.unsigned_8(static_cast<uint8_t>(dir));
}

}  // namespace Widelands
