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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "editor/tools/decrease_height_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/increase_height_tool.h"
#include "editor/tools/set_height_tool.h"
#include "logic/field.h"
#include "logic/mapregion.h"

/// Decreases the heights by a value. Chages surrounding nodes if necessary.
int32_t EditorDecreaseHeightTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                    EditorActionArgs* args,
                                                    Widelands::Map* map) {
	if (args->original_heights.empty()) {
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   *map, Widelands::Area<Widelands::FCoords>(
		            map->get_fcoords(center.node),
		            args->sel_radius + MAX_FIELD_HEIGHT / MAX_FIELD_HEIGHT_DIFF + 1));
		do {
			args->original_heights.push_back(mr.location().field->get_height());
		} while (mr.advance(*map));
	}

	return map->change_height(
	   parent_.egbase(),
	   Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius),
	   -args->change_by);
}

int32_t EditorDecreaseHeightTool::handle_undo_impl(const Widelands::NodeAndTriangle<>& center,
                                                   EditorActionArgs* args,
                                                   Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(
	            map->get_fcoords(center.node),
	            args->sel_radius + MAX_FIELD_HEIGHT / MAX_FIELD_HEIGHT_DIFF + 1));
	std::list<Widelands::Field::Height>::iterator i = args->original_heights.begin();

	do {
		mr.location().field->set_height(*i);
		++i;
	} while (mr.advance(*map));

	map->recalc_for_field_area(
	   parent_.egbase(), Widelands::Area<Widelands::FCoords>(
	                        map->get_fcoords(center.node),
	                        args->sel_radius + MAX_FIELD_HEIGHT / MAX_FIELD_HEIGHT_DIFF + 2));

	return mr.radius() + 1;
}

EditorActionArgs EditorDecreaseHeightTool::format_args_impl() {
	EditorActionArgs a(parent_);
	a.change_by = change_by_;
	return a;
}
