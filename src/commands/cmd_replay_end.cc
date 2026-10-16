/*
 * Copyright (C) 2004-2026 by the Widelands Development Team
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

#include "commands/cmd_replay_end.h"

#include "logic/game.h"
#include "logic/game_controller.h"

namespace Widelands {

void CmdReplayEnd::execute(Widelands::Game& game) {
	game.game_controller()->set_desired_speed(0);
	game.get_game_interface()->notify_replay_ended();
}

}  // namespace Widelands
