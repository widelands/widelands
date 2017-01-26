/*
 * Copyright (C) 2005-2017 by the Widelands Development Team
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

#include <cstring>
#include <map>
#include <set>
#include <string>

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
 * See LuaBuildingDescription::get_workarea_radius, InteractiveBase::show_work_area
 */

// TODO(Hasi50): LuaBuildingDescription::get_workarea_radius will only give us the very first
// size found, which seems to be correct but depends on the std:map implementation
//
// We could just use a unit8 as base for the map?
// We should document (as const) the expected stings.

using WorkareaInfo = std::map<uint32_t, std::set<std::string>>;

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WORKAREA_INFO_H
