/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_DIRANIMATIONS_H
#define WL_GRAPHIC_DIRANIMATIONS_H

#include <string>

#include <stdint.h>

#include "logic/widelands.h"

/// Manages a set of 6 animations, one for each possible direction.
struct DirAnimations {
	DirAnimations
		(uint32_t dir1 = 0, uint32_t dir2 = 0, uint32_t dir3 = 0,
		 uint32_t dir4 = 0, uint32_t dir5 = 0, uint32_t dir6 = 0);

	uint32_t get_animation(Widelands::Direction const dir) const {
		return m_animations[dir - 1];
	}
	void set_animation(const Widelands::Direction dir, const uint32_t anim) {
		m_animations[dir - 1] = anim;
	}

	static DirAnimations null() {
		return DirAnimations(0); // Since real animation IDs are positive, this is safe
	}

	operator bool() const {return m_animations[0];}

private:
	uint32_t m_animations[6];
};

#endif  // end of include guard: WL_GRAPHIC_DIRANIMATIONS_H
