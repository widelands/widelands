/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_LOGIC_VISION_H
#define WL_LOGIC_VISION_H

namespace Widelands {

// Simplified vision information reduced to 3 states.
// Do not change the order! It is stored in savegames.
enum class VisibleState {
	kUnexplored = 0,      // Never seen yet
	kPreviouslySeen = 1,  // Previously seen
	kVisible = 2          // Currently visible
};

// Extended vision information encoded as a number with some helper functions.
// See Player::Field for explanation of values.
struct Vision {
	uint16_t value;

	Vision(uint16_t v = 0) : value{v} {
	}

	operator VisibleState() const {
		switch (value) {
		case 0:
			return VisibleState::kUnexplored;
		case 1:
			return VisibleState::kPreviouslySeen;
		default:
			return VisibleState::kVisible;
		}
	}

	bool is_explored() const {
		return value > 0;
	}

	bool is_visible() const {
		return value > 1;
	}

	bool is_seen_by_us() const {
		return value > 2;
	}

	bool is_revealed() const {
		return value > 2 && value % 2 == 1;
	}

	uint16_t seers() const {
		return value > 3 ? (value - 2) / 2 : 0;
	}
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_VISION_H
