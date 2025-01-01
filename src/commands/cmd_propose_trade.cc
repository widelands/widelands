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

#include "logic/game.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/player.h"

namespace Widelands {

static void serialize_bill_of_materials(const BillOfMaterials& bill, StreamWrite* ser) {
	ser->unsigned_32(bill.size());
	for (const WareAmount& amount : bill) {
		ser->unsigned_8(amount.first);
		ser->unsigned_32(amount.second);
	}
}

static BillOfMaterials deserialize_bill_of_materials(StreamRead* des) {
	BillOfMaterials bill;
	const int count = des->unsigned_32();
	for (int i = 0; i < count; ++i) {
		const auto index = des->unsigned_8();
		const auto amount = des->unsigned_32();
		bill.emplace_back(index, amount);
	}
	return bill;
}

CmdProposeTrade::CmdProposeTrade(const Time& time, PlayerNumber pn, const Trade& trade)
   : PlayerCommand(time, pn), trade_(trade) {
}

CmdProposeTrade::CmdProposeTrade() = default;

void CmdProposeTrade::execute(Game& game) {
	Player* plr = game.get_player(sender());
	if (plr == nullptr) {
		return;
	}

	Market* initiator = dynamic_cast<Market*>(game.objects().get_object(trade_.initiator));
	if (initiator == nullptr) {
		log_warn_time(
		   game.get_gametime(), "CmdProposeTrade: initiator vanished or is not a market.\n");
		return;
	}
	if (&initiator->owner() != plr) {
		log_warn_time(game.get_gametime(), "CmdProposeTrade: sender %u, but market owner %u\n",
		              sender(), initiator->owner().player_number());
		return;
	}
	Market* receiver = dynamic_cast<Market*>(game.objects().get_object(trade_.receiver));
	if (receiver == nullptr) {
		log_warn_time(
		   game.get_gametime(), "CmdProposeTrade: receiver vanished or is not a market.\n");
		return;
	}
	if (initiator->get_owner() == receiver->get_owner()) {
		log_warn_time(
		   game.get_gametime(), "CmdProposeTrade: Sending and receiving player are the same.\n");
		return;
	}

	// TODO(sirver,trading): Maybe check connectivity between markets here and
	// report errors.
	game.propose_trade(trade_);
}

CmdProposeTrade::CmdProposeTrade(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	trade_.initiator = des.unsigned_32();
	trade_.receiver = des.unsigned_32();
	trade_.items_to_send = deserialize_bill_of_materials(&des);
	trade_.items_to_receive = deserialize_bill_of_materials(&des);
	trade_.num_batches = des.signed_32();
}

void CmdProposeTrade::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(trade_.initiator);
	ser.unsigned_32(trade_.receiver);
	serialize_bill_of_materials(trade_.items_to_send, &ser);
	serialize_bill_of_materials(trade_.items_to_receive, &ser);
	ser.signed_32(trade_.num_batches);
}

void CmdProposeTrade::read(FileRead& /* fr */,
                           EditorGameBase& /* egbase */,
                           MapObjectLoader& /* mol */) {
	// TODO(sirver,trading): Implement this.
	NEVER_HERE();
}

void CmdProposeTrade::write(FileWrite& /* fw */,
                            EditorGameBase& /* egbase */,
                            MapObjectSaver& /* mos */) {
	// TODO(sirver,trading): Implement this.
	NEVER_HERE();
}

}  // namespace Widelands
