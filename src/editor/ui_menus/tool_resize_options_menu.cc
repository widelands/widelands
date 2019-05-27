/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include <cstdio>
#include <string>

#include "base/i18n.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "editor/tools/resize_tool.h"
#include "graphic/graphic.h"
#include "logic/map.h"

inline EditorInteractive& EditorToolResizeOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

EditorToolResizeOptionsMenu::EditorToolResizeOptionsMenu(EditorInteractive& parent,
                                                         EditorResizeTool& resize_tool,
                                                         UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 260, 200, _("Resize")),
     resize_tool_(resize_tool),
     box_(this, hmargin(), vmargin(), UI::Box::Vertical, 0, 0, vspacing()),
     new_width_(&box_,
                0,
                0,
                get_inner_w() - 2 * hmargin(),
                200,
                24,
                _("New width"),
                UI::DropdownType::kTextual,
                UI::PanelStyle::kWui),
     new_height_(&box_,
                 0,
                 0,
                 get_inner_w() - 2 * hmargin(),
                 200,
                 24,
                 _("New height"),
                 UI::DropdownType::kTextual,
                 UI::PanelStyle::kWui),
     text_area_(
        &box_,
        0,
        0,
        get_inner_w() - 2 * hmargin(),
        48,
        UI::PanelStyle::kWui,
        _("Select the new map size, then click the map to split it at the desired location."),
        UI::Align::kCenter,
        UI::MultilineTextarea::ScrollMode::kNoScrolling) {

	for (const int32_t& i : Widelands::kMapDimensions) {
		new_width_.add(std::to_string(i), i);
		new_height_.add(std::to_string(i), i);
	}
	new_width_.select(parent.egbase().map().get_width());
	new_height_.select(parent.egbase().map().get_height());
	new_width_.set_max_items(8);
	new_height_.set_max_items(8);

	new_width_.selected.connect(
	   boost::bind(&EditorToolResizeOptionsMenu::update_width, boost::ref(*this)));
	new_height_.selected.connect(
	   boost::bind(&EditorToolResizeOptionsMenu::update_height, boost::ref(*this)));

	box_.add(&text_area_);
	box_.set_size(100, 20);  // Prevent assert failures
	box_.add(&new_width_, UI::Box::Resizing::kFullSize);
	box_.add(&new_height_, UI::Box::Resizing::kFullSize);

	box_.set_size(get_inner_w() - 2 * hmargin(),
	              new_width_.get_h() + new_height_.get_h() + text_area_.get_h() + 2 * vspacing());
	set_inner_size(get_inner_w(), box_.get_h() + 1 * vmargin());
}

void EditorToolResizeOptionsMenu::update_width() {
	int32_t w = new_width_.get_selected();
	assert(w > 0);
	resize_tool_.set_width(w);
	select_correct_tool();
}

void EditorToolResizeOptionsMenu::update_height() {
	int32_t h = new_height_.get_selected();
	assert(h > 0);
	resize_tool_.set_height(h);
	select_correct_tool();
}
