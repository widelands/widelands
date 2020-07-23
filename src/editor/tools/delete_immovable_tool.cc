/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "editor/tools/delete_immovable_tool.h"

#include "base/macros.h"
#include "editor/editorinteractive.h"
#include "logic/field.h"
#include "logic/map_objects/immovable.h"
#include "logic/mapregion.h"

/**
 * Deletes the immovable at the given location
 */
int32_t EditorDeleteImmovableTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                     EditorInteractive& eia,
                                                     EditorActionArgs* args,
                                                     Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		if (upcast(Widelands::Immovable, immovable, mr.location().field->get_immovable())) {
			args->old_immovable_types.push_back(immovable->descr().name());
			immovable->remove(eia.egbase());  //  Delete no buildings or stuff.
		} else {
			args->old_immovable_types.push_back("");
		}
	} while (mr.advance(*map));
	return mr.radius() + 2;
}

int32_t EditorDeleteImmovableTool::handle_undo_impl(
   const Widelands::NodeAndTriangle<Widelands::Coords>& center,
   EditorInteractive& parent,
   EditorActionArgs* args,
   Widelands::Map* map) {
	return parent.tools()->place_immovable.handle_undo_impl(center, parent, args, map);
}

EditorActionArgs EditorDeleteImmovableTool::format_args_impl(EditorInteractive& parent) {
	return EditorTool::format_args_impl(parent);
}
