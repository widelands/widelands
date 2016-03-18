/*
 * Copyright (C) 2002-2004, 2006-2008, 2010, 2012 by the Widelands Development Team
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

#include "editor/tools/editor_set_resources_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/editor_decrease_resources_tool.h"
#include "editor/tools/editor_increase_resources_tool.h"
#include "graphic/graphic.h"
#include "logic/field.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapregion.h"

int32_t EditorSetResourcesTool::handle_click_impl(const Widelands::World& world,
                                                  Widelands::NodeAndTriangle<> const center,
                                                  EditorInteractive& /* parent */,
                                                  EditorActionArgs* args,
                                                  Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
	(*map,
	 Widelands::Area<Widelands::FCoords>
	 (map->get_fcoords(center.node), args->sel_radius));
	do {
		int32_t amount     = args->set_to;
		int32_t max_amount = args->current_resource != Widelands::kNoResource ?
							 world.get_resource(args->current_resource)->max_amount() : 0;
		if (amount < 0)
			amount = 0;
		else if (amount > max_amount)
			amount = max_amount;

		if (map->is_resource_valid(world, mr.location(), args->current_resource) &&
				mr.location().field->get_resources_amount() != amount) {

			args->original_resource.push_back(EditorActionArgs::ResourceState{
				mr.location(),
				mr.location().field->get_resources(),
				mr.location().field->get_resources_amount()
			});

			map->initialize_resources(mr.location(), args->current_resource, amount);
		}
	} while (mr.advance(*map));
	return mr.radius();
}

int32_t
EditorSetResourcesTool::handle_undo_impl(const Widelands::World& world,
                                         Widelands::NodeAndTriangle<Widelands::Coords> /* center */,
                                         EditorInteractive& /* parent */,
                                         EditorActionArgs* args,
                                         Widelands::Map* map) {
	for (const auto & res : args->original_resource) {
		int32_t amount     = res.amount;
		int32_t max_amount = world.get_resource(args->current_resource)->max_amount();

		if (amount < 0)
			amount = 0;
		if (amount > max_amount)
			amount = max_amount;

		map->initialize_resources(res.location, res.idx, amount);
	}

	args->original_resource.clear();
	return args->sel_radius;
}

EditorActionArgs EditorSetResourcesTool::format_args_impl(EditorInteractive & parent)
{
	EditorActionArgs a(parent);
	a.current_resource = cur_res_;
	a.set_to = set_to_;
	return a;
}
