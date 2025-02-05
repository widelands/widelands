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

#include "commands/cmd_pick_custom_starting_position.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

// CmdPickCustomStartingPosition
void CmdPickCustomStartingPosition::execute(Game& game) {
	game.get_player(sender())->do_pick_custom_starting_position(coords_);
}

CmdPickCustomStartingPosition::CmdPickCustomStartingPosition(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	coords_.x = des.unsigned_16();
	coords_.y = des.unsigned_16();
}

void CmdPickCustomStartingPosition::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_16(coords_.x);
	ser.unsigned_16(coords_.y);
}

constexpr uint8_t kCurrentPacketVersionCmdPickCustomStartingPosition = 1;

void CmdPickCustomStartingPosition::read(FileRead& fr,
                                         EditorGameBase& egbase,
                                         MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdPickCustomStartingPosition) {
			PlayerCommand::read(fr, egbase, mol);
			coords_.x = fr.unsigned_16();
			coords_.y = fr.unsigned_16();
		} else {
			throw UnhandledVersionError("CmdPickCustomStartingPosition", packet_version,
			                            kCurrentPacketVersionCmdPickCustomStartingPosition);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_PickCustomStartingPosition: %s", e.what());
	}
}

void CmdPickCustomStartingPosition::write(FileWrite& fw,
                                          EditorGameBase& egbase,
                                          MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdPickCustomStartingPosition);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_16(coords_.x);
	fw.unsigned_16(coords_.y);
}

}  // namespace Widelands
