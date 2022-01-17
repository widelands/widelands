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

#ifndef WL_UI_BASIC_ICONGRID_H
#define WL_UI_BASIC_ICONGRID_H

#include "ui_basic/panel.h"

namespace UI {

/**
 * Arranges clickable pictures of common size in a regular grid.
 *
 * Arrangement is horizontal (pictures fill the grid from left to right, top to
 * bottom).
 */
struct IconGrid : public Panel {
	IconGrid(
	   Panel* parent, PanelStyle, int32_t x, int32_t y, int32_t cellw, int32_t cellh, int32_t cols);

	Notifications::Signal<int32_t> icon_clicked;
	Notifications::Signal<int32_t> mouseout;
	Notifications::Signal<int32_t> mousein;

	int32_t
	add(const std::string& name, const Image* pic, void* data, const std::string& tooltip_text = "");
	void* get_data(int32_t idx);

private:
	void clicked_button(uint32_t);

	struct Item {
		void* data;
	};

	/// max # of columns (or rows, depending on orientation) in the grid
	int32_t columns_;
	int32_t cell_width_;  ///< size of one cell
	int32_t cell_height_;

	std::vector<Item> items_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_ICONGRID_H
