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

#include "editor/ui_menus/tool_resize_options_menu.h"

#include "base/i18n.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "editor/tools/resize_tool.h"
#include "logic/map.h"

inline EditorInteractive& EditorToolResizeOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

EditorToolResizeOptionsMenu::EditorToolResizeOptionsMenu(EditorInteractive& parent,
                                                         EditorResizeTool& resize_tool,
                                                         UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 260, 200, _("Resize"), resize_tool),
     resize_tool_(resize_tool),
     box_(this, UI::PanelStyle::kWui, hmargin(), vmargin(), UI::Box::Vertical, 0, 0, vspacing()),
     map_size_box_(box_,
                   UI::PanelStyle::kWui,
                   "tool_resize_map",
                   4,
                   parent.egbase().map().get_width(),
                   parent.egbase().map().get_height()),
     text_area_(
        &box_,
        0,
        0,
        get_inner_w() - 2 * hmargin(),
        48,
        UI::PanelStyle::kWui,
        _("Select the new map size, then click the map to split it at the desired location. When "
          "increasing the map size, new columns/rows will be inserted to the east/south of the "
          "selected point. When shrinking the map, the desired number of columns/rows will be "
          "removed from the east/south of the splitting location."),
        UI::Align::kCenter,
        UI::MultilineTextarea::ScrollMode::kNoScrolling) {

	map_size_box_.set_selection_function([this] { update_dimensions(); });

	box_.add(&map_size_box_, UI::Box::Resizing::kExpandBoth);
	box_.add(&text_area_, UI::Box::Resizing::kFullSize);

	set_center_panel(&box_);

	initialization_complete();
}

void EditorToolResizeOptionsMenu::update_dimensions() {
	const int32_t w = map_size_box_.selected_width();
	const int32_t h = map_size_box_.selected_height();
	assert(w > 0);
	assert(h > 0);
	resize_tool_.set_width(w);
	resize_tool_.set_height(h);
	select_correct_tool();
}
