/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "editor/tools/info_tool.h"

#include "editor/ui_menus/field_info_window.h"
#include "wui/unique_window_handler.h"

constexpr int kOffset = 30;
/// Show a window with information about the pointed at node and triangle.
int32_t EditorInfoTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                          EditorInteractive& parent,
                                          EditorActionArgs* /* args */,
                                          Widelands::Map* map) {

	parent.stop_painting();

	Widelands::Field& f = (*map)[center.node];
	Widelands::Field& tf = (*map)[center.triangle.node];
	const Widelands::Coords& coords = center.node;

	UI::UniqueWindow::Registry& registry =
	   parent.unique_windows().get_registry(format("fieldinfo_%d_%d", coords.x, coords.y));

	registry.open_window = [this, &parent, &registry, &center, &f, &tf, map]() {
		// if window reaches bottom right corner, start from top left corner again
		int a = (parent.get_inner_w() - FieldInfoWindow::total_width) / kOffset;
		int b = (parent.get_inner_h() - FieldInfoWindow::total_height) / kOffset;
		int offset_factor = number_of_open_windows_ % std::min(a, b);
		int offset = offset_factor * kOffset;

		new FieldInfoWindow(parent, registry, offset, offset, center, f, tf, map);
	};

	cached_subscribers_opened_[coords] =
	   registry.opened.subscribe([this]() { ++number_of_open_windows_; });

	cached_subscribers_closed_[coords] =
	   registry.closed.subscribe([this]() { --number_of_open_windows_; });

	registry.create();
	return 0;
}
