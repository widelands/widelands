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

#include "commands/cmd_enhance_building.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_EnhanceBuilding ***/

CmdEnhanceBuilding::CmdEnhanceBuilding(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	bi_ = des.unsigned_16();
	keep_wares_ = (des.unsigned_8() != 0u);
}

void CmdEnhanceBuilding::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial_);
	if (upcast(ConstructionSite, cs, mo)) {
		if (bi_ == cs->building().enhancement()) {
			cs->enhance(game);
		}
	} else if (upcast(Building, building, mo)) {
		game.get_player(sender())->enhance_building(building, bi_, keep_wares_);
	}
}

void CmdEnhanceBuilding::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_16(bi_);
	ser.unsigned_8(keep_wares_ ? 1 : 0);
}

constexpr uint16_t kCurrentPacketVersionCmdEnhanceBuilding = 2;

void CmdEnhanceBuilding::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdEnhanceBuilding) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			bi_ = fr.unsigned_16();
			keep_wares_ = (fr.unsigned_8() != 0u);
		} else {
			throw UnhandledVersionError(
			   "CmdEnhanceBuilding", packet_version, kCurrentPacketVersionCmdEnhanceBuilding);
		}
	} catch (const WException& e) {
		throw GameDataError("enhance building: %s", e.what());
	}
}
void CmdEnhanceBuilding::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionCmdEnhanceBuilding);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_16(bi_);
	fw.unsigned_8(keep_wares_ ? 1 : 0);
}

}  // namespace Widelands
