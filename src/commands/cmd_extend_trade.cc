/*
 * Copyright (C) 2025 by the Widelands Development Team
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

#include "commands/cmd_extend_trade.h"

#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

CmdExtendTrade::CmdExtendTrade(
   const Time& time, PlayerNumber pn, TradeID trade_id, TradeAction action, int32_t batches)
   : PlayerCommand(time, pn), trade_id_(trade_id), action_(action), batches_(batches) {
}

void CmdExtendTrade::execute(Game& game) {
	switch (action_) {
	case TradeAction::kExtend:
		game.propose_trade_extension(sender(), trade_id_, batches_);
		break;
	case TradeAction::kReject:
		game.reject_trade_extension(sender(), trade_id_, batches_);
		break;
	case TradeAction::kRetract:
		game.retract_trade_extension(sender(), trade_id_, batches_);
		break;
	case TradeAction::kAccept:
		game.accept_trade_extension(sender(), trade_id_, batches_);
		break;
	default:
		throw wexception("CmdExtendTrade: unrecognized action %u", static_cast<uint8_t>(action_));
	}
}

CmdExtendTrade::CmdExtendTrade(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	trade_id_ = des.unsigned_32();
	action_ = static_cast<TradeAction>(des.unsigned_8());
	batches_ = des.signed_32();
}

void CmdExtendTrade::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(trade_id_);
	ser.unsigned_8(static_cast<uint8_t>(action_));
	ser.signed_32(batches_);
}

constexpr uint8_t kCurrentPacketVersionCmdExtendTrade = 1;

void CmdExtendTrade::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdExtendTrade) {
			PlayerCommand::read(fr, egbase, mol);

			trade_id_ = fr.unsigned_32();
			action_ = static_cast<TradeAction>(fr.unsigned_8());
			batches_ = fr.signed_32();
		} else {
			throw UnhandledVersionError(
			   "CmdExtendTrade", packet_version, kCurrentPacketVersionCmdExtendTrade);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_ExtendTrade: %s", e.what());
	}
}

void CmdExtendTrade::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdExtendTrade);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(trade_id_);
	fw.unsigned_8(static_cast<uint8_t>(action_));
	fw.signed_32(batches_);
}

}  // namespace Widelands
