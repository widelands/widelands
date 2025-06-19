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

#include "commands/cmd_set_worker_target_quantity.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

CmdSetWorkerTargetQuantity::CmdSetWorkerTargetQuantity(const Time& init_duetime,
                                                       const PlayerNumber init_sender,
                                                       const uint32_t init_economy,
                                                       const DescriptionIndex init_ware_type,
                                                       const uint32_t init_permanent)
   : CmdChangeTargetQuantity(init_duetime, init_sender, init_economy, init_ware_type),
     permanent_(init_permanent) {
}

void CmdSetWorkerTargetQuantity::execute(Game& game) {
	Player* player = game.get_player(sender());
	if (player->has_economy(economy()) && game.descriptions().worker_exists(ware_type())) {
		player->get_economy(economy())->set_target_quantity(
		   wwWORKER, ware_type(), permanent_, duetime());
	}
}

constexpr uint16_t kCurrentPacketVersionSetWorkerTargetQuantity = 2;

void CmdSetWorkerTargetQuantity::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionSetWorkerTargetQuantity);
	CmdChangeTargetQuantity::write(fw, egbase, mos);
	fw.unsigned_32(permanent_);
}

void CmdSetWorkerTargetQuantity::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionSetWorkerTargetQuantity) {
			CmdChangeTargetQuantity::read(fr, egbase, mol);
			permanent_ = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("CmdSetWorkerTargetQuantity", packet_version,
			                            kCurrentPacketVersionSetWorkerTargetQuantity);
		}
	} catch (const WException& e) {
		throw GameDataError("set worker target quantity: %s", e.what());
	}
}

CmdSetWorkerTargetQuantity::CmdSetWorkerTargetQuantity(StreamRead& des)
   : CmdChangeTargetQuantity(des), permanent_(des.unsigned_32()) {
	if (cmdserial() == 1) {
		des.unsigned_32();
	}
}

void CmdSetWorkerTargetQuantity::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	CmdChangeTargetQuantity::serialize(ser);
	ser.unsigned_32(permanent_);
}

}  // namespace Widelands
