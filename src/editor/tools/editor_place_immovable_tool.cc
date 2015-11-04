/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2012 by the Widelands Development Team
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

#include "editor/tools/editor_place_immovable_tool.h"

#include <string>

#include "base/macros.h"
#include "editor/editorinteractive.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/immovable.h"
#include "logic/mapregion.h"

/**
 * Choses an object to place randomly from all enabled
 * and places this on the current field
*/
int32_t EditorPlaceImmovableTool::handle_click_impl(Widelands::Map& map,
                                                       const Widelands::World&,
                                                       Widelands::NodeAndTriangle<> const center,
                                                       EditorInteractive& parent,
                                                       EditorActionArgs& args) {
	const int32_t radius = args.sel_radius;
	if (!get_nr_enabled())
		return radius;
	Widelands::EditorGameBase & egbase = parent.egbase();
	if (args.oimmov_types.empty())
	{
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 (map.get_fcoords(center.node), radius));
		do {
			const Widelands::BaseImmovable * im = mr.location().field->get_immovable();
			args.oimmov_types.push_back((im ? im->descr().name() : ""));
			args.nimmov_types.push_back(get_random_enabled());
		} while (mr.advance(map));
	}

	if (!args.nimmov_types.empty())
	{
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 (map.get_fcoords(center.node), radius));
		std::list<int32_t>::iterator i = args.nimmov_types.begin();
		do {
			if
			(!mr.location().field->get_immovable()
			        &&
			        (mr.location().field->nodecaps() & Widelands::MOVECAPS_WALK))
				egbase.create_immovable(mr.location(), *i);
			++i;
		} while (mr.advance(map));
	}
	return radius + 2;
}

int32_t EditorPlaceImmovableTool::handle_undo_impl(
   Widelands::Map& map,
   const Widelands::World&,
   Widelands::NodeAndTriangle<Widelands::Coords> center,
   EditorInteractive& parent,
   EditorActionArgs& args) {
	const int32_t radius = args.sel_radius;
	if (args.oimmov_types.empty())
		return radius;

	Widelands::EditorGameBase & egbase = parent.egbase();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
	(map,
	 Widelands::Area<Widelands::FCoords>
	 (map.get_fcoords(center.node), radius));
	std::list<std::string>::iterator i = args.oimmov_types.begin();
	do {
		if
			(upcast(Widelands::Immovable, immovable,
		           mr.location().field->get_immovable()))
		{
			immovable->remove(egbase);
		}
		if (!i->empty())
			egbase.create_immovable(mr.location(), *i);

		++i;
	} while (mr.advance(map));
	return radius + 2;
}

EditorActionArgs EditorPlaceImmovableTool::format_args_impl(EditorInteractive & parent)
{
	return EditorTool::format_args_impl(parent);
}
