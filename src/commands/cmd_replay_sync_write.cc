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

#include "commands/cmd_replay_sync_write.h"

#include "logic/game.h"
#include "logic/replay.h"

namespace Widelands {

void CmdReplaySyncWrite::execute(Game& game) {
	if (ReplayWriter* const rw = game.get_replaywriter()) {
		rw->send_sync(game.get_sync_hash());

		game.enqueue_command(new CmdReplaySyncWrite(duetime() + kReplaySyncInterval));
	}
}

}  // namespace Widelands
