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

EditorToolResizeOptionsMenu::EditorToolResizeOptionsMenu(
   EditorInteractive& parent,
   EditorResizeTool& resize_tool,
   UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 260, 200, _("Resize")),
     resize_tool_(resize_tool),
     box_(this, hmargin(), vmargin(), UI::Box::Vertical, 0, 0, vspacing()),
     new_width_(&box_,
                0,
                0,
                get_inner_w() - 2 * hmargin(),
                80,
                0,
                1,
                1000,
                UI::PanelStyle::kWui,
                _("New width:"),
                UI::SpinBox::Units::kNone,
                UI::SpinBox::Type::kValueList),
     new_height_(&box_,
             0,
             0,
             get_inner_w() - 2 * hmargin(),
             80,
             0,
             1,
             1000,
             UI::PanelStyle::kWui,
             _("New height:"),
             UI::SpinBox::Units::kNone,
             UI::SpinBox::Type::kValueList) {
	// Configure spin boxes
	new_width_.set_value_list(Widelands::kMapDimensions);
	new_height_.set_value_list(Widelands::kMapDimensions);
	{
		size_t width_index, height_index;
		Widelands::Extent const map_extent = parent.egbase().map().extent();
		for (width_index = 0; width_index < Widelands::kMapDimensions.size() &&
		                      Widelands::kMapDimensions[width_index] < map_extent.w;
		     ++width_index) {
		}
		new_width_.set_value(width_index);

		for (height_index = 0; height_index < Widelands::kMapDimensions.size() &&
		                       Widelands::kMapDimensions[height_index] < map_extent.h;
		     ++height_index) {
		}
		new_height_.set_value(height_index);
	}

	new_width_.changed.connect(
	   boost::bind(&EditorToolResizeOptionsMenu::update_width, boost::ref(*this)));
	new_height_.changed.connect(
	   boost::bind(&EditorToolResizeOptionsMenu::update_height, boost::ref(*this)));

	box_.add(&new_width_);
	box_.add(&new_height_);

	box_.set_size(get_inner_w() - 2 * hmargin(), new_width_.get_h() + new_height_.get_h() + vspacing());
	set_inner_size(get_inner_w(), box_.get_h() + 1 * vmargin());
}

void EditorToolResizeOptionsMenu::update_width() {
	int32_t w = new_width_.get_value();
	assert(w > 0);
	resize_tool_.set_width(w);
	select_correct_tool();
}

void EditorToolResizeOptionsMenu::update_height() {
	int32_t h = new_height_.get_value();
	assert(h > 0);
	resize_tool_.set_height(h);
	select_correct_tool();
}

