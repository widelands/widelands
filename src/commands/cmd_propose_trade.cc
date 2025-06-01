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

#include "commands/cmd_propose_trade.h"

#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

static void serialize_bill_of_materials(const BillOfMaterials& bill, StreamWrite* ser) {
	ser->unsigned_32(bill.size());
	for (const WareAmount& amount : bill) {
		ser->unsigned_32(amount.first);
		ser->unsigned_32(amount.second);
	}
}

static BillOfMaterials deserialize_bill_of_materials(StreamRead* des) {
	BillOfMaterials bill;
	const int count = des->unsigned_32();
	for (int i = 0; i < count; ++i) {
		const auto index = des->unsigned_32();
		const auto amount = des->unsigned_32();
		bill.emplace_back(index, amount);
	}
	return bill;
}

CmdProposeTrade::CmdProposeTrade(const Time& time, PlayerNumber pn, const TradeInstance& trade)
   : PlayerCommand(time, pn), trade_(trade), initiator_(trade.initiator.serial()) {
}

CmdProposeTrade::CmdProposeTrade() = default;

void CmdProposeTrade::execute(Game& game) {
	Player* plr = game.get_player(sender());
	if (plr == nullptr) {
		return;
	}

	Market* initiator = dynamic_cast<Market*>(game.objects().get_object(initiator_));
	if (initiator == nullptr) {
		return;
	}
	if (&initiator->owner() != plr) {
		log_warn_time(game.get_gametime(), "CmdProposeTrade: sender %u, but market owner %u\n",
		              sender(), initiator->owner().player_number());
		return;
	}
	if (sender() == trade_.receiving_player) {
		log_warn_time(
		   game.get_gametime(), "CmdProposeTrade: sender and recipient are the same %u\n", sender());
		return;
	}

	if (std::string err = trade_.check_illegal(); !err.empty()) {
		log_warn_time(game.get_gametime(), "CmdProposeTrade: malformed trade proposal: %s", err.c_str());
		return;
	}

	trade_.initiator = initiator;
	game.propose_trade(trade_);
}

CmdProposeTrade::CmdProposeTrade(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	trade_.state = static_cast<TradeInstance::State>(des.unsigned_8());
	initiator_ = des.unsigned_32();
	trade_.sending_player = des.unsigned_8();
	trade_.receiving_player = des.unsigned_8();
	trade_.items_to_send = deserialize_bill_of_materials(&des);
	trade_.items_to_receive = deserialize_bill_of_materials(&des);
	trade_.num_batches = des.signed_32();
}

void CmdProposeTrade::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_8(static_cast<uint8_t>(trade_.state));
	ser.unsigned_32(initiator_);
	ser.unsigned_8(trade_.sending_player);
	ser.unsigned_8(trade_.receiving_player);
	serialize_bill_of_materials(trade_.items_to_send, &ser);
	serialize_bill_of_materials(trade_.items_to_receive, &ser);
	ser.signed_32(trade_.num_batches);
}

constexpr uint8_t kCurrentPacketVersionCmdProposeTrade = 1;

void CmdProposeTrade::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdProposeTrade) {
			PlayerCommand::read(fr, egbase, mol);

			trade_.state = static_cast<TradeInstance::State>(fr.unsigned_8());

			Serial s = fr.unsigned_32();
			trade_.initiator = s == 0 ? nullptr : &mol.get<Market>(s);
			initiator_ = trade_.initiator.serial();

			trade_.sending_player = fr.unsigned_8();
			trade_.receiving_player = fr.unsigned_8();
			trade_.items_to_send = deserialize_bill_of_materials(&fr);
			trade_.items_to_receive = deserialize_bill_of_materials(&fr);
			trade_.num_batches = fr.signed_32();
		} else {
			throw UnhandledVersionError(
			   "CmdProposeTrade", packet_version, kCurrentPacketVersionCmdProposeTrade);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_ProposeTrade: %s", e.what());
	}
}

void CmdProposeTrade::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdProposeTrade);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_8(static_cast<uint8_t>(trade_.state));
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(initiator_)));
	fw.unsigned_8(trade_.sending_player);
	fw.unsigned_8(trade_.receiving_player);
	serialize_bill_of_materials(trade_.items_to_send, &fw);
	serialize_bill_of_materials(trade_.items_to_receive, &fw);
	fw.signed_32(trade_.num_batches);
}

}  // namespace Widelands
