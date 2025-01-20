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

#include "commands/cmd_set_stock_policy.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** struct Cmd_SetStockPolicy ***/
CmdSetStockPolicy::CmdSetStockPolicy(const Time& time,
                                     PlayerNumber p,
                                     const Building& wh,
                                     bool isworker,
                                     DescriptionIndex ware,
                                     StockPolicy policy)
   : PlayerCommand(time, p) {
	warehouse_ = wh.serial();
	isworker_ = isworker;
	ware_ = ware;
	policy_ = policy;
}

CmdSetStockPolicy::CmdSetStockPolicy() : warehouse_(0), isworker_(false), ware_(0), policy_() {
}

void CmdSetStockPolicy::execute(Game& game) {
	// Sanitize data that could have come from the network
	if (Player* plr = game.get_player(sender())) {
		MapObject* mo = game.objects().get_object(warehouse_);
		if (upcast(ConstructionSite, cs, mo)) {
			if (upcast(WarehouseSettings, s, cs->get_settings())) {
				if (isworker_) {
					s->worker_preferences[ware_] = policy_;
				} else {
					s->ware_preferences[ware_] = policy_;
				}
			}
		} else if (upcast(Warehouse, warehouse, mo)) {
			if (warehouse->get_owner() != plr) {
				log_warn_time(game.get_gametime(),
				              "Cmd_SetStockPolicy: sender %u, but warehouse owner %u\n", sender(),
				              warehouse->owner().player_number());
				return;
			}

			if (isworker_) {
				if (!(game.descriptions().worker_exists(ware_))) {
					log_warn_time(game.get_gametime(),
					              "Cmd_SetStockPolicy: sender %u, worker %u does not exist\n", sender(),
					              ware_);
					return;
				}
				warehouse->set_worker_policy(ware_, policy_);
			} else {
				if (!(game.descriptions().ware_exists(ware_))) {
					log_warn_time(game.get_gametime(),
					              "Cmd_SetStockPolicy: sender %u, ware %u does not exist\n", sender(),
					              ware_);
					return;
				}
				warehouse->set_ware_policy(ware_, policy_);
			}
		}
	}
}

CmdSetStockPolicy::CmdSetStockPolicy(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	warehouse_ = des.unsigned_32();
	isworker_ = (des.unsigned_8() != 0u);
	ware_ = DescriptionIndex(des.unsigned_8());
	policy_ = static_cast<StockPolicy>(des.unsigned_8());
}

void CmdSetStockPolicy::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(warehouse_);
	ser.unsigned_8(static_cast<uint8_t>(isworker_));
	ser.unsigned_8(ware_);
	ser.unsigned_8(static_cast<uint8_t>(policy_));
}

constexpr uint8_t kCurrentPacketVersionCmdSetStockPolicy = 1;

void CmdSetStockPolicy::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdSetStockPolicy) {
			PlayerCommand::read(fr, egbase, mol);
			warehouse_ = fr.unsigned_32();
			isworker_ = (fr.unsigned_8() != 0u);
			ware_ = DescriptionIndex(fr.unsigned_8());
			policy_ = static_cast<StockPolicy>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError(
			   "CmdSetStockPolicy", packet_version, kCurrentPacketVersionCmdSetStockPolicy);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_SetStockPolicy: %s", e.what());
	}
}

void CmdSetStockPolicy::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdSetStockPolicy);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(warehouse_);
	fw.unsigned_8(static_cast<uint8_t>(isworker_));
	fw.unsigned_8(ware_);
	fw.unsigned_8(static_cast<uint8_t>(policy_));
}

}  // namespace Widelands
