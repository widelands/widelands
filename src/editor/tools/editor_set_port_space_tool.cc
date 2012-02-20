/*
 * Copyright (C) 2002-2004, 2006-2012 by the Widelands Development Team
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

#include "editor_set_port_space_tool.h"

#include "logic/building.h"
#include "editor_tool.h"
#include "graphic/graphic.h"
#include "logic/map.h"
#include "logic/mapfringeregion.h"
#include "wui/overlay_manager.h"
#include <editor/editorinteractive.h>

using namespace Widelands;

/// static callback function for overlay calculation
int32_t Editor_Tool_Set_Port_Space_Callback
	(Widelands::TCoords<Widelands::FCoords> const c, void * const data, int32_t)
{
	assert(data);
	Map const & map = *static_cast<Map const *>(data);
	NodeCaps const caps = c.field->nodecaps();
	FCoords f = map.get_fcoords(*c.field);
	if ((caps & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG) {
		MapFringeRegion<Area<FCoords> > mr(map, Area<FCoords>(f, 2));
		do {
			if (mr.location().field->get_caps() & MOVECAPS_SWIM)
				return caps;
		} while (mr.advance(map));
	}
	return 0;
}


Editor_Set_Port_Space_Tool::Editor_Set_Port_Space_Tool
(Editor_Unset_Port_Space_Tool & the_unset_tool)
	:
	Editor_Tool(the_unset_tool, *this),
	m_unset_tool(the_unset_tool)
{}


Editor_Unset_Port_Space_Tool::Editor_Unset_Port_Space_Tool()
	:
	Editor_Tool(*this, *this)
{}


int32_t Editor_Set_Port_Space_Tool::handle_click_impl
	(Map & map,
	Widelands::Node_and_Triangle<> const center,
	Editor_Interactive &,  Editor_Action_Args &)
{
	assert(0 <= center.node.x);
	assert(center.node.x < map.get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map.get_height());

	//  check if field is valid
	if (Editor_Tool_Set_Port_Space_Callback(map.get_fcoords(center.node), &map, 0)) {
		map.set_port_space(map.get_fcoords(center.node), true);
		Area<FCoords> a(map.get_fcoords(center.node), 0);
		map.recalc_for_field_area(a);
		return 1;
	}
	return 0;
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
	Editor_Interactive &, Editor_Action_Args &)
{
	assert(0 <= center.node.x);
	assert(center.node.x < map.get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map.get_height());

	//  check if field is valid
	//  check if field is valid
	if (Editor_Tool_Set_Port_Space_Callback(map.get_fcoords(center.node), &map, 0))
	{
		map.set_port_space(map.get_fcoords(center.node), false);
		Area<FCoords> a(map.get_fcoords(center.node), 0);
		map.recalc_for_field_area(a);
		return 1;
	}
	return 0;
}

int32_t Editor_Unset_Port_Space_Tool::handle_undo_impl
	(Map & map, Node_and_Triangle< Coords > center,
	Editor_Interactive & parent, Editor_Action_Args & args)
{
	return parent.tools.set_port_space.handle_click_impl(map, center, parent, args);
}
