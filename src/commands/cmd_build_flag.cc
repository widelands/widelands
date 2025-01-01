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

#include "commands/cmd_build_flag.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** class Cmd_BuildFlag ***/

CmdBuildFlag::CmdBuildFlag(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()), coords(read_coords_32(&des)) {
}

void CmdBuildFlag::execute(Game& game) {
	game.get_player(sender())->build_flag(coords);
}

void CmdBuildFlag::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	write_coords_32(&ser, coords);
}

constexpr uint16_t kCurrentPacketVersionCmdBuildFlag = 1;

void CmdBuildFlag::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBuildFlag) {
			PlayerCommand::read(fr, egbase, mol);
			coords = read_coords_32(&fr, egbase.map().extent());
		} else {
			throw UnhandledVersionError(
			   "CmdBuildFlag", packet_version, kCurrentPacketVersionCmdBuildFlag);
		}
	} catch (const WException& e) {
		throw GameDataError("build flag: %s", e.what());
	}
}
void CmdBuildFlag::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBuildFlag);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	write_coords_32(&fw, coords);
}

}  // namespace Widelands
