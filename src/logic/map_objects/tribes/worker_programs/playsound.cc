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

#include "logic/map_objects/tribes/worker.h"

#include "sound/note_sound.h"

namespace Widelands {

/**
 * Demand from the g_sh to play a certain sound effect.
 * Whether the effect actually gets played is decided only by the sound server.
 */
bool Worker::run_playsound(Game& game, State& state, const Action& action) {
	Notifications::publish(NoteSound(
	   SoundType::kAmbient, action.iparam2, get_position(), action.iparam1, action.iparam3 == 1));

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

}  // namespace Widelands
