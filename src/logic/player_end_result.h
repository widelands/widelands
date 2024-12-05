/*
 * Copyright (C) 2008-2024 by the Widelands Development Team
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

#ifndef WL_LOGIC_PLAYER_END_RESULT_H
#define WL_LOGIC_PLAYER_END_RESULT_H

#include <cstdint>

namespace Widelands {
enum class PlayerEndResult : uint8_t {
	kLost = 0,        // player determined to have lost, but is tecnically still capable of
	                  // carrying on if the "continue playing" option is selected
	                  // e.g. has less X after time limit elapsed
	kWon = 1,         // player has already won; also able to continue playing
	kResigned = 2,    // player has voluntarily left the game
	kEliminated = 3,  // player has been eliminated from the game; they have not only lost,
	                  // but are also no longer capable of (or allowed to) continue playing
	kUndefined = 255
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_PLAYER_END_RESULT_H
