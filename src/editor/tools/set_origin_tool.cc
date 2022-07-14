/*
 * Copyright (C) 2009-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "editor/tools/set_origin_tool.h"

#include "editor/editorinteractive.h"

int32_t EditorSetOriginTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                               EditorActionArgs* /* args */,
                                               Widelands::Map* map) {
	map->set_origin(center.node);
	parent_.map_changed(EditorInteractive::MapWas::kGloballyMutated);
	parent_.map_view()->scroll_to_field(Widelands::Coords(0, 0), MapView::Transition::Jump);
	return 0;
}

int32_t
EditorSetOriginTool::handle_undo_impl(const Widelands::NodeAndTriangle<Widelands::Coords>& center,
                                      EditorActionArgs* /* args */,
                                      Widelands::Map* map) {
	Widelands::Coords nc(map->get_width() - center.node.x, map->get_height() - center.node.y);

	// Because of the triangle design of map, y is changed by an odd number.
	// The x must be syncronized with the y when coordinate pair is applied
	// and also when undoing an action like here.
	if ((nc.y % 2) != 0) {
		nc.x = nc.x - 1;
	}
	map->normalize_coords(nc);
	map->set_origin(nc);
	parent_.map_changed(EditorInteractive::MapWas::kGloballyMutated);
	parent_.map_view()->scroll_to_field(Widelands::Coords(0, 0), MapView::Transition::Jump);
	return 0;
}

EditorActionArgs EditorSetOriginTool::format_args_impl() {
	return EditorTool::format_args_impl();
}
