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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "editor/tools/set_port_space_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/mapfringeregion.h"
#include "logic/mapregion.h"

using namespace Widelands;

namespace {

Widelands::NodeCaps port_tool_nodecaps(const Widelands::FCoords& c, const Map& map) {
	NodeCaps const caps = c.field->nodecaps();
	if ((caps & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG) {
		if (!map.find_portdock(c, false).empty()) {
			return caps;
		}
	}
	return NodeCaps::CAPS_NONE;
}

}  // namespace

EditorSetPortSpaceTool::EditorSetPortSpaceTool(EditorUnsetPortSpaceTool& the_unset_tool)
   : EditorTool(the_unset_tool, the_unset_tool) {
}

EditorUnsetPortSpaceTool::EditorUnsetPortSpaceTool() : EditorTool(*this, *this) {
}

int32_t EditorSetPortSpaceTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                  EditorInteractive& eia,
                                                  EditorActionArgs* args,
                                                  Map* map) {
	assert(0 <= center.node.x);
	assert(center.node.x < map->get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map->get_height());

	uint32_t nr = 0;

	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		//  check if field is valid
		if (port_tool_nodecaps(mr.location(), *map) != NodeCaps::CAPS_NONE) {
			map->set_port_space(eia.egbase(), mr.location(), true);
			Area<FCoords> a(mr.location(), 0);
			map->recalc_for_field_area(eia.egbase(), a);
			++nr;
		}
	} while (mr.advance(*map));

	return nr;
}

Widelands::NodeCaps
EditorSetPortSpaceTool::nodecaps_for_buildhelp(const Widelands::FCoords& fcoords,
                                               const Widelands::EditorGameBase& egbase) {
	return port_tool_nodecaps(fcoords, egbase.map());
}

int32_t EditorSetPortSpaceTool::handle_undo_impl(const NodeAndTriangle<Coords>& center,
                                                 EditorInteractive& parent,
                                                 EditorActionArgs* args,
                                                 Map* map) {
	return parent.tools()->unset_port_space.handle_click_impl(center, parent, args, map);
}

int32_t EditorUnsetPortSpaceTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                    EditorInteractive& eia,
                                                    EditorActionArgs* args,
                                                    Map* map) {
	assert(0 <= center.node.x);
	assert(center.node.x < map->get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map->get_height());

	uint32_t nr = 0;

	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		//  check if field is valid
		if (port_tool_nodecaps(mr.location(), *map)) {
			map->set_port_space(eia.egbase(), mr.location(), false);
			Area<FCoords> a(mr.location(), 0);
			map->recalc_for_field_area(eia.egbase(), a);
			++nr;
		}
	} while (mr.advance(*map));

	return nr;
}

int32_t EditorUnsetPortSpaceTool::handle_undo_impl(const NodeAndTriangle<Coords>& center,
                                                   EditorInteractive& parent,
                                                   EditorActionArgs* args,
                                                   Map* map) {
	return parent.tools()->set_port_space.handle_click_impl(center, parent, args, map);
}

Widelands::NodeCaps
EditorUnsetPortSpaceTool::nodecaps_for_buildhelp(const Widelands::FCoords& fcoords,
                                                 const Widelands::EditorGameBase& egbase) {
	return port_tool_nodecaps(fcoords, egbase.map());
}
