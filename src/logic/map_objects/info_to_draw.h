/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_INFO_TO_DRAW_H
#define WL_LOGIC_MAP_OBJECTS_INFO_TO_DRAW_H

// This is a bitmask; use powers of 2.
enum InfoToDraw {
	kNone = 0,
	kCensus = 1,
	kStatistics = 2,
	kSoldierLevels = 4,
	kShowBuildings = 8,
};

inline InfoToDraw operator|(InfoToDraw a, InfoToDraw b) {
	return static_cast<InfoToDraw>(static_cast<int>(a) | static_cast<int>(b));
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_INFO_TO_DRAW_H
