/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_COMMANDS_CMD_QUEUE_H
#define WL_COMMANDS_CMD_QUEUE_H

#include <queue>

#include "commands/command.h"

namespace Widelands {

constexpr uint32_t kCommandQueueBucketSize = 65536;  // Make this a power of two, so that % is fast

// This is the command queue. It is fully widelands specific,
// it needs to know nearly all modules.
//
// It used to be implemented as a priority_queue sorted by execution_time,
// serial and type of commands. This proved to be a performance bottleneck on
// big games. I then changed this to use a constant size hash_map[gametime] of
// priority_queues. This allows for ~O(1) access by time and in my analyses,
// practically all buckets were used, so there is not much memory overhead.
// This removed the bottleneck for big games.
//
// I first tried with unordered_map, but as expected, it grew beyond all
// limits when accessed with gametime. Therefore I reverted back to a simple
// vector.
//
// The price we pay is that when saving, we also have to traverse till we no
// longer find any new command to write. This could theoretically take forever
// but in my tests it was not noticeable.

class CmdQueue {
	friend struct GameCmdQueuePacket;

	enum { cat_nongamelogic = 0, cat_gamelogic, cat_playercommand };

	struct CmdItem {
		Command* cmd;

		/**
		 * category and serial are used to sort commands such that
		 * commands will be executed in the same order on all systems
		 * independent of details of the priority_queue implementation.
		 */
		int32_t category;
		uint32_t serial;

		bool operator<(const CmdItem& c) const {
			if (cmd->duetime() != c.cmd->duetime()) {
				return cmd->duetime() > c.cmd->duetime();
			}
			if (category != c.category) {
				return category > c.category;
			}
			return serial > c.serial;
		}
	};

public:
	explicit CmdQueue(Game&);
	~CmdQueue();

	/// Add a command to the queue. Takes ownership.
	void enqueue(Command*);

	// Run all commands scheduled for the next interval milliseconds, and update
	// the internal time as well. the game_time_var represents the current game
	// time, which we update and with which we must mess around (to run all
	// queued cmd.s) and which we update (add the interval)
	void run_queue(const Duration& interval, Time& game_time_var);

	void flush();  // delete all commands in the queue now

private:
	Game& game_;
	uint32_t nextserial_{0};
	uint32_t ncmds_{0};
	using CommandsContainer = std::vector<std::priority_queue<CmdItem>>;
	CommandsContainer cmds_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_QUEUE_H
