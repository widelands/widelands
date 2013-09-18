/*
 * Copyright (C) 2013 by the Widelands Development Team
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

#include "logic/walkingdir.h"

namespace Widelands {

/// \returns the neighbour direction in clockwise
uint8_t get_cw_neighbour(uint8_t dir) {
	switch (dir) {
		case WALK_NE:
			return WALK_E;
		case WALK_E:
			return WALK_SE;
		case WALK_SE:
			return WALK_SW;
		case WALK_SW:
			return WALK_W;
		case WALK_W:
			return WALK_NW;
		case WALK_NW:
			return WALK_NE;
		default:
			return 0;
	}
}

/// \returns the neighbour direction in counterclockwise
uint8_t get_ccw_neighbour(uint8_t dir) {
	switch (dir) {
		case WALK_E:
			return WALK_NE;
		case WALK_NE:
			return WALK_NW;
		case WALK_NW:
			return WALK_W;
		case WALK_W:
			return WALK_SW;
		case WALK_SW:
			return WALK_SE;
		case WALK_SE:
			return WALK_E;
		default:
			return 0;
	}
}

uint8_t get_backward_dir(uint8_t dir) {
	switch (dir) {
		case WALK_E:
			return WALK_W;
		case WALK_NE:
			return WALK_SW;
		case WALK_NW:
			return WALK_SE;
		case WALK_W:
			return WALK_E;
		case WALK_SW:
			return WALK_NE;
		case WALK_SE:
			return WALK_NW;
		default:
			return 0;
	}
}

};

