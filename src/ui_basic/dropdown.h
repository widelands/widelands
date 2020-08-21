/*
 * Copyright (C) 2016-2020 by the Widelands Development Team
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

#include "graphic/graphic.h"
#include "graphic/image.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/panel.h"

namespace UI {
// We use this to make sure that only 1 dropdown is open at the same time.
struct NoteDropdown {
	CAN_BE_SENT_AS_NOTE(NoteId::Dropdown)

	int id;

	explicit NoteDropdown(int init_id) : id(init_id) {
	}
};

/// The narrow textual dropdown omits the extra push button.
/// Use kPictorialMenu if you want to trigger an action without changing the menu button.
enum class DropdownType { kTextual, kTextualNarrow, kPictorial, kPictorialMenu, kTextualMenu };

/// Implementation for a dropdown menu that lets the user select a value.
class BaseDropdown : public NamedPanel {
protected:
	/// \param parent             the parent panel
	/// \param name               a name so that we can reference the dropdown via Lua
	/// \param x                  the x-position within 'parent'
	/// \param y                  the y-position within 'parent'
	/// \param list_w             the dropdown's width
	/// \param max_list_items     the maximum number of items shown in the list before it starts
	/// using a scrollbar \param button_dimension   the width of the push button in textual
	/// dropdowns. For pictorial dropdowns, this is both the width and the height of the button.
	/// \param label              a label to prefix to the selected entry on the display button.
	/// \param type               whether this is a textual or pictorial dropdown
	/// \param style              the style used for buttons and background
	BaseDropdown(Panel* parent,
	             const std::string& name,
	             int32_t x,
	             int32_t y,
	             uint32_t list_w,
	             uint32_t max_list_items,
	             int button_dimension,
	             const std::string& label,
	             const DropdownType type,
	             PanelStyle style,
	             ButtonStyle button_style);
	~BaseDropdown() override;

public:
	/// An entry was selected
	boost::signals2::signal<void()> selected;

	/// \return true if an element has been selected from the list
	bool has_selection() const;

	/// Sets a label that will be prefixed to the currently selected element's name
	/// and displayed on the display button.
	void set_label(const std::string& text);

	/// Sets the image for the display button (for pictorial dropdowns).
	void set_image(const Image* image);

	/// Sets the tooltip for the display button.
	void set_tooltip(const std::string& text);

	/// Displays an error message on the button instead of the current selection.
	void set_errored(const std::string& error_message);

	/// Enables/disables the dropdown selection.
	void set_enabled(bool on);

	/// Whether the dropdown selection is enabled.
	bool is_enabled() const {
		return is_enabled_;
	}

	/// Which visual style to use for disabled pictorial dropdowns.
	void set_disable_style(UI::ButtonDisableStyle disable_style);

	/// Whether the dropdown has no elements to select.
	bool empty() {
		return size() == 0;
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

	/// Set maximum available height in the UI
	void set_height(int height);

	/// Toggle the list on and off and position the mouse on the button so that the dropdown won't
	/// close on us. If this is a menu and nothing was selected yet, select the first item for easier
	/// keyboard navigation.
	void toggle();

	/// If 'open', show the list and position the mouse on the button so that the dropdown won't
	/// close on us. If this is a menu and nothing was selected yet, select the first item for easier
	/// keyboard navigation. If not 'open', close the list.
	void set_list_visibility(bool open);

	void set_size(int nw, int nh) override;
	void set_desired_size(int w, int h) override;

	/// Expand display button to make enough room for each entry's text. Call this before adding any
	/// entries.
	void set_autoexpand_display_button();

	void notify_list_deleted() {
		list_ = nullptr;
	}

protected:
	/// Add an element to the list
	/// \param name         the display name of the entry
	/// \param value        the index of the entry
	/// \param pic          an image to illustrate the entry. Can be nullptr for textual dropdowns.
	/// \param select_this  whether this element should be selected
	/// \param tooltip_text a tooltip for this entry
	/// \param hotkey       a hotkey tip if any
	///
	/// Text conventions: Title Case for the 'name', Sentence case for the 'tooltip_text'
	void add(const std::string& name,
	         uint32_t value,
	         const Image* pic,
	         const bool select_this,
	         const std::string& tooltip_text,
	         const std::string& hotkey);

	/// \return the index of the selected element
	uint32_t get_selected() const;

	/// Select the entry. Assumes that it exists. Does not trigger the 'selected' signal.
	void select(uint32_t entry);

	/// Removes all elements from the list.
	void clear();

	/// Automatically collapses the list if the mouse gets too far away from the dropdown, or if it
	/// loses focus.
	void think() override;

	UI::Panel* get_open_dropdown() override;

private:
	static void layout_if_alive(int);
	void layout() override;

	/// Updates the buttons
	void update();

	/// Updates the title and tooltip of the display button and triggers a 'selected' signal.
	void set_value();
	/// Toggles the dropdown list on and off and sends a notification if the list is visible
	/// afterwards.
	void toggle_list();
	/// Toggle the list closed if the dropdown is currently expanded.
	void close();

	/// Returns true if the mouse pointer left the vicinity of the dropdown.
	bool is_mouse_away() const;

	/// Give each dropdown a unique ID
	static int next_id_;
	const int id_;
	std::unique_ptr<Notifications::Subscriber<NoteDropdown>> dropdown_subscriber_;
	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;

	// Dimensions
	unsigned int max_list_items_;
	unsigned int max_list_height_;
	int list_offset_x_;
	int list_offset_y_;
	const int base_height_;
	const int mouse_tolerance_;  // Allow mouse outside the panel a bit before autocollapse
	UI::Box button_box_;
	UI::Button* push_button_;  // Only used in textual dropdowns
	UI::Button display_button_;
	// The list needs to be a pointer for destruction, because we hook into the highest parent that
	// we can get.
	UI::Listselect<uintptr_t>* list_;
	std::string label_;
	std::string tooltip_;
	uint32_t current_selection_;
	DropdownType type_;
	bool is_enabled_;
	ButtonStyle button_style_;
	bool autoexpand_display_button_;
};

/// A dropdown menu that lets the user select a value of the datatype 'Entry'.
template <typename Entry> class Dropdown : public BaseDropdown {
public:
	/// \param parent             the parent panel
	/// \param name               a name so that we can reference the dropdown via Lua
	/// \param x                  the x-position within 'parent'
	/// \param y                  the y-position within 'parent'
	/// \param list_w             the dropdown's width
	/// \param max_list_items     the maximum number of items shown in the list before it starts
	/// using a scrollbar \param button_dimension   the width of the push button in textual
	/// dropdowns. For pictorial dropdowns, this is both the width and the height of the button.
	/// \param label              a label to prefix to the selected entry on the display button.
	/// \param type               whether this is a textual or pictorial dropdown
	/// \param style              the style used for buttons and background
	/// Text conventions: Title Case for all elements
	Dropdown(Panel* parent,
	         const std::string& name,
	         int32_t x,
	         int32_t y,
	         uint32_t list_w,
	         uint32_t max_list_items,
	         int button_dimension,
	         const std::string& label,
	         const DropdownType type,
	         PanelStyle style,
	         ButtonStyle button_style)
	   : BaseDropdown(parent,
	                  name,
	                  x,
	                  y,
	                  list_w,
	                  max_list_items,
	                  button_dimension,
	                  label,
	                  type,
	                  style,
	                  button_style) {
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
	/// \param hotkey       a hotkey tip if any
	void add(const std::string& name,
	         Entry value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string(),
	         const std::string& hotkey = std::string()) {
		entry_cache_.push_back(std::unique_ptr<Entry>(new Entry(value)));
		BaseDropdown::add(name, size(), pic, select_this, tooltip_text, hotkey);
	}

	/// \return the selected element
	const Entry& get_selected() const {
		assert(BaseDropdown::has_selection());
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
