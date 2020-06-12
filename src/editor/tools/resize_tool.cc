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

#include "editor/tools/resize_tool.h"

#include "editor/editorinteractive.h"
#include "logic/widelands_geometry.h"

int32_t EditorResizeTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                            EditorInteractive& eia,
                                            EditorActionArgs* args,
                                            Widelands::Map* map) {
	args->resized = map->dump_state(eia.egbase());  // save old state for undo
	map->resize(eia.egbase(), center.node, args->new_map_size.w, args->new_map_size.h);

	// fix for issue #3754 (remove selection markers from deleted fields to prevent a crash)
	Widelands::NodeAndTriangle<> sel = eia.get_sel_pos();
	map->normalize_coords(sel.node);
	map->normalize_coords(sel.triangle.node);
	eia.set_sel_pos(sel);

	return 0;
}

int32_t EditorResizeTool::handle_undo_impl(const Widelands::NodeAndTriangle<Widelands::Coords>&,
                                           EditorInteractive& eia,
                                           EditorActionArgs* args,
                                           Widelands::Map* map) {
	map->set_to(eia.egbase(), args->resized);

	// fix for issue #3754 (same as above)
	Widelands::NodeAndTriangle<> sel = eia.get_sel_pos();
	map->normalize_coords(sel.node);
	map->normalize_coords(sel.triangle.node);
	eia.set_sel_pos(sel);

	return 0;
}

EditorActionArgs EditorResizeTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.new_map_size = Widelands::Extent(width_, height_);
	return a;
}
