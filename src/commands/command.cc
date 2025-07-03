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

#include "commands/command.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

PlayerCommand::PlayerCommand(const Time& time, const PlayerNumber s)
   : GameLogicCommand(time), sender_(s), cmdserial_(0) {
}

void PlayerCommand::write_id_and_sender(StreamWrite& ser) {
	ser.unsigned_8(static_cast<uint8_t>(id()));
	ser.unsigned_8(sender());
}

/**
 * Write this player command to a file. Call this from base classes
 */
constexpr uint16_t kCurrentPacketVersionPlayerCommand = 3;

void PlayerCommand::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionPlayerCommand);

	GameLogicCommand::write(fw, egbase, mos);
	// Now sender
	fw.unsigned_8(sender());
	fw.unsigned_32(cmdserial());
}

void PlayerCommand::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionPlayerCommand) {
			GameLogicCommand::read(fr, egbase, mol);
			sender_ = fr.unsigned_8();
			if (egbase.get_player(sender_) == nullptr) {
				throw GameDataError("player %u does not exist", sender_);
			}
			cmdserial_ = fr.unsigned_32();
		} else {
			throw UnhandledVersionError(
			   "PlayerCommand", packet_version, kCurrentPacketVersionPlayerCommand);
		}
	} catch (const WException& e) {
		throw GameDataError("player command: %s", e.what());
	}
}

/*** struct PlayerMessageCommand ***/

PlayerMessageCommand::PlayerMessageCommand(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()), message_id_(des.unsigned_32()) {
}

constexpr uint16_t kCurrentPacketVersionPlayerMessageCommand = 1;

void PlayerMessageCommand::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionPlayerMessageCommand) {
			PlayerCommand::read(fr, egbase, mol);
			message_id_ = MessageId(fr.unsigned_32());
			if (message_id_.valid()) {
				verb_log_warn("PlayerMessageCommand (player %u): message ID is null", sender());
			}
		} else {
			throw UnhandledVersionError(
			   "PlayerMessageCommand", packet_version, kCurrentPacketVersionPlayerMessageCommand);
		}
	} catch (const WException& e) {
		throw GameDataError("player message: %s", e.what());
	}
}

void PlayerMessageCommand::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionPlayerMessageCommand);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.message_savers[sender() - 1][message_id()].value());
}

/*** class CmdChangeTargetQuantity ***/

CmdChangeTargetQuantity::CmdChangeTargetQuantity(const Time& init_duetime,
                                                 const PlayerNumber init_sender,
                                                 const uint32_t init_economy,
                                                 const DescriptionIndex init_ware_type)
   : PlayerCommand(init_duetime, init_sender), economy_(init_economy), ware_type_(init_ware_type) {
}

void CmdChangeTargetQuantity::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(economy());
	fw.c_string(egbase.player(sender()).tribe().get_ware_descr(ware_type())->name());
}

void CmdChangeTargetQuantity::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		PlayerCommand::read(fr, egbase, mol);
		economy_ = fr.unsigned_32();
		ware_type_ = egbase.player(sender()).tribe().ware_index(fr.c_string());
	} catch (const WException& e) {
		throw GameDataError("change target quantity: %s", e.what());
	}
}

CmdChangeTargetQuantity::CmdChangeTargetQuantity(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()),
     economy_(des.unsigned_32()),
     ware_type_(des.unsigned_8()) {
}

void CmdChangeTargetQuantity::serialize(StreamWrite& ser) {
	// Subclasses take care of writing their id() and sender()
	ser.unsigned_32(economy());
	ser.unsigned_8(ware_type());
}

}  // namespace Widelands
