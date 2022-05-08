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

#include "logic/cmd_queue.h"

#include "base/macros.h"
#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"
#include "logic/playercommand.h"

namespace Widelands {

//
// class Cmd_Queue
//
CmdQueue::CmdQueue(Game& game)
   : game_(game),
     nextserial_(0),
     ncmds_(0),
     cmds_(kCommandQueueBucketSize, std::priority_queue<CmdItem>()) {
}

CmdQueue::~CmdQueue() {
	flush();
}

/*
 * flushs all commands from the queue. Needed for
 * game loading (while in game)
 */
// TODO(unknown): ...but game loading while in game is not possible!
// Note: Order of destruction of Items is not guaranteed
void CmdQueue::flush() {
	uint32_t cbucket = 0;
	while ((ncmds_ != 0u) && cbucket < kCommandQueueBucketSize) {
		std::priority_queue<CmdItem>& current_cmds = cmds_[cbucket];

		while (!current_cmds.empty()) {
			Command* cmd = current_cmds.top().cmd;
			current_cmds.pop();
			delete cmd;
			--ncmds_;
		}
		++cbucket;
	}
	assert(ncmds_ == 0);
}

/*
===============
Insert a new command into the queue; it will be executed at the given time
===============
*/
void CmdQueue::enqueue(Command* const cmd) {
	CmdItem ci;

	ci.cmd = cmd;
	if (upcast(PlayerCommand, plcmd, cmd)) {
		ci.category = cat_playercommand;
		ci.serial = plcmd->cmdserial();
	} else if (dynamic_cast<GameLogicCommand*>(cmd) != nullptr) {
		ci.category = cat_gamelogic;
		ci.serial = nextserial_++;
	} else {
		// the order of non-gamelogic commands matters only with respect to
		// gamelogic commands; the order of non-gamelogic commands wrt other
		// non-gamelogic commands shouldn't matter, so we can assign a
		// constant serial number.
		ci.category = cat_nongamelogic;
		ci.serial = 0;
	}

	assert(cmd->duetime() >= game_.get_gametime());
	cmds_[cmd->duetime().get() % kCommandQueueBucketSize].push(ci);
	++ncmds_;
}

void CmdQueue::run_queue(const Duration& interval, Time& game_time_var) {
	const Time final_time = game_time_var + interval;

	while (game_time_var < final_time) {
		std::priority_queue<CmdItem>& current_cmds =
		   cmds_[game_time_var.get() % kCommandQueueBucketSize];

		while (!current_cmds.empty()) {
			Command& c = *current_cmds.top().cmd;
			if (game_time_var < c.duetime()) {
				break;
			}

			current_cmds.pop();
			--ncmds_;
			assert(game_time_var == c.duetime());

			if (dynamic_cast<GameLogicCommand*>(&c) != nullptr) {
				StreamWrite& ss = game_.syncstream();
				ss.unsigned_8(SyncEntry::kRunQueue);
				ss.unsigned_32(c.duetime().get());
				ss.unsigned_32(static_cast<uint32_t>(c.id()));
			}

			c.execute(game_);

			delete &c;
		}
		game_time_var.increment();
	}

	assert(final_time == game_time_var);
}

constexpr uint16_t kCurrentPacketVersion = 1;

/**
 * Write variables from the base command to a file.
 *
 * \note This function must be called by deriving objects that override it.
 */
void GameLogicCommand::write(FileWrite& fw,
#ifndef NDEBUG
                             EditorGameBase& egbase,
#else
                             EditorGameBase&,
#endif
                             MapObjectSaver& /* mos */) {
	fw.unsigned_16(kCurrentPacketVersion);

	// Write duetime
	assert(egbase.get_gametime() <= duetime());
	duetime().save(fw);
}

/**
 * Read variables for the base command from a file.
 *
 * \note This function must be called by deriving objects that override it.
 */
void GameLogicCommand::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& /* mol */) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			set_duetime(Time(fr));
			const Time& gametime = egbase.get_gametime();
			if (duetime() < gametime) {
				throw GameDataError("duetime (%i) < gametime (%i)", duetime().get(), gametime.get());
			}
		} else {
			throw UnhandledVersionError("GameLogicCommand", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("game logic: %s", e.what());
	}
}
}  // namespace Widelands
