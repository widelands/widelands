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

#ifndef WL_COMMANDS_CMD_REPLAY_SYNC_WRITE_H
#define WL_COMMANDS_CMD_REPLAY_SYNC_WRITE_H

#include "commands/command.h"

namespace Widelands {

constexpr Duration kReplaySyncInterval(200);

/**
 * Command / timer that regularly inserts synchronization hashes into
 * the replay.
 */
class CmdReplaySyncWrite : public Command {
public:
	explicit CmdReplaySyncWrite(const Time& init_duetime) : Command(init_duetime) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kReplaySyncWrite;
	}

	void execute(Game& game) override;
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_REPLAY_SYNC_WRITE_H
