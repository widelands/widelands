/*
 * Copyright (C) 2003-2022 by the Widelands Development Team
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

#include "ui_basic/icongrid.h"

#include "ui_basic/button.h"

namespace UI {

struct IconGridButton : public Button {
	IconGridButton(IconGrid& parent,
	               const std::string& name,
	               int32_t x,
	               int32_t y,
	               uint32_t w,
	               uint32_t h,
	               const Image* foreground_picture_id,
	               uint32_t callback_argument_id,
	               const std::string& tooltip_text)
	   : Button(&parent,
	            name,
	            x,
	            y,
	            w,
	            h,
	            UI::ButtonStyle::kWuiBuildingStats,
	            foreground_picture_id,
	            tooltip_text,
	            UI::Button::VisualState::kFlat),
	     icongrid_(parent),
	     callback_argument_id_(callback_argument_id) {
	}

private:
	IconGrid& icongrid_;
	const uint32_t callback_argument_id_;

	void handle_mousein(bool inside) override {
		if (inside) {
			icongrid_.mousein(callback_argument_id_);
		} else {
			icongrid_.mouseout(callback_argument_id_);
		}
		Button::handle_mousein(inside);
	}
};

/**
 * Initialize the grid
 */
IconGrid::IconGrid(Panel* const parent,
                   PanelStyle s,
                   int32_t x,
                   int32_t y,
                   int32_t cellw,
                   int32_t cellh,
                   int32_t cols)
   : Panel(parent, s, x, y, 0, 0), columns_(cols), cell_width_(cellw), cell_height_(cellh) {
}

/**
 * Add a new icon to the list and resize appropriately.
 * Returns the index of the newly added icon.
 */
int32_t IconGrid::add(const std::string& name,
                      const Image* pic,
                      void* data,
                      const std::string& tooltip_text) {
	Item it;

	it.data = data;

	items_.push_back(it);

	// resize
	const int32_t rows = (items_.size() + columns_ - 1) / columns_;

	if (rows <= 1) {
		set_desired_size(cell_width_ * columns_, cell_height_);
	} else {
		set_desired_size(cell_width_ * columns_, cell_height_ * rows);
	}

	uint32_t idx = items_.size() - 1;
	uint32_t x = (idx % columns_) * cell_width_;
	uint32_t y = (idx / columns_) * cell_height_;

	UI::Button* btn =
	   new IconGridButton(*this, name, x, y, cell_width_, cell_height_, pic, idx, tooltip_text);
	btn->sigclicked.connect([this, idx]() { clicked_button(idx); });

	return idx;
}

void IconGrid::clicked_button(uint32_t idx) {
	icon_clicked(idx);
	play_click();
}

/**
 * Returns the user-defined data of the icon with the given index.
 */
void* IconGrid::get_data(int32_t idx) {
	assert(static_cast<uint32_t>(idx) < items_.size());

	return items_[idx].data;
}
}  // namespace UI
