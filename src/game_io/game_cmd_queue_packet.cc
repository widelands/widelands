/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "game_io/game_cmd_queue_packet.h"

#include "base/macros.h"
#include "base/multithreading.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/queue_cmd_factory.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 2;

void GameCmdQueuePacket::read(FileSystem& fs, Game& game, MapObjectLoader* const ol) {
	try {
		FileRead fr;
		fr.open(fs, "binary/cmd_queue");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			CmdQueue& cmdq = game.cmdqueue();

			// nothing to be done for game_

			// Next serial
			cmdq.nextserial_ = fr.unsigned_32();

			// Erase all currently pending commands in the queue
			cmdq.flush();

			for (;;) {
				uint32_t const packet_id = fr.unsigned_16();

				if (!packet_id) {
					break;
				}

				CmdQueue::CmdItem item;
				item.category = fr.signed_32();
				item.serial = fr.unsigned_32();

				GameLogicCommand& cmd = QueueCmdFactory::create_correct_queue_command(
				   static_cast<QueueCommandTypes>(packet_id));
				cmd.read(fr, game, *ol);

				item.cmd = &cmd;

				cmdq.cmds_[cmd.duetime().get() % kCommandQueueBucketSize].push(item);
				++cmdq.ncmds_;
			}
		} else {
			throw UnhandledVersionError("GameCmdQueuePacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("command queue: %s", e.what());
	}
}

void GameCmdQueuePacket::write(FileSystem& fs, Game& game, MapObjectSaver* const os) {
	// If the player would send a command while we're saving the queue,
	// this function would get trapped in an endless loop.
	// So all new commands are put on hold until we're done here.
	MutexLock m(MutexLock::ID::kCommands);

	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	const CmdQueue& cmdq = game.cmdqueue();

	// nothing to be done for game_

	// Next serial
	fw.unsigned_32(cmdq.nextserial_);

	// Write all commands

	// Find all the items in the current cmdqueue
	Time time = game.get_gametime();
	size_t nhandled = 0;

	while (nhandled < cmdq.ncmds_) {
		// Make a copy, so we can pop stuff
		std::priority_queue<CmdQueue::CmdItem> p = cmdq.cmds_[time.get() % kCommandQueueBucketSize];

		while (!p.empty()) {
			const CmdQueue::CmdItem& it = p.top();
			if (it.cmd->duetime() > time) {
				// Time is the primary sorting key, so we can't have any additional commands in this
				// queue for this time
				break;
			}
			if (it.cmd->duetime() == time) {
				if (upcast(GameLogicCommand, cmd, it.cmd)) {
					// The id (aka command type)
					fw.unsigned_16(static_cast<uint16_t>(cmd->id()));

					// Serial number
					fw.signed_32(it.category);
					fw.unsigned_32(it.serial);

					// Now the command itself
					cmd->write(fw, game, *os);
				}
				++nhandled;
			}

			// DONE: next command
			p.pop();
		}
		time.increment();
	}

	fw.unsigned_16(0);  // end of command queue

	fw.write(fs, "binary/cmd_queue");
}
}  // namespace Widelands
