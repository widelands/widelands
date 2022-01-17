/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/cmd_calculate_statistics.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"

namespace Widelands {

void CmdCalculateStatistics::execute(Game& game) {
	game.sample_statistics();
	game.enqueue_command(new CmdCalculateStatistics(game.get_gametime() + kStatisticsSampleTime));
}

constexpr uint16_t kCurrentPacketVersion = 1;

void CmdCalculateStatistics::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			GameLogicCommand::read(fr, egbase, mol);
		} else {
			throw UnhandledVersionError(
			   "CmdCalculateStatistics", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("calculate statistics function: %s", e.what());
	}
}
void CmdCalculateStatistics::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersion);
	GameLogicCommand::write(fw, egbase, mos);
}
}  // namespace Widelands
