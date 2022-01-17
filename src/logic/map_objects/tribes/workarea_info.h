/*
 * Copyright (C) 2005-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_WORKAREA_INFO_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_WORKAREA_INFO_H

#include <map>
#include <string>

#include "logic/widelands_geometry.h"

/** The WorkareaInfo stores radii and for each radius a set of strings.
 *
 * A Workarea is a "circle" around a building that this building affects
 * or is needed by this buildingr., e.g. Areas for Mines, Fields of a Farm.
 * Worareas are shown on the Map when clicking on or placing a building.
 *
 * Each string contains a description of an activity (or similar)
 * that can be performed within the radius. Examples are buldings
 * that can be upgraded like a Fortress, and will have a bigger
 * workarea then.
 *
 * See LuaBuildingDescription::get_workarea_radius, InteractiveBase::show_workarea
 */

// TODO(Hasi50): We could just use a unit8 as base for the map? We should
// document (as const) the expected stings.

using WorkareaInfo = std::map<uint32_t, std::set<std::string>>;

// Visualization-related structs
struct WorkareaPreviewData {
	WorkareaPreviewData(Widelands::TCoords<> c, uint8_t i)
	   : coords(c), index(i), use_special_coloring(false), special_coloring(0) {
	}
	WorkareaPreviewData(Widelands::TCoords<> c, uint8_t i, uint32_t col)
	   : coords(c), index(i), use_special_coloring(true), special_coloring(col) {
	}
	WorkareaPreviewData()
	   : coords(Widelands::TCoords<>(Widelands::Coords::null(), Widelands::TriangleIndex::D)),
	     index(0),
	     use_special_coloring(false),
	     special_coloring(0) {
	}
	WorkareaPreviewData(const WorkareaPreviewData& other) = default;
	WorkareaPreviewData& operator=(const WorkareaPreviewData&) = default;

	bool operator==(const WorkareaPreviewData& d) const {
		return index == d.index && coords == d.coords &&
		       use_special_coloring == d.use_special_coloring &&
		       (!use_special_coloring || special_coloring == d.special_coloring);
	}

	// The triangle this data is applied to
	Widelands::TCoords<> coords;
	// The underlying workarea color
	uint8_t index;
	// If a "special coloring" is specified, its RGB will be overlayed over the base color as
	// strongly as if it had full alpha, and the final transparency of the entire triangle will be
	// set to this color's alpha
	bool use_special_coloring;
	uint32_t special_coloring;
};
// Pair of interior information and a per-circle list of border coords
using WorkareasEntry =
   std::pair<std::vector<WorkareaPreviewData>, std::vector<std::vector<Widelands::Coords>>>;
using Workareas = std::vector<WorkareasEntry>;

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WORKAREA_INFO_H
