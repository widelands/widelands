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

#include "editor/tools/noise_height_tool.h"

#include <cstdlib>

#include "editor/editorinteractive.h"
#include "editor/tools/decrease_height_tool.h"
#include "editor/tools/increase_height_tool.h"
#include "editor/tools/set_height_tool.h"
#include "logic/field.h"
#include "logic/mapregion.h"

/// Sets the heights to random values. Changes surrounding nodes if necessary.
int32_t EditorNoiseHeightTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                 EditorInteractive& eia,
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

	uint32_t max = 0;

	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		max = std::max(
		   max, map->set_height(eia.egbase(), mr.location(),
		                        args->interval.min +
		                           RNG::static_rand(args->interval.max - args->interval.min + 1)));
	} while (mr.advance(*map));
	return mr.radius() + max;
}

int32_t
EditorNoiseHeightTool::handle_undo_impl(const Widelands::NodeAndTriangle<Widelands::Coords>& center,
                                        EditorInteractive& parent,
                                        EditorActionArgs* args,
                                        Widelands::Map* map) {
	return set_tool_.handle_undo_impl(center, parent, args, map);
}

EditorActionArgs EditorNoiseHeightTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.interval = interval_;
	return a;
}
