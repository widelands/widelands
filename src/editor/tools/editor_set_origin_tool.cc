/*
 * Copyright (C) 2009, 2012 by the Widelands Development Team
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

#include "editor/tools/editor_set_origin_tool.h"

#include "editor/editorinteractive.h"
#include "logic/map.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/overlay_manager.h"

int32_t Editor_Set_Origin_Tool::handle_click_impl
	(Widelands::Map           &          map,
	Widelands::Node_and_Triangle<> const center,
	Editor_Interactive        &          eia,
	Editor_Action_Args        &          /* args */)
{
	map.set_origin(center.node);
	eia.register_overlays();
	eia.set_rel_viewpoint
	(Point
	 (-(center.node.x * 2 + (center.node.y & 1)) * (TRIANGLE_WIDTH / 2),
	  - center.node.y *                             TRIANGLE_HEIGHT),
	 true);
	return 0;
}

int32_t Editor_Set_Origin_Tool::handle_undo_impl
	(Widelands::Map & map, Widelands::Node_and_Triangle< Widelands::Coords > center,
	Editor_Interactive & parent, Editor_Action_Args & /* args */)
{
	Widelands::Coords nc
		(map.get_width()  - center.node.x,
		 map.get_height() - center.node.y);
	map.set_origin(nc);
	parent.register_overlays();
	parent.set_rel_viewpoint
	(Point
	 (- (nc.x * 2 + (nc.y & 1)) *(TRIANGLE_WIDTH / 2),
	  - nc.y * TRIANGLE_HEIGHT),
	 true);
	return 0;
}

Editor_Action_Args Editor_Set_Origin_Tool::format_args_impl(Editor_Interactive & parent)
{
	return Editor_Tool::format_args_impl(parent);
}
