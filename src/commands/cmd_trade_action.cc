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

#include "commands/cmd_trade_action.h"

#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/market.h"

namespace Widelands {

CmdTradeAction::CmdTradeAction(
   const Time& time, PlayerNumber pn, TradeID trade_id, TradeAction action, Serial accepter, Serial source)
   : PlayerCommand(time, pn), trade_id_(trade_id), action_(action), accepter_(accepter), source_(source) {
}

CmdTradeAction::CmdTradeAction() = default;

void CmdTradeAction::execute(Game& game) {
	switch (action_) {
	case TradeAction::kCancel:
		game.cancel_trade(trade_id_, false, game.get_player(sender()));
		break;

	case TradeAction::kRetract:
		game.retract_trade(trade_id_);
		break;

	case TradeAction::kReject:
		game.reject_trade(trade_id_);
		break;

	case TradeAction::kAccept:
		if (Market* market = dynamic_cast<Market*>(game.objects().get_object(accepter_));
		    market != nullptr) {
			game.accept_trade(trade_id_, *market);
		}
		break;

	case TradeAction::kPause:
	case TradeAction::kResume:
		if (Market* market = dynamic_cast<Market*>(game.objects().get_object(accepter_));
		    market != nullptr) {
			market->set_paused(game, trade_id_, action_ == TradeAction::kPause);
		}
		break;

	case TradeAction::kMove:
		if (Market* new_market = dynamic_cast<Market*>(game.objects().get_object(accepter_)); new_market != nullptr) {
			if (Market* old_market = dynamic_cast<Market*>(game.objects().get_object(source_)); old_market != nullptr) {
				game.move_trade(trade_id_, *old_market, *new_market);
			}
		}
		break;

	default:
		throw wexception("CmdTradeAction: unrecognized action %u", static_cast<uint8_t>(action_));
	}
}

CmdTradeAction::CmdTradeAction(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	trade_id_ = des.unsigned_32();
	action_ = static_cast<TradeAction>(des.unsigned_8());
	accepter_ = des.unsigned_32();
	source_ = des.unsigned_32();
}

void CmdTradeAction::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(trade_id_);
	ser.unsigned_8(static_cast<uint8_t>(action_));
	ser.unsigned_32(accepter_);
	ser.unsigned_32(source_);
}

/* Changelog:
 * 1 (pre-v1.3): Initial version
 * 2 (v1.3): Added `source` parameter
 */
constexpr uint8_t kCurrentPacketVersionCmdTradeAction = 2;

void CmdTradeAction::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version >= 1 && packet_version <= kCurrentPacketVersionCmdTradeAction) {
			PlayerCommand::read(fr, egbase, mol);
			trade_id_ = fr.unsigned_32();
			action_ = static_cast<TradeAction>(fr.unsigned_8());
			accepter_ = fr.unsigned_32();
			// TODO(Nordfriese): Savegame compatibility v1.2
			source_ = packet_version >= 2 ? fr.unsigned_32() : 0;
		} else {
			throw UnhandledVersionError(
			   "CmdTradeAction", packet_version, kCurrentPacketVersionCmdTradeAction);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_TradeAction: %s", e.what());
	}
}

void CmdTradeAction::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdTradeAction);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(trade_id_);
	fw.unsigned_8(static_cast<uint8_t>(action_));
	fw.unsigned_32(accepter_);
	fw.unsigned_32(source_);
}

}  // namespace Widelands
