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

#include "commands/cmd_flag_action.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_FlagAction ***/
CmdFlagAction::CmdFlagAction(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	type_ = static_cast<FlagJob::Type>(des.unsigned_8());
	serial_ = des.unsigned_32();
}

void CmdFlagAction::execute(Game& game) {
	Player* player = game.get_player(sender());
	if (upcast(Flag, flag, game.objects().get_object(serial_))) {
		if (flag->get_owner() == player) {
			player->flagaction(*flag, type_);
		}
	}
}

void CmdFlagAction::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_8(static_cast<uint8_t>(type_));
	ser.unsigned_32(serial_);
}

constexpr uint16_t kCurrentPacketVersionCmdFlagAction = 3;  // since v1.0

void CmdFlagAction::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version >= 3 && packet_version <= kCurrentPacketVersionCmdFlagAction) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Flag>(fr.unsigned_32(), mol);
			type_ = static_cast<FlagJob::Type>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError(
			   "CmdFlagAction", packet_version, kCurrentPacketVersionCmdFlagAction);
		}
	} catch (const WException& e) {
		throw GameDataError("flag action: %s", e.what());
	}
}
void CmdFlagAction::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdFlagAction);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_8(static_cast<uint8_t>(type_));
}

}  // namespace Widelands
