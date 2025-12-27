/*
 * Copyright (C) 2025 by the Widelands Development Team
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

#include "editor/tools/picker_tool.h"

#include "editor/ui_menus/tool_options_menu.h"

int32_t EditorPickerTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                            EditorActionArgs* /* args */,
                                            Widelands::Map* map) {
	if (linked_tool_window_ != nullptr) {
		if (linked_tool_window_->pick_from_field(*map, center)) {
			linked_tool_window_ = nullptr;
		}
	}
	return 0;
}
