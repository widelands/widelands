/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "editor/tools/set_port_space_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/mapfringeregion.h"
#include "logic/mapregion.h"

using namespace Widelands;

/// static callback function for overlay calculation
int32_t editor_Tool_set_port_space_callback
	(const Widelands::TCoords<Widelands::FCoords>& c, const Map& map)
{
	NodeCaps const caps = c.field->nodecaps();
	FCoords f = map.get_fcoords(*c.field);
	if ((caps & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG) {
		if (!map.find_portdock(f).empty())
			return caps;
	}
	return 0;
}


EditorSetPortSpaceTool::EditorSetPortSpaceTool
(EditorUnsetPortSpaceTool & the_unset_tool)
	:
	EditorTool(the_unset_tool, *this)
{}


EditorUnsetPortSpaceTool::EditorUnsetPortSpaceTool()
	:
	EditorTool(*this, *this)
{}

int32_t EditorSetPortSpaceTool::handle_click_impl(const Widelands::World& world,
                                                  Widelands::NodeAndTriangle<> const center,
                                                  EditorInteractive&,
                                                  EditorActionArgs* args,
												  Map* map) {
	assert(0 <= center.node.x);
	assert(center.node.x < map->get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map->get_height());

	uint32_t nr = 0;

	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(*map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		//  check if field is valid
		if (editor_Tool_set_port_space_callback(mr.location(), *map)) {
			map->set_port_space(mr.location(), true);
			Area<FCoords> a(mr.location(), 0);
			map->recalc_for_field_area(world, a);
			++nr;
		}
	} while (mr.advance(*map));

	return nr;
}

int32_t EditorSetPortSpaceTool::handle_undo_impl(const Widelands::World& world,
                                                 NodeAndTriangle<Coords> center,
                                                 EditorInteractive& parent,
                                                 EditorActionArgs* args,
												 Map* map) {
	return parent.tools()->unset_port_space.handle_click_impl(world, center, parent, args, map);
}

int32_t EditorUnsetPortSpaceTool::handle_click_impl(const Widelands::World& world,
                                                    NodeAndTriangle<> const center,
                                                    EditorInteractive&,
                                                    EditorActionArgs* args,
													Map* map) {
	assert(0 <= center.node.x);
	assert(center.node.x < map->get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map->get_height());

	uint32_t nr = 0;

	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(*map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		//  check if field is valid
		if (editor_Tool_set_port_space_callback(mr.location(), *map)) {
			map->set_port_space(mr.location(), false);
			Area<FCoords> a(mr.location(), 0);
			map->recalc_for_field_area(world, a);
			++nr;
		}
	} while (mr.advance(*map));

	return nr;
}

int32_t EditorUnsetPortSpaceTool::handle_undo_impl(const Widelands::World& world,
                                                   NodeAndTriangle<Coords> center,
                                                   EditorInteractive& parent,
                                                   EditorActionArgs* args,
												   Map* map) {
	return parent.tools()->set_port_space.handle_click_impl(world, center, parent, args, map);
}
