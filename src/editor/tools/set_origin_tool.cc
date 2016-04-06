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

#include "editor/tools/set_origin_tool.h"

#include "editor/editorinteractive.h"
#include "logic/map.h"
#include "wui/mapviewpixelconstants.h"

int32_t EditorSetOriginTool::handle_click_impl(const Widelands::World&,
                                               Widelands::NodeAndTriangle<> const center,
                                               EditorInteractive& eia,
                                               EditorActionArgs* /* args */,
											   Widelands::Map* map) {
	map->set_origin(center.node);
	eia.map_changed(EditorInteractive::MapWas::kGloballyMutated);
	eia.set_rel_viewpoint
	(Point
	 (-(center.node.x * 2 + (center.node.y & 1)) * (kTriangleWidth / 2),
	  - center.node.y *                             kTriangleHeight),
	 true);
	return 0;
}

int32_t
EditorSetOriginTool::handle_undo_impl(const Widelands::World&,
                                      Widelands::NodeAndTriangle<Widelands::Coords> center,
                                      EditorInteractive& eia,
                                      EditorActionArgs* /* args */,
									  Widelands::Map* map) {
	Widelands::Coords nc
		(map->get_width()  - center.node.x,
		 map->get_height() - center.node.y);
	map->set_origin(nc);
	eia.map_changed(EditorInteractive::MapWas::kGloballyMutated);
	eia.set_rel_viewpoint
	(Point
	 (- (nc.x * 2 + (nc.y & 1)) *(kTriangleWidth / 2),
	  - nc.y * kTriangleHeight),
	 true);
	return 0;
}

EditorActionArgs EditorSetOriginTool::format_args_impl(EditorInteractive & eia)
{
	return EditorTool::format_args_impl(eia);
}
