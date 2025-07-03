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

#ifndef WL_COMMANDS_CMD_REPLAY_SYNC_READ_H
#define WL_COMMANDS_CMD_REPLAY_SYNC_READ_H

#include "base/crypto.h"
#include "commands/command.h"

namespace Widelands {

class CmdReplaySyncRead : public Command {
public:
	CmdReplaySyncRead(const Time& init_duetime, const crypto::MD5Checksum& hash)
	   : Command(init_duetime), hash_(hash) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kReplaySyncRead;
	}

	void execute(Game& game) override;

private:
	crypto::MD5Checksum hash_;

	static const Game* reported_desync_for_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_REPLAY_SYNC_READ_H
