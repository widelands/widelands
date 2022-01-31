/*
 * Copyright (C) 2019-2022 by the Widelands Development Team
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

#include "editor/ui_menus/map_size_box.h"

#include "base/i18n.h"
#include "logic/map.h"

MapSizeBox::MapSizeBox(UI::Box& parent,
                       UI::PanelStyle s,
                       const std::string& name,
                       int spacing,
                       int map_width,
                       int map_height)
   : UI::Box(&parent, s, 0, 0, UI::Box::Horizontal, 0, 0, spacing),
     width_(this,
            name + "_map_width",
            0,
            0,
            160,
            12,
            24,
            _("Width"),
            UI::DropdownType::kTextual,
            s,
            s == UI::PanelStyle::kWui ? UI::ButtonStyle::kWuiSecondary :
                                        UI::ButtonStyle::kFsMenuSecondary),
     height_(this,
             name + "_map_height",
             0,
             0,
             160,
             12,
             24,
             _("Height"),
             UI::DropdownType::kTextual,
             s,
             s == UI::PanelStyle::kWui ? UI::ButtonStyle::kWuiSecondary :
                                         UI::ButtonStyle::kFsMenuSecondary) {
	for (const int32_t& i : Widelands::kMapDimensions) {
		width_.add(std::to_string(i), i);
		height_.add(std::to_string(i), i);
	}
	width_.select(map_width);
	height_.select(map_height);
	add(&width_, UI::Box::Resizing::kFillSpace);
	add(&height_, UI::Box::Resizing::kFillSpace);
}

void MapSizeBox::set_selection_function(const std::function<void()>& func) {
	width_.selected.connect(func);
	height_.selected.connect(func);
}

uint32_t MapSizeBox::selected_width() const {
	return width_.get_selected();
}
uint32_t MapSizeBox::selected_height() const {
	return height_.get_selected();
}

void MapSizeBox::select_width(int new_width) {
	width_.select(new_width);
}
void MapSizeBox::select_height(int new_height) {
	height_.select(new_height);
}
