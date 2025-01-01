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

#include "commands/cmd_mark_map_object_for_removal.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

// CmdMarkMapObjectForRemoval
void CmdMarkMapObjectForRemoval::execute(Game& game) {
	if (upcast(Immovable, mo, game.objects().get_object(object_))) {
		mo->set_marked_for_removal(sender(), mark_);
	}
}

CmdMarkMapObjectForRemoval::CmdMarkMapObjectForRemoval(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	object_ = des.unsigned_32();
	mark_ = (des.unsigned_8() != 0u);
}

void CmdMarkMapObjectForRemoval::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(object_);
	ser.unsigned_8(mark_ ? 1 : 0);
}

constexpr uint8_t kCurrentPacketVersionCmdMarkMapObjectForRemoval = 1;

void CmdMarkMapObjectForRemoval::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdMarkMapObjectForRemoval) {
			PlayerCommand::read(fr, egbase, mol);
			object_ = fr.unsigned_32();
			mark_ = (fr.unsigned_8() != 0u);
		} else {
			throw UnhandledVersionError("CmdMarkMapObjectForRemoval", packet_version,
			                            kCurrentPacketVersionCmdMarkMapObjectForRemoval);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_MarkMapObjectForRemoval: %s", e.what());
	}
}

void CmdMarkMapObjectForRemoval::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdMarkMapObjectForRemoval);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(object_)));
	fw.unsigned_8(static_cast<uint8_t>(mark_));
}

}  // namespace Widelands
