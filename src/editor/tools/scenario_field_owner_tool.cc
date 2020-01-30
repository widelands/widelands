/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "editor/tools/scenario_field_owner_tool.h"

#include "editor/editorinteractive.h"
#include "logic/field.h"
#include "logic/mapregion.h"
#include "logic/widelands_geometry.h"

int32_t ScenarioFieldOwnerTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                  EditorInteractive& eia,
                                                  EditorActionArgs* args,
                                                  Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));

	args->old_owners.clear();
	do {
		args->old_owners.push_back((*map)[mr.location()].get_owned_by());
	} while (mr.advance(*map));

	eia.egbase().conquer_area_no_building(Widelands::PlayerArea<Widelands::Area<Widelands::FCoords>>(
	   args->new_owner,
	   Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius)));
	return mr.radius();
}

int32_t ScenarioFieldOwnerTool::handle_undo_impl(const Widelands::NodeAndTriangle<>& center,
                                                 EditorInteractive& eia,
                                                 EditorActionArgs* args,
                                                 Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	std::list<uint8_t>::iterator i = args->old_owners.begin();
	do {
		eia.egbase().conquer_area_no_building(
		   Widelands::PlayerArea<Widelands::Area<Widelands::FCoords>>(
		      *i, Widelands::Area<Widelands::FCoords>(mr.location(), 0)));
		++i;
	} while (mr.advance(*map));
	return mr.radius();
}

EditorActionArgs ScenarioFieldOwnerTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.new_owner = new_owner_;
	return a;
}
