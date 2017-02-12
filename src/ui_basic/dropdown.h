/*
 * Copyright (C) 2016-2017 by the Widelands Development Team
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
#include <memory>

#include <boost/signals2.hpp>

#include "graphic/graphic.h"
#include "graphic/image.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/panel.h"

namespace UI {

enum class DropdownType { kTextual, kPictorial };

/// Implementation for a dropdown menu that lets the user select a value.
class BaseDropdown : public Panel {
protected:
	/// \param parent             the parent panel
	/// \param x                  the x-position within 'parent'
	/// \param y                  the y-position within 'parent'
	/// \param list_w             the dropdown's width
	/// \param list_h             the maximum height for the dropdown list
	/// \param button_dimension   the width of the push button in textual dropdowns. For pictorial
	/// dropdowns, this both the width and the height of the button.
	/// \param label              a label to prefix to the selected entry on the display button.
	/// \param type               whether this is a textual or pictorial dropdown
	/// \param background         the background image for this dropdown
	/// \param button_background  the background image all buttons in this dropdown
	BaseDropdown(Panel* parent,
	             int32_t x,
	             int32_t y,
	             uint32_t list_w,
	             uint32_t list_h,
	             int button_dimension,
	             const std::string& label,
	             const DropdownType type,
	             const Image* background,
	             const Image* button_background);
	~BaseDropdown();

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

	/// Whether the dropdown selection is enabled.
	bool is_enabled() const {
		return is_enabled_;
	}

	/// Whether the dropdown has been opened by the user.
	bool is_expanded() const;

	/// Move the dropdown. The dropdown's position is relative to the parent in
	/// pixels.
	void set_pos(Vector2i point) override;

	/// The number of elements listed in the dropdown.
	uint32_t size() const;

	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

	void set_height(int height);

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

	/// Select the entry. Assumes that it exists. Does not trigger the 'selected' signal.
	void select(uint32_t entry);

	/// Removes all elements from the list.
	void clear();

	/// Automatically collapses the list if the mouse gets too far away from the dropdown, or if it
	/// loses focus.
	void think() override;

private:
	void layout() override;

	/// Updates the buttons
	void update();

	/// Updates the title and tooltip of the display button and triggers a 'selected' signal.
	void set_value();
	/// Toggles the dropdown list on and off.
	void toggle_list();

	/// Returns true if the mouse pointer left the vicinity of the dropdown.
	bool is_mouse_away() const;

	int max_list_height_;
	int list_width_;
	int button_dimension_;
	const int mouse_tolerance_;  // Allow mouse outside the panel a bit before autocollapse
	UI::Box button_box_;
	UI::Button* push_button_;  // Only used in textual dropdowns
	UI::Button display_button_;
	// The list needs to be a pointer for destruction, because we hook into the paren't parent.
	UI::Listselect<uintptr_t>* list_;
	std::string label_;
	std::string tooltip_;
	uint32_t current_selection_;
	DropdownType type_;
	bool is_enabled_;
};

/// A dropdown menu that lets the user select a value of the datatype 'Entry'.
template <typename Entry> class Dropdown : public BaseDropdown {
public:
	/// \param parent             the parent panel
	/// \param x                  the x-position within 'parent'
	/// \param y                  the y-position within 'parent'
	/// \param list_w             the dropdown's width
	/// \param list_h             the maximum height for the dropdown list
	/// \param button_dimension   the width of the push button in textual dropdowns. For pictorial
	/// dropdowns, this both the width and the height of the button.
	/// \param label              a label to prefix to the selected entry on the display button.
	/// \param type               whether this is a textual or pictorial dropdown
	/// \param background         the background image for this dropdown
	/// \param button_background  the background image all buttons in this dropdown
	Dropdown(Panel* parent,
	         int32_t x,
	         int32_t y,
	         uint32_t list_w,
	         uint32_t list_h,
	         int button_dimension,
	         const std::string& label,
	         const DropdownType type = DropdownType::kTextual,
	         const Image* background = g_gr->images().get("images/ui_basic/but1.png"),
	         const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"))
	   : BaseDropdown(parent,
	                  x,
	                  y,
	                  list_w,
	                  list_h,
	                  button_dimension,
	                  label,
	                  type,
	                  background,
	                  button_background) {
	}
	~Dropdown() {
		entry_cache_.clear();
	}

	/// Add an element to the list
	/// \param name         the display name of the entry
	/// \param value        the value for the entry
	/// \param pic          an image to illustrate the entry. Can be nullptr in textual dropdowns
	/// only.
	/// \param select_this  whether this element should be selected
	/// \param tooltip_text a tooltip for this entry
	void add(const std::string& name,
	         Entry value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string()) {
		entry_cache_.push_back(std::unique_ptr<Entry>(new Entry(value)));
		BaseDropdown::add(name, size(), pic, select_this, tooltip_text);
	}

	/// \return the selected element
	const Entry& get_selected() const {
		return *entry_cache_[BaseDropdown::get_selected()];
	}

	/// Select the entry if it exists. Does not trigger the 'selected' signal.
	void select(const Entry& entry) {
		for (uint32_t i = 0; i < entry_cache_.size(); ++i) {
			if (entry == *entry_cache_[i]) {
				BaseDropdown::select(i);
			}
		}
	}

	/// Removes all elements from the list.
	void clear() {
		BaseDropdown::clear();
		entry_cache_.clear();
	}

private:
	// Contains the actual elements. The BaseDropdown registers the indices only.
	std::deque<std::unique_ptr<Entry>> entry_cache_;
};

}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_DROPDOWN_H
