/*
 * Copyright (C) 2002-2004, 2006-2010, 2013 by the Widelands Development Team
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

#ifndef WALKINGDIR_H
#define WALKINGDIR_H

#include <stdint.h>

namespace Widelands {

/// Constants for where we are going.
enum WalkingDir {
	IDLE            = 0,
	FIRST_DIRECTION = 1,
	WALK_NE         = 1,
	WALK_E          = 2,
	WALK_SE         = 3,
	WALK_SW         = 4,
	WALK_W          = 5,
	WALK_NW         = 6,
	LAST_DIRECTION  = 6,
};

uint8_t get_cw_neighbour(uint8_t dir);
uint8_t get_ccw_neighbour(uint8_t dir);
uint8_t get_backward_dir(uint8_t dir);

}

#endif
