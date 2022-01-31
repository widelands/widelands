/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_ROAD_SEGMENTS_H
#define WL_GRAPHIC_ROAD_SEGMENTS_H

namespace Widelands {

enum RoadSegment {
	kNone = 0,
	kNormal = 1,
	kBusy = 2,
	kWaterway = 3,
	kBridgeNormal = 4,
	kBridgeBusy = 5,
};

bool is_bridge_segment(RoadSegment);

}  // namespace Widelands

#endif  // end of include guard: WL_GRAPHIC_ROAD_SEGMENTS_H
