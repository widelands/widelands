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

#include "editor/tools/editor_increase_resources_tool.h"

#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "logic/field.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapregion.h"

using Widelands::TCoords;

int32_t EditorIncreaseResourcesTool::handle_click_impl(const Widelands::World& world,
                                                       Widelands::NodeAndTriangle<> const center,
                                                       EditorInteractive& /* parent */,
                                                       EditorActionArgs* args,
                                                       Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(*map,
			Widelands::Area<Widelands::FCoords>
				(map->get_fcoords(center.node), args->sel_radius));
	do {
		int32_t amount = mr.location().field->get_resources_amount();
		int32_t max_amount = args->cur_res != Widelands::kNoResource ?
							 world.get_resource(args->cur_res)->max_amount() : 0;

		amount += args->change_by;
		if (amount > max_amount)
			amount = max_amount;

		if ((mr.location().field->get_resources() == args->cur_res ||
				!mr.location().field->get_resources_amount()) &&
				map->is_resource_valid(world, mr.location(), args->cur_res)) {
			args->orgResT.push_back(mr.location().field->get_resources());
			args->orgRes.push_back(mr.location().field->get_resources_amount());
			map->initialize_resources(mr.location(), args->cur_res, amount);
		}
	} while (mr.advance(*map));
	return mr.radius();
}

int32_t EditorIncreaseResourcesTool::handle_undo_impl(const Widelands::World& world,
		                                              Widelands::NodeAndTriangle<Widelands::Coords> center,
													  EditorInteractive& parent,
													  EditorActionArgs* args,
													  Widelands::Map* map) {
	return m_set_tool.handle_undo_impl(world, center, parent, args, map);
}

EditorActionArgs EditorIncreaseResourcesTool::format_args_impl(EditorInteractive & parent)
{
	EditorActionArgs a(parent);
	a.change_by = m_change_by;
	a.cur_res = m_cur_res;
	return a;
}
