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

#ifndef WL_COMMANDS_CMD_NET_CHECK_SYNC_H
#define WL_COMMANDS_CMD_NET_CHECK_SYNC_H

#include <functional>

#include "commands/command.h"

namespace Widelands {

using SyncReportCallback = std::function<void()>;

/**
 * This non-gamelogic command is used by \ref GameHost and \ref GameClient
 * to schedule taking a synchronization hash.
 */
struct CmdNetCheckSync : public Widelands::Command {
	CmdNetCheckSync(const Time& dt, SyncReportCallback);

	void execute(Widelands::Game&) override;

	[[nodiscard]] Widelands::QueueCommandTypes id() const override {
		return Widelands::QueueCommandTypes::kNetCheckSync;
	}

private:
	SyncReportCallback callback_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_NET_CHECK_SYNC_H
