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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_WALKINGDIR_H
#define WL_LOGIC_MAP_OBJECTS_WALKINGDIR_H

namespace Widelands {

/// Constants for where we are going.
enum WalkingDir {
	IDLE = 0,
	FIRST_DIRECTION = 1,
	WALK_NE = 1,
	WALK_E = 2,
	WALK_SE = 3,
	WALK_SW = 4,
	WALK_W = 5,
	WALK_NW = 6,
	LAST_DIRECTION = 6,
};

WalkingDir get_cw_neighbour(WalkingDir dir);
WalkingDir get_ccw_neighbour(WalkingDir dir);
WalkingDir get_backward_dir(WalkingDir dir);
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_WALKINGDIR_H
