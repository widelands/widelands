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

#include <deque>

#include <boost/signals2.hpp>

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/panel.h"

namespace UI {

/// Implementation for a dropdown menu that lets the user select a value.
class BaseDropdown : public Panel {
protected:
	/// \param parent     the parent panel
	/// \param x          the x-position within 'parent'
	/// \param y          the y-position within 'parent'
	/// \param w          the dropdown's width
	/// \param h          the maximum height for the dropdown list
	/// \param label      a label to prefix to the selected entry on the display button.
	/// \param show_tick  if 'true', the dropdown list will show a tick for the currently selected
	///                   entry.
	BaseDropdown(Panel* parent,
	             int32_t x,
	             int32_t y,
	             uint32_t w,
	             uint32_t h,
	             const std::string& label,
	             bool show_tick = true);
	~BaseDropdown() {
		clear();
	}

public:
	boost::signals2::signal<void()> selected;

	/// \return true if an element has been selected from the list
	bool has_selection() const;

	/// Sets a label that will be prefixed to the currently selected element's name
	/// and displayed on the display button.
	void set_label(const std::string& text);
	/// Sets the tooltip for the display button.
	void set_tooltip(const std::string& text);
	/// Enables/disables the dropdown selection.
	void set_enabled(bool on);
	/// Move the dropdown. The dropdown's position is relative to the parent.
	void set_pos(Point point) override;

	/// The number of elements listed in the dropdown.
	uint32_t size() const;

	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

protected:
	/// Add an element to the list
	/// \param name         the display name of the entry
	/// \param value        the index of the entry
	/// \param pic          an image to illustrate the entry
	/// \param select_this  whether this element should be selected
	/// \param tooltip_text a tooltip for this entry
	void add(const std::string& name,
	         uint32_t value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string());

	/// \return the index of the selected element
	uint32_t get_selected() const;

	/// Removes all elements from the list.
	void clear();

	/// Automatically collapses the list if the mouse gets too far away from the dropdown, or if it
	/// loses focus.
	void think() override;

private:
	/// Updates the title and tooltip of the display button and triggers a 'selected' signal.
	void set_value();
	/// Toggles the dropdown list on and off.
	void toggle_list();

	uint32_t max_list_height_;
	const int mouse_tolerance_;  // Allow mouse outside the panel a bit before autocollapse
	UI::Box button_box_;
	UI::Button push_button_;
	UI::Button display_button_;
	UI::Listselect<uintptr_t> list_;
	std::string label_;
	std::string tooltip_;
	uint32_t current_selection_;
};

/// A dropdown menu that lets the user select a value of the datatype 'Entry'.
template <typename Entry> class Dropdown : public BaseDropdown {
public:
	/// \param parent     the parent panel
	/// \param x          the x-position within 'parent'
	/// \param y          the y-position within 'parent'
	/// \param w          the dropdown's width
	/// \param h          the maximum height for the dropdown list
	/// \param label      a label to prefix to the selected entry on the display button.
	/// \param show_tick  if 'true', the dropdown list will show a tick for the currently selected
	///                   entry.
	Dropdown(Panel* parent,
	         int32_t x,
	         int32_t y,
	         uint32_t w,
	         uint32_t h,
	         const std::string& label,
	         bool show_tick = true)
	   : BaseDropdown(parent, x, y, w, h, label, show_tick) {
	}
	~Dropdown() {
		clear();
	}

	/// Add an element to the list
	/// \param name         the display name of the entry
	/// \param value        the value for the entry
	/// \param pic          an image to illustrate the entry
	/// \param select_this  whether this element should be selected
	/// \param tooltip_text a tooltip for this entry
	void add(const std::string& name,
	         Entry value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string()) {
		entry_cache_.push_back(new Entry(value));
		BaseDropdown::add(name, size(), pic, select_this, tooltip_text);
	}

	/// \return the selected element
	const Entry& get_selected() const {
		return *entry_cache_[BaseDropdown::get_selected()];
	}

	/// Removes all elements from the list.
	void clear() {
		BaseDropdown::clear();
		for (Entry* entry : entry_cache_) {
			delete entry;
		}
		entry_cache_.clear();
	}

private:
	// Contains the actual elements. The BaseDropdown registers the indices only.
	std::deque<Entry*> entry_cache_;
};

}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_DROPDOWN_H
