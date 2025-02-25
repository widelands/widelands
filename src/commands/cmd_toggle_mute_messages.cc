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

#include "commands/cmd_toggle_mute_messages.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

// CmdToggleMuteMessages
void CmdToggleMuteMessages::execute(Game& game) {
	if (upcast(Building, b, game.objects().get_object(building_))) {
		if (all_) {
			const DescriptionIndex di = game.descriptions().safe_building_index(b->descr().name());
			b->get_owner()->set_muted(di, !b->owner().is_muted(di));
		} else {
			b->set_mute_messages(!b->mute_messages());
		}
	}
}

CmdToggleMuteMessages::CmdToggleMuteMessages(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	building_ = des.unsigned_32();
	all_ = (des.unsigned_8() != 0u);
}

void CmdToggleMuteMessages::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(building_);
	ser.unsigned_8(all_ ? 1 : 0);
}

constexpr uint8_t kCurrentPacketVersionCmdToggleMuteMessages = 1;

void CmdToggleMuteMessages::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdToggleMuteMessages) {
			PlayerCommand::read(fr, egbase, mol);
			building_ = fr.unsigned_32();
			all_ = (fr.unsigned_8() != 0u);
		} else {
			throw UnhandledVersionError(
			   "CmdToggleMuteMessages", packet_version, kCurrentPacketVersionCmdToggleMuteMessages);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_ToggleMuteMessages: %s", e.what());
	}
}

void CmdToggleMuteMessages::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdToggleMuteMessages);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(building_)));
	fw.unsigned_8(static_cast<uint8_t>(all_));
}

}  // namespace Widelands
