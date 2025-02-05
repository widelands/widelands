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

#include "commands/cmd_set_input_max_fill.h"

#include "economy/expedition_bootstrap.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** class CmdSetInputMaxFill ***/
CmdSetInputMaxFill::CmdSetInputMaxFill(const Time& init_duetime,
                                       const PlayerNumber init_sender,
                                       PlayerImmovable& imm,
                                       const DescriptionIndex index,
                                       const WareWorker type,
                                       const uint32_t max_fill,
                                       bool cs_setting)
   : PlayerCommand(init_duetime, init_sender),
     serial_(imm.serial()),
     index_(index),
     type_(type),
     max_fill_(max_fill),
     is_constructionsite_setting_(cs_setting) {
}

void CmdSetInputMaxFill::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial_);
	if (is_constructionsite_setting_) {
		if (upcast(ConstructionSite, cs, mo)) {
			if (upcast(ProductionsiteSettings, s, cs->get_settings())) {
				switch (type_) {
				case wwWARE:
					for (auto& pair : s->ware_queues) {
						if (pair.first == index_) {
							assert(pair.second.max_fill >= max_fill_);
							pair.second.desired_fill = max_fill_;
							return;
						}
					}
					NEVER_HERE();
				case wwWORKER:
					for (auto& pair : s->worker_queues) {
						if (pair.first == index_) {
							assert(pair.second.max_fill >= max_fill_);
							pair.second.desired_fill = max_fill_;
							return;
						}
					}
					NEVER_HERE();
				default:
					NEVER_HERE();
				}
			}
		}
	} else if (upcast(Building, b, mo)) {
		if (b->owner().player_number() == sender()) {
			b->inputqueue(index_, type_, nullptr).set_max_fill(max_fill_);
			if (upcast(Warehouse, wh, b)) {
				if (PortDock* p = wh->get_portdock()) {
					// Update in case the expedition was ready previously and now lacks a ware again
					p->expedition_bootstrap()->check_is_ready(game);
				}
			}
		}
	}
}

constexpr uint16_t kCurrentPacketVersionCmdSetInputMaxFill = 3;

void CmdSetInputMaxFill::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionCmdSetInputMaxFill);

	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.signed_32(index_);
	fw.unsigned_8(type_ == wwWARE ? 0 : 1);
	fw.unsigned_32(max_fill_);
	fw.unsigned_8(is_constructionsite_setting_ ? 1 : 0);
}

void CmdSetInputMaxFill::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdSetInputMaxFill) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			index_ = fr.signed_32();
			if (fr.unsigned_8() == 0) {
				type_ = wwWARE;
			} else {
				type_ = wwWORKER;
			}
			max_fill_ = fr.unsigned_32();
			is_constructionsite_setting_ = (fr.unsigned_8() != 0u);
		} else {
			throw UnhandledVersionError(
			   "CmdSetInputMaxFill", packet_version, kCurrentPacketVersionCmdSetInputMaxFill);
		}
	} catch (const WException& e) {
		throw GameDataError("set ware max fill: %s", e.what());
	}
}

CmdSetInputMaxFill::CmdSetInputMaxFill(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	index_ = des.signed_32();
	if (des.unsigned_8() == 0) {
		type_ = wwWARE;
	} else {
		type_ = wwWORKER;
	}
	max_fill_ = des.unsigned_32();
	is_constructionsite_setting_ = (des.unsigned_8() != 0u);
}

void CmdSetInputMaxFill::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.signed_32(index_);
	ser.unsigned_8(type_ == wwWARE ? 0 : 1);
	ser.unsigned_32(max_fill_);
	ser.unsigned_8(is_constructionsite_setting_ ? 1 : 0);
}

}  // namespace Widelands
