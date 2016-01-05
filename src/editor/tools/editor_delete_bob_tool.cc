/*
 * Copyright (C) 2002-2004, 2006-2008, 2012 by the Widelands Development Team
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

#include "editor/tools/editor_delete_bob_tool.h"

#include "editor/editorinteractive.h"
#include "logic/bob.h"
#include "logic/field.h"
#include "logic/mapregion.h"

/**
 * Deletes the bob at the given location
*/
int32_t
EditorDeleteBobTool::handle_click_impl(const Widelands::World&,
                                       Widelands::NodeAndTriangle<Widelands::Coords> center,
                                       EditorInteractive& parent,
                                       EditorActionArgs* args,
									   Widelands::Map* map) {
	Widelands::EditorGameBase & egbase = parent.egbase();
	const int32_t radius = args->sel_radius;
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
	(*map,
	 Widelands::Area<Widelands::FCoords>
	 (map->get_fcoords(center.node), radius));

	do if (Widelands::Bob * const bob = mr.location().field->get_first_bob()) {
			args->obob_type.push_back(&bob->descr());
			bob->remove(egbase);
		} else {
			args->obob_type.push_back(nullptr);
		}
	while (mr.advance(*map));
	return radius + 2;
}

int32_t
EditorDeleteBobTool::handle_undo_impl(const Widelands::World& world,
                                      Widelands::NodeAndTriangle<Widelands::Coords> center,
                                      EditorInteractive& parent,
                                      EditorActionArgs* args,
									  Widelands::Map* map) {

	uint32_t ret = parent.tools.place_bob.handle_undo_impl(world, center, parent, args, map);
	args->obob_type.clear();
	return ret;
}

EditorActionArgs EditorDeleteBobTool::format_args_impl(EditorInteractive & parent)
{
	return EditorTool::format_args_impl(parent);
}
