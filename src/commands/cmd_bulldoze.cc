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

#include "commands/cmd_bulldoze.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** class Cmd_Bulldoze ***/

CmdBulldoze::CmdBulldoze(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()),
     serial(des.unsigned_32()),
     recurse(des.unsigned_8() != 0u) {
}

void CmdBulldoze::execute(Game& game) {
	if (upcast(PlayerImmovable, pimm, game.objects().get_object(serial))) {
		game.get_player(sender())->bulldoze(*pimm, recurse);
	}
}

void CmdBulldoze::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_8(static_cast<uint8_t>(recurse));
}

constexpr uint16_t kCurrentPacketVersionCmdBulldoze = 2;

void CmdBulldoze::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBulldoze) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PlayerImmovable>(fr.unsigned_32(), mol);
			recurse = (fr.unsigned_8() != 0);
		} else {
			throw UnhandledVersionError(
			   "CmdBulldoze", packet_version, kCurrentPacketVersionCmdBulldoze);
		}
	} catch (const WException& e) {
		throw GameDataError("bulldoze: %s", e.what());
	}
}
void CmdBulldoze::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBulldoze);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
	fw.unsigned_8(static_cast<uint8_t>(recurse));
}

}  // namespace Widelands
