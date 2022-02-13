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

#include "editor/tools/increase_resources_tool.h"

#include "editor/editorinteractive.h"
#include "logic/field.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/mapregion.h"

int32_t EditorIncreaseResourcesTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                       EditorInteractive& eia,
                                                       EditorActionArgs* args,
                                                       Widelands::Map* map) {
	const Widelands::Descriptions& descriptions = eia.egbase().descriptions();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		Widelands::ResourceAmount amount = mr.location().field->get_resources_amount();
		Widelands::ResourceAmount max_amount =
		   args->current_resource != Widelands::kNoResource ?
            descriptions.get_resource_descr(args->current_resource)->max_amount() :
            0;

		amount += args->change_by;
		if (amount > max_amount) {
			amount = max_amount;
		}

		if ((mr.location().field->get_resources() == args->current_resource ||
		     !mr.location().field->get_resources_amount()) &&
		    map->is_resource_valid(descriptions, mr.location(), args->current_resource) &&
		    mr.location().field->get_resources_amount() != max_amount) {

			args->original_resource.push_back(
			   EditorActionArgs::ResourceState{mr.location(), mr.location().field->get_resources(),
			                                   mr.location().field->get_resources_amount()});

			map->initialize_resources(mr.location(), args->current_resource, amount);
		}
	} while (mr.advance(*map));
	return mr.radius();
}

int32_t EditorIncreaseResourcesTool::handle_undo_impl(
   const Widelands::NodeAndTriangle<Widelands::Coords>& center,
   EditorInteractive& parent,
   EditorActionArgs* args,
   Widelands::Map* map) {
	return set_tool_.handle_undo_impl(center, parent, args, map);
}

EditorActionArgs EditorIncreaseResourcesTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.change_by = change_by_;
	a.current_resource = cur_res_;
	return a;
}
