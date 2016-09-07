/*
 * Copyright (C) 2016 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_DROPDOWN_H
#define WL_UI_BASIC_DROPDOWN_H

#include <map>

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/panel.h"

namespace UI {

/// A dropdown menu that lets the user select a string value.
class Dropdown : public Panel {
public:
	/// \param h determines the size of the list that's shown by the dropdown.
	Dropdown(Panel* parent,
	         int32_t x,
	         int32_t y,
	         uint32_t w,
	         uint32_t h,
	         const std::string& label,
	         bool show_tick = true);

	/// Add an element to the list
	/// \param name         the display name of the entry
	/// \param value        the internal name of the value
	/// \param pic          an image to illustrate the entry
	/// \param select_this  whether this element should be selected
	/// \param tooltip_text a tooltip for this entry
	void add(const std::string& name,
	         const std::string& value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string());

	/// \return true if an element has been selected from the list
	bool has_selection() const;

	/// \return the internal name of the selected element
	const std::string& get_selected() const;

private:
	void set_value();
	void toggle_list();

	UI::Box button_box_;
	UI::Button push_button_;
	UI::Button display_button_;
	UI::Listselect<std::string> list_;
	std::map<std::string, std::string> entries_;
	std::string selected_;
	const std::string label_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_DROPDOWN_H
