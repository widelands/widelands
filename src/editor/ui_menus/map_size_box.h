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

#ifndef WL_EDITOR_UI_MENUS_MAP_SIZE_BOX_H
#define WL_EDITOR_UI_MENUS_MAP_SIZE_BOX_H

#include "ui_basic/box.h"
#include "ui_basic/dropdown.h"

/**
 * A box containing 2 dropdowns to select map width and height with horizontal layout.
 * Selections are taken from Widelands::kMapDimensions.
 */
struct MapSizeBox : public UI::Box {

	/**
	 * @param parent The parent panel
	 * @param name A string to prefix for the dropdown names, so that they can be idenfitied uniquely
	 * @param spacing The horizontal space between the 2 dropdowns
	 * @param map_width Width to preselect
	 * @param map_height Height to preselect
	 */
	MapSizeBox(UI::Box& parent,
	           UI::PanelStyle,
	           const std::string& name,
	           int spacing,
	           int map_width,
	           int map_height);

	/// This function will be triggered when a new width or height is selected from the dropdowns
	void set_selection_function(const std::function<void()>& func);
	/// The currently selected width
	uint32_t selected_width() const;
	/// The currently selected height
	uint32_t selected_height() const;
	/// Set the selected width
	void select_width(int new_width);
	/// Set the selected height
	void select_height(int new_height);

private:
	UI::Dropdown<uint32_t> width_;
	UI::Dropdown<uint32_t> height_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAP_SIZE_BOX_H
