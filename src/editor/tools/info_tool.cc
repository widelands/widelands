/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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
#include "base/i18n.h"
#include "base/string.h"
#include "editor/editorinteractive.h"
#include "editor/ui_menus/field_info_window.h"
#include "graphic/text_layout.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"
#include "wui/unique_window_handler.h"

/// Show a window with information about the pointed at node and triangle.
int32_t EditorInfoTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                          EditorInteractive& parent,
                                          EditorActionArgs* /* args */,
                                          Widelands::Map* map) {

	parent.stop_painting();

	//	UI::Window* const w = new UI::Window(&parent, UI::WindowStyle::kWui, "field_information", 30,
	// 30, 	                                     400, 200, _("Field Information"));
	// UI::MultilineTextarea* const multiline_textarea = 	   new UI::MultilineTextarea(w, 0, 0,
	// w->get_inner_w(), w->get_inner_h(), UI::PanelStyle::kWui);

	Widelands::Field& f = (*map)[center.node];
	Widelands::Field& tf = (*map)[center.triangle.node];

	UI::UniqueWindow::Registry& registry = parent.unique_windows().get_registry(
	   bformat("fieldinfo_%d_%d", center.node.x, center.node.y));
	registry.open_window = [&parent, &registry, &center, &f, &tf, map]() {
		new FieldInfoWindow(parent, registry, _("Field Information"), center, f, tf, map);
	};
	registry.create();
	//		return registry.window;

	return 0;
}
