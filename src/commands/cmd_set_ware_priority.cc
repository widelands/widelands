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

#include "commands/cmd_set_ware_priority.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** class Cmd_SetWarePriority ***/
CmdSetWarePriority::CmdSetWarePriority(const Time& init_duetime,
                                       const PlayerNumber init_sender,
                                       PlayerImmovable& imm,
                                       const WareWorker init_type,
                                       const DescriptionIndex i,
                                       const WarePriority& init_priority,
                                       bool cs_setting)
   : PlayerCommand(init_duetime, init_sender),
     serial_(imm.serial()),
     type_(init_type),
     index_(i),
     priority_(init_priority),
     is_constructionsite_setting_(cs_setting) {
}

void CmdSetWarePriority::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial_);
	if (is_constructionsite_setting_) {
		if (upcast(ConstructionSite, cs, mo)) {
			if (upcast(ProductionsiteSettings, s, cs->get_settings())) {
				for (auto& pair : s->ware_queues) {
					if (pair.first == index_) {
						pair.second.priority = priority_;
						return;
					}
				}
				NEVER_HERE();
			}
		}
	} else if (upcast(Building, psite, mo)) {
		if (psite->owner().player_number() == sender()) {
			psite->set_priority(WareWorker(type_), index_, priority_);
		}
	}
}

constexpr uint16_t kCurrentPacketVersionCmdSetWarePriority = 2;

void CmdSetWarePriority::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionCmdSetWarePriority);

	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_8(static_cast<uint8_t>(type_));
	fw.signed_32(index_);
	priority_.write(fw);
	fw.unsigned_8(is_constructionsite_setting_ ? 1 : 0);
}

void CmdSetWarePriority::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdSetWarePriority) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			type_ = WareWorker(fr.unsigned_8());
			index_ = fr.signed_32();
			priority_ = WarePriority(fr);
			is_constructionsite_setting_ = (fr.unsigned_8() != 0u);
		} else {
			throw UnhandledVersionError(
			   "CmdSetWarePriority", packet_version, kCurrentPacketVersionCmdSetWarePriority);
		}

	} catch (const WException& e) {
		throw GameDataError("set ware priority: %s", e.what());
	}
}

CmdSetWarePriority::CmdSetWarePriority(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()),
     serial_(des.unsigned_32()),
     type_(WareWorker(des.unsigned_8())),
     index_(des.signed_32()),
     priority_(des),
     is_constructionsite_setting_(des.unsigned_8() != 0u) {
}

void CmdSetWarePriority::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_8(static_cast<uint8_t>(type_));
	ser.signed_32(index_);
	priority_.write(ser);
	ser.unsigned_8(is_constructionsite_setting_ ? 1 : 0);
}

}  // namespace Widelands
