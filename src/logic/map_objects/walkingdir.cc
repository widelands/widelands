/*
 * Copyright (C) 2013-2022 by the Widelands Development Team
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

#include "logic/map_objects/walkingdir.h"

#include "base/wexception.h"

namespace Widelands {

/// \returns the neighbour direction in clockwise
WalkingDir get_cw_neighbour(WalkingDir dir) {
	switch (dir) {
	case WalkingDir::WALK_NE:
		return WalkingDir::WALK_E;
	case WalkingDir::WALK_E:
		return WalkingDir::WALK_SE;
	case WalkingDir::WALK_SE:
		return WalkingDir::WALK_SW;
	case WalkingDir::WALK_SW:
		return WalkingDir::WALK_W;
	case WalkingDir::WALK_W:
		return WalkingDir::WALK_NW;
	case WalkingDir::WALK_NW:
		return WalkingDir::WALK_NE;
	case WalkingDir::IDLE:
		return WalkingDir::IDLE;
	}
	NEVER_HERE();
}

/// \returns the neighbour direction in counterclockwise
WalkingDir get_ccw_neighbour(WalkingDir dir) {
	switch (dir) {
	case WalkingDir::WALK_E:
		return WalkingDir::WALK_NE;
	case WalkingDir::WALK_NE:
		return WalkingDir::WALK_NW;
	case WalkingDir::WALK_NW:
		return WalkingDir::WALK_W;
	case WalkingDir::WALK_W:
		return WalkingDir::WALK_SW;
	case WalkingDir::WALK_SW:
		return WalkingDir::WALK_SE;
	case WalkingDir::WALK_SE:
		return WalkingDir::WALK_E;
	case WalkingDir::IDLE:
		return WalkingDir::IDLE;
	}
	NEVER_HERE();
}

WalkingDir get_backward_dir(WalkingDir dir) {
	switch (dir) {
	case WalkingDir::WALK_E:
		return WalkingDir::WALK_W;
	case WalkingDir::WALK_NE:
		return WalkingDir::WALK_SW;
	case WalkingDir::WALK_NW:
		return WalkingDir::WALK_SE;
	case WalkingDir::WALK_W:
		return WalkingDir::WALK_E;
	case WalkingDir::WALK_SW:
		return WalkingDir::WALK_NE;
	case WalkingDir::WALK_SE:
		return WalkingDir::WALK_NW;
	case WalkingDir::IDLE:
		return WalkingDir::IDLE;
	}
	NEVER_HERE();
}
}  // namespace Widelands
