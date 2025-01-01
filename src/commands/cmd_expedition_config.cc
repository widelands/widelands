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

#include "commands/cmd_expedition_config.h"

#include "economy/expedition_bootstrap.h"
#include "economy/portdock.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_ExpeditionConfig ***/

CmdExpeditionConfig::CmdExpeditionConfig(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	type = des.unsigned_8() == 0 ? wwWARE : wwWORKER;
	index = des.unsigned_32();
	add = (des.unsigned_8() != 0u);
}

void CmdExpeditionConfig::execute(Game& game) {
	if (upcast(PortDock, pd, game.objects().get_object(serial))) {
		if (ExpeditionBootstrap* x = pd->expedition_bootstrap()) {
			x->demand_additional_item(game, type, index, add);
		}
	}
}

void CmdExpeditionConfig::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_8(type == wwWARE ? 0 : 1);
	ser.unsigned_32(index);
	ser.unsigned_8(add ? 1 : 0);
}

constexpr uint16_t kCurrentPacketVersionCmdExpeditionConfig = 1;

void CmdExpeditionConfig::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdExpeditionConfig) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PortDock>(fr.unsigned_32(), mol);
			type = fr.unsigned_8() == 0 ? wwWARE : wwWORKER;
			index = fr.unsigned_32();
			add = (fr.unsigned_8() != 0u);
		} else {
			throw UnhandledVersionError(
			   "CmdExpeditionConfig", packet_version, kCurrentPacketVersionCmdExpeditionConfig);
		}
	} catch (const WException& e) {
		throw GameDataError("enhance building: %s", e.what());
	}
}
void CmdExpeditionConfig::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionCmdExpeditionConfig);
	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
	fw.unsigned_8(type == wwWARE ? 0 : 1);
	fw.unsigned_32(index);
	fw.unsigned_8(add ? 1 : 0);
}

}  // namespace Widelands
