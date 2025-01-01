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

#include "commands/cmd_pinned_note.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/pinned_note.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

// CmdPinnedNote
void CmdPinnedNote::execute(Game& game) {
	for (Bob* b = game.map()[pos_].get_first_bob(); b != nullptr; b = b->get_next_bob()) {
		if (b->descr().type() == MapObjectType::PINNED_NOTE &&
		    b->owner().player_number() == sender()) {
			PinnedNote& pn = dynamic_cast<PinnedNote&>(*b);
			if (delete_) {
				pn.remove(game);
			} else {
				pn.set_text(text_);
				pn.set_rgb(rgb_);
			}
			return;
		}
	}

	if (!delete_) {
		PinnedNote::create(game, *game.get_player(sender()), pos_, text_, rgb_);
	}
}

CmdPinnedNote::CmdPinnedNote(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	text_ = des.string();
	pos_.x = des.unsigned_16();
	pos_.y = des.unsigned_16();
	rgb_.r = des.unsigned_8();
	rgb_.g = des.unsigned_8();
	rgb_.b = des.unsigned_8();
	delete_ = des.unsigned_8() != 0;
}

void CmdPinnedNote::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.string(text_);
	ser.unsigned_16(pos_.x);
	ser.unsigned_16(pos_.y);
	ser.unsigned_8(rgb_.r);
	ser.unsigned_8(rgb_.g);
	ser.unsigned_8(rgb_.b);
	ser.unsigned_8(delete_ ? 1 : 0);
}

constexpr uint8_t kCurrentPacketVersionCmdPinnedNote = 1;

void CmdPinnedNote::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdPinnedNote) {
			PlayerCommand::read(fr, egbase, mol);
			text_ = fr.string();
			pos_.x = fr.unsigned_16();
			pos_.y = fr.unsigned_16();
			rgb_.r = fr.unsigned_8();
			rgb_.g = fr.unsigned_8();
			rgb_.b = fr.unsigned_8();
			delete_ = fr.unsigned_8() != 0;
		} else {
			throw UnhandledVersionError(
			   "CmdPinnedNote", packet_version, kCurrentPacketVersionCmdPinnedNote);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_PinnedNote: %s", e.what());
	}
}

void CmdPinnedNote::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdPinnedNote);
	PlayerCommand::write(fw, egbase, mos);
	fw.string(text_);
	fw.unsigned_16(pos_.x);
	fw.unsigned_16(pos_.y);
	fw.unsigned_8(rgb_.r);
	fw.unsigned_8(rgb_.g);
	fw.unsigned_8(rgb_.b);
	fw.unsigned_8(delete_ ? 1 : 0);
}

}  // namespace Widelands
