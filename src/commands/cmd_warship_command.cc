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

#include "commands/cmd_warship_command.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_WarshipCommand ***/
CmdWarshipCommand::CmdWarshipCommand(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	cmd_ = static_cast<WarshipCommand>(des.unsigned_8());
	for (uint32_t i = des.unsigned_32(); i > 0U; --i) {
		parameters_.push_back(des.signed_32());
	}
}

void CmdWarshipCommand::execute(Game& game) {
	if (!game.naval_warfare_allowed()) {
		log_warn("Received a warship command but naval warfare is disabled, ignoring.");
		return;
	}

	upcast(Ship, ship, game.objects().get_object(serial_));
	if (ship != nullptr && ship->get_owner()->player_number() == sender()) {
		ship->warship_command(game, cmd_, parameters_);
	}
}

void CmdWarshipCommand::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_8(static_cast<uint8_t>(cmd_));
	ser.unsigned_32(parameters_.size());
	for (uint32_t p : parameters_) {
		ser.unsigned_32(p);
	}
}

constexpr uint16_t kCurrentPacketVersionWarshipCommand = 1;

void CmdWarshipCommand::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionWarshipCommand) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			cmd_ = static_cast<WarshipCommand>(fr.unsigned_8());
			for (uint32_t i = fr.unsigned_32(); i > 0U; --i) {
				parameters_.push_back(fr.signed_32());
			}
		} else {
			throw UnhandledVersionError(
			   "CmdWarshipCommand", packet_version, kCurrentPacketVersionWarshipCommand);
		}
	} catch (const WException& e) {
		throw GameDataError("Warship command: %s", e.what());
	}
}
void CmdWarshipCommand::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionWarshipCommand);
	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_8(static_cast<uint8_t>(cmd_));
	fw.unsigned_32(parameters_.size());
	for (uint32_t p : parameters_) {
		fw.unsigned_32(p);
	}
}

}  // namespace Widelands
