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

#include "editor/tools/editor_set_port_space_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/editor_tool.h"
#include "logic/building.h"
#include "logic/map.h"
#include "logic/mapfringeregion.h"
#include "logic/mapregion.h"
#include "wui/overlay_manager.h"

using namespace Widelands;

/// static callback function for overlay calculation
int32_t Editor_Tool_Set_Port_Space_Callback
	(const Widelands::TCoords<Widelands::FCoords>& c, Map& map)
{
	NodeCaps const caps = c.field->nodecaps();
	FCoords f = map.get_fcoords(*c.field);
	if ((caps & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG) {
		if (!map.find_portdock(f).empty())
			return caps;
	}
	return 0;
}


Editor_Set_Port_Space_Tool::Editor_Set_Port_Space_Tool
(Editor_Unset_Port_Space_Tool & the_unset_tool)
	:
	Editor_Tool(the_unset_tool, *this)
{}


Editor_Unset_Port_Space_Tool::Editor_Unset_Port_Space_Tool()
	:
	Editor_Tool(*this, *this)
{}


int32_t Editor_Set_Port_Space_Tool::handle_click_impl
	(Map & map,
	Widelands::Node_and_Triangle<> const center,
	Editor_Interactive &,  Editor_Action_Args & args)
{
	assert(0 <= center.node.x);
	assert(center.node.x < map.get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map.get_height());

	uint32_t nr = 0;

	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map, Widelands::Area<Widelands::FCoords>(map.get_fcoords(center.node), args.sel_radius));
	do {
		//  check if field is valid
		if (Editor_Tool_Set_Port_Space_Callback(mr.location(), map)) {
			map.set_port_space(mr.location(), true);
			Area<FCoords> a(mr.location(), 0);
			map.recalc_for_field_area(a);
			++nr;
		}
	} while (mr.advance(map));

	return nr;
}

int32_t Editor_Set_Port_Space_Tool::handle_undo_impl
	(Map & map, Node_and_Triangle< Coords > center,
	Editor_Interactive & parent, Editor_Action_Args & args)
{
	return parent.tools.unset_port_space.handle_click_impl(map, center, parent, args);
}


int32_t Editor_Unset_Port_Space_Tool::handle_click_impl
	(Map & map,
	Node_and_Triangle<> const center,
	Editor_Interactive &, Editor_Action_Args & args)
{
	assert(0 <= center.node.x);
	assert(center.node.x < map.get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map.get_height());

	uint32_t nr = 0;

	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map, Widelands::Area<Widelands::FCoords>(map.get_fcoords(center.node), args.sel_radius));
	do {
		//  check if field is valid
		if (Editor_Tool_Set_Port_Space_Callback(mr.location(), map)) {
			map.set_port_space(mr.location(), false);
			Area<FCoords> a(mr.location(), 0);
			map.recalc_for_field_area(a);
			++nr;
		}
	} while (mr.advance(map));

	return nr;
}


int32_t Editor_Unset_Port_Space_Tool::handle_undo_impl
	(Map & map, Node_and_Triangle< Coords > center,
	Editor_Interactive & parent, Editor_Action_Args & args)
{
	return parent.tools.set_port_space.handle_click_impl(map, center, parent, args);
}
