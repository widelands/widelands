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

#include "commands/cmd_ship_refit.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_ShipRefit ***/
CmdShipRefit::CmdShipRefit(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	type_ = static_cast<ShipType>(des.unsigned_8());
}

void CmdShipRefit::execute(Game& game) {
	if (!game.naval_warfare_allowed()) {
		log_warn("Received a refit command but naval warfare is disabled, ignoring.");
		return;
	}

	upcast(Ship, ship, game.objects().get_object(serial_));
	if (ship != nullptr && ship->get_owner()->player_number() == sender()) {
		ship->refit(game, type_);
	}
}

void CmdShipRefit::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_8(static_cast<uint8_t>(type_));
}

constexpr uint16_t kCurrentPacketVersionShipRefit = 1;

void CmdShipRefit::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipRefit) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			type_ = static_cast<ShipType>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError(
			   "CmdShipRefit", packet_version, kCurrentPacketVersionShipRefit);
		}
	} catch (const WException& e) {
		throw GameDataError("Ship refit: %s", e.what());
	}
}
void CmdShipRefit::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionShipRefit);
	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_8(static_cast<uint8_t>(type_));
}

}  // namespace Widelands
