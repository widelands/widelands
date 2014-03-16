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

#ifndef DIRANIMATIONS_H
#define DIRANIMATIONS_H

#include <string>

#include <stdint.h>

#include "logic/widelands.h"

namespace Widelands {struct Map_Object_Descr;}

struct Profile;
struct Section;

/// Manages a set of 6 animations, one for each possible direction.
struct DirAnimations {
	explicit DirAnimations
		(uint32_t dir1 = 0, uint32_t dir2 = 0, uint32_t dir3 = 0,
		 uint32_t dir4 = 0, uint32_t dir5 = 0, uint32_t dir6 = 0);

	void parse
		(Widelands::Map_Object_Descr &,
		 const std::string           & directory,
		 Profile                     &,
		 const std::string & name,
		 bool optional = false,
		 const std::string & default_dirpics = std::string());

	uint32_t get_animation(Widelands::Direction const dir) const {
		return m_animations[dir - 1];
	}

	static DirAnimations Null() {
		return DirAnimations(0); // Since real animation IDs are positive, this is safe
	}

	operator bool() const {return m_animations[0];}

private:
	uint32_t m_animations[6];
};

#endif
