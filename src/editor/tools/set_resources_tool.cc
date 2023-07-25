/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#include "editor/tools/set_resources_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/decrease_resources_tool.h"
#include "editor/tools/increase_resources_tool.h"
#include "logic/field.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/mapregion.h"

int32_t EditorSetResourcesTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                  EditorActionArgs* args,
                                                  Widelands::Map* map) {
	const Widelands::Descriptions& descriptions = parent_.egbase().descriptions();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	auto gap_it = args->selection_gaps.cbegin();
	do {
		if (*gap_it++) {
			continue;
		}

		Widelands::ResourceAmount amount = args->set_to;
		Widelands::ResourceAmount max_amount =
		   args->current_resource != Widelands::kNoResource ?
            descriptions.get_resource_descr(args->current_resource)->max_amount() :
            0;

		if (amount > max_amount) {
			amount = max_amount;
		}

		if (map->is_resource_valid(descriptions, mr.location(), args->current_resource)) {

			args->original_resource.push_back(
			   EditorActionArgs::ResourceState{mr.location(), mr.location().field->get_resources(),
			                                   mr.location().field->get_resources_amount()});

			map->initialize_resources(mr.location(), args->current_resource, amount);
		}
	} while (mr.advance(*map));
	return mr.radius();
}

int32_t EditorSetResourcesTool::handle_undo_impl(
   const Widelands::NodeAndTriangle<Widelands::Coords>& /* center */,
   EditorActionArgs* args,
   Widelands::Map* map) {
	for (const auto& res : args->original_resource) {
		Widelands::ResourceAmount amount = res.amount;
		Widelands::ResourceAmount max_amount =
		   parent_.egbase().descriptions().get_resource_descr(args->current_resource)->max_amount();

		if (amount > max_amount) {
			amount = max_amount;
		}

		map->initialize_resources(map->get_fcoords(res.location), res.idx, amount);
	}

	args->original_resource.clear();
	return args->sel_radius;
}

EditorActionArgs EditorSetResourcesTool::format_args_impl() {
	EditorActionArgs a(parent_);
	a.current_resource = cur_res_;
	a.set_to = set_to_;
	return a;
}

Widelands::NodeCaps resource_tools_nodecaps(const Widelands::FCoords& fcoords,
                                            const Widelands::EditorGameBase& egbase,
                                            Widelands::DescriptionIndex resource) {
	if (egbase.map().is_resource_valid(egbase.descriptions(), fcoords, resource)) {
		return fcoords.field->nodecaps();
	}
	return Widelands::NodeCaps::CAPS_NONE;
}
