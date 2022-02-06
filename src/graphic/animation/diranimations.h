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

#ifndef WL_GRAPHIC_ANIMATION_DIRANIMATIONS_H
#define WL_GRAPHIC_ANIMATION_DIRANIMATIONS_H

#include "logic/widelands.h"

/// Manages a set of 6 animations, one for each possible direction.
struct DirAnimations {
	DirAnimations(uint32_t dir1 = 0,
	              uint32_t dir2 = 0,
	              uint32_t dir3 = 0,
	              uint32_t dir4 = 0,
	              uint32_t dir5 = 0,
	              uint32_t dir6 = 0);

	uint32_t get_animation(Widelands::Direction const dir) const {
		return animations_[dir - 1];
	}
	void set_animation(const Widelands::Direction dir, const uint32_t anim) {
		animations_[dir - 1] = anim;
	}

	static DirAnimations null() {
		return DirAnimations(0);  // Since real animation IDs are positive, this is safe
	}

	operator bool() const {
		return animations_[0];
	}

private:
	uint32_t animations_[6];
};

#endif  // end of include guard: WL_GRAPHIC_ANIMATION_DIRANIMATIONS_H
