/*
 * Copyright (C) 2016-2024 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_DROPDOWN_H
#define WL_UI_BASIC_DROPDOWN_H

#include <deque>
#include <functional>
#include <memory>

#include "base/i18n.h"
#include "base/string.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/note_graphic_resolution_changed.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/panel.h"
#include "wlapplication_options.h"

namespace UI {
// We use this to make sure that only 1 dropdown is open at the same time.
struct NoteDropdown {
	CAN_BE_SENT_AS_NOTE(NoteId::Dropdown)

	int id;

	explicit NoteDropdown(int init_id) : id(init_id) {
	}
};

struct DropdownType {
	enum class Display { kShowText, kShowIcon };
	enum class Format {
		kTraditional,  // selected entry display separate from drop-down arrow push button
		kButtonOnly,   // selected entry displayed on button, no separate drop-down arrow
		kMenu,  // selected entry not displayed; use to trigger actions without changing the button
		        // text or icon
		kCheckmark,   // like kMenu, but with checkmark next to the selected entry in the dropdown
		kMultiSelect  // like kCheckmark, but checkmark of each entry is toggled independently
	};

	Display display;
	Format format;

	DropdownType(Display d, Format f) : display(d), format(f) {
		assert(!(d == Display::kShowIcon && f == Format::kTraditional));  // pointless combination
	}

	bool operator==(const DropdownType& other) const {
		return (display == other.display) && (format == other.format);
	}

	// ----- PRESETS -----
	static const DropdownType kTextual;            // Display::kShowText, Format::kTraditional
	static const DropdownType kTextualNarrow;      // Display::kShowText, Format::kButtonOnly
	static const DropdownType kPictorial;          // Display::kShowIcon, Format::kButtonOnly
	static const DropdownType kTextualMenu;        // Display::kShowText, Format::kMenu
	static const DropdownType kPictorialMenu;      // Display::kShowIcon, Format::kMenu
	static const DropdownType kTextualRadioGrp;    // Display::kShowText, Format::kCheckmark
	static const DropdownType kPictorialRadioGrp;  // Display::kShowIcon, Format::kCheckmark
	static const DropdownType kTextualToggles;     // Display::kShowText, Format::kMultiSelect
	static const DropdownType kPictorialToggles;   // Display::kShowIcon, Format::kMultiSelect
};

/// Implementation for a dropdown menu that lets the user select a value.
class BaseDropdown : public Panel {
protected:
	/// \param parent             the parent panel
	/// \param name               a name so that we can reference the dropdown via Lua
	/// \param x                  the x-position within 'parent'
	/// \param y                  the y-position within 'parent'
	/// \param list_w             the dropdown's width
	/// \param max_list_items     the maximum number of items shown in the list before it starts
	///                           using a scrollbar
	/// \param button_dimension   the width of the push button in textual dropdowns. For pictorial
	///                           dropdowns, this is both the width and the height of the button.
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
	             DropdownType type,
	             PanelStyle style,
	             ButtonStyle button_style);

public:
	~BaseDropdown() override;

	/// An entry was selected
	Notifications::Signal<> selected;

	/// \return true if an element has been selected from the list
	bool has_selection() const;

	void clear_selection();

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

	/// Accessor for dropdown type.
	DropdownType type() const {
		return type_;
	}

	/// Whether the dropdown has no elements to select.
	bool empty() const {
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

	///  Return the total height (text + spacing) occupied by a single entry.
	int lineheight() const;
	void set_min_lineheight(int minh) {
		list_->set_min_lineheight(minh);
	}

	/// Toggle the list on and off and position the mouse on the button so that the dropdown won't
	/// close on us. If this is a menu and nothing was selected yet, select the first item for easier
	/// keyboard navigation.
	void toggle();
	/// Toggle the list closed if the dropdown is currently expanded.
	void close();

	/// If 'open', show the list and position the mouse on the button so that the dropdown won't
	/// close on us. If this is a menu and nothing was selected yet, select the first item for easier
	/// keyboard navigation. If not 'open', close the list.
	void set_list_visibility(bool open, bool move_mouse = true);

	void set_size(int nw, int nh) override;
	void set_desired_size(int w, int h) override;
	void enable_textinput();
	void disable_textinput();

	/// Expand display button to make enough room for each entry's text. Call this before adding any
	/// entries.
	void set_autoexpand_display_button();

	void notify_list_deleted() {
		list_ = nullptr;
	}

	virtual void clear_filter() = 0;
	void delete_last_of_filter();

	bool is_filtered();

	std::string get_filter_text();

	virtual void clear_filtered_out_checkmarks(uint32_t) = 0;

protected:
	/// Add an element to the list
	/// \param name         the display name of the entry
	/// \param value        the index of the entry
	/// \param pic          an image to illustrate the entry. Can be nullptr for textual dropdowns.
	/// \param select_this  whether this element should be selected (checked state for MultiSelect)
	/// \param tooltip_text a tooltip for this entry
	/// \param hotkey       a hotkey tip if any
	///
	/// Text conventions: Title Case for the 'name', Sentence case for the 'tooltip_text'
	void add(const std::string& name,
	         uint32_t value,
	         const Image* pic,
	         bool select_this,
	         const std::string& tooltip_text,
	         const std::string& hotkey,
	         unsigned indent = 0,
	         bool enable = true);

	/// \return the index of the selected element
	uint32_t get_selected() const;

	/// Select the entry. Assumes that it exists. Does not trigger the 'selected' signal.
	void select(uint32_t entry);

	/// Removes all elements from the list.
	void clear();

	/// Connect to checkmark_changed signal of underlying listselect
	void connect_checkmark_changed(std::function<void(uint32_t, bool)> callback) const;

	/// manipulate checkmarks in entries (requires Format::kMultiSelect)
	void clear_checked(bool notify = false);
	bool is_checked(uint32_t entry) const;
	bool set_checked(uint32_t entry, bool newstate, bool notify = false);
	bool toggle_checked(uint32_t entry, bool notify = false);

	/// Automatically collapses the list if the mouse gets too far away from the dropdown, or if it
	/// loses focus.
	void think() override;

	UI::Panel* get_open_dropdown() override;

	std::vector<Recti> focus_overlay_rects() override;

	std::string current_filter_;
	bool no_filter_matches_{false};

private:
	static void layout_if_alive(int);
	void layout() override;

	/// Updates the buttons
	void update();

	/// Updates the title and tooltip of the display button, closes the dropdown (unless
	/// keep_open is set) and triggers a 'selected' signal.
	void set_value(bool keep_open = false);
	/// Toggles the dropdown list on and off and sends a notification if the list is visible
	/// afterwards.
	void toggle_list();

	/// Returns true if the mouse pointer left the vicinity of the dropdown.
	bool is_mouse_away() const;

	virtual void apply_filter() = 0;

	virtual void
	save_selected_entry(uint32_t)  // cannot be pure virtual because it is called from constructor
	{
	}

	/// Give each dropdown a unique ID
	static int next_id_;
	const int id_;
	std::unique_ptr<Notifications::Subscriber<NoteDropdown>> dropdown_subscriber_;
	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;

	// Dimensions
	unsigned int max_list_items_;
	uint32_t max_list_height_{std::numeric_limits<uint32_t>::max()};
	int list_offset_x_{0};
	int list_offset_y_{0};
	const int base_height_;
	const int mouse_tolerance_{50};  // Allow mouse outside the panel a bit before autocollapse
	UI::Box button_box_;
	UI::Button* push_button_;  // Only used in textual dropdowns
	UI::Button display_button_;
	// The list needs to be a pointer for destruction, because we hook into the highest parent that
	// we can get.
	UI::Listselect<uintptr_t>* list_;
	std::string label_;
	std::string tooltip_;
	uint32_t current_selection_;

protected:
	DropdownType type_;

private:
	bool is_enabled_{true};
	ButtonStyle button_style_;
	bool autoexpand_display_button_{false};
};

/// A dropdown menu that lets the user select a value of the datatype 'Entry'.
template <typename Entry> class Dropdown : public BaseDropdown {
public:
	class ExtendedEntry {
	public:
		const std::string name;
		const Entry value;
		const Image* img;
		const std::string tooltip;
		const std::string hotkey;
		bool checked{false};  // checked state
	};

	using HotkeyFunction = std::function<void(Entry)>;
	/// \param parent             the parent panel
	/// \param name               a name so that we can reference the dropdown via Lua
	/// \param x                  the x-position within 'parent'
	/// \param y                  the y-position within 'parent'
	/// \param list_w             the dropdown's width
	/// \param max_list_items     the maximum number of items shown in the list before it starts
	/// using a scrollbar
	/// \param button_dimension   the width of the push button in textual
	/// dropdowns. For pictorial dropdowns, this is both the width and the height of the button.
	/// \param label              a label to prefix to the selected entry on the display button.
	/// \param type               whether this is a textual or pictorial dropdown
	/// \param style              the style used for buttons and background
	/// \param hotkey_fn          function that is invoked with selected value if matching hotkey was
	/// pressed Text conventions: Title Case for all elements
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
	         ButtonStyle button_style,
	         const HotkeyFunction& hotkey_fn = HotkeyFunction())
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
	                  button_style),
	     hotkey_fn_(hotkey_fn) {
		if (type_.format >= DropdownType::Format::kCheckmark) {
			// Adapt checkmark_changed signal from underlying listselect
			BaseDropdown::connect_checkmark_changed([this](uint32_t idx, bool newstate) {
				assert(!no_filter_matches_);  // entry is disabled, should not respond to input
				Entry entry = *filtered_entries[idx];

				// update state stored in unfiltered
				auto it = std::find_if(unfiltered_entries.begin(), unfiltered_entries.end(),
				                       [entry](auto& e) { return e.value == entry; });
				assert(it != unfiltered_entries.end());
				if (it->checked != newstate) {
					it->checked = newstate;
					checkmark_changed(entry, newstate);
				}
			});
		}
	}
	~Dropdown() override {
		filtered_entries.clear();
		unfiltered_entries.clear();
	}

	Notifications::Signal<Entry, bool> checkmark_changed;

	[[nodiscard]] uint32_t unfiltered_size() const {
		return unfiltered_entries.size();
	}

	void clear_filter() override {
		if (current_filter_.empty()) {
			return;
		}
		current_filter_.clear();
		no_filter_matches_ = false;

		restore_filtered_list();
		// re-select the selected entry after clear & repopulate; excludes
		// MultiSelect, which restores checkmark state during repopulation
		if (type_.format != DropdownType::Format::kMultiSelect) {
			select(selected_entry_);
		}
	}

	void clear_filtered_out_checkmarks(uint32_t sel) override {
		assert(type_.format == DropdownType::Format::kCheckmark);
		if (no_filter_matches_) {
			return;
		}

		Entry sel_entry{};
		if (sel != BaseListselect::no_selection_index()) {
			sel_entry = *filtered_entries[sel];
		}

		auto it = std::find_if(unfiltered_entries.begin(), unfiltered_entries.end(),
		                       [sel_entry](auto& x) { return x.checked && x.value != sel_entry; });
		if (it != unfiltered_entries.end()) {
			it->checked = false;
			checkmark_changed(it->value, false);
		}
	}

	bool handle_textinput(const std::string& input_text) override {
		const std::string lowered_input_text = to_lower(input_text);
		if (current_filter_.empty() && check_hotkey_match(lowered_input_text)) {
			return true;
		}
		update_filter(lowered_input_text);
		apply_filter();
		return true;
	}

	/// Add an element to the list
	/// \param name         the display name of the entry
	/// \param value        the value for the entry
	/// \param pic          an image to illustrate the entry. Can be nullptr in textual dropdowns
	/// only.
	/// \param select_this  whether this element should be selected (also: checked state)
	/// \param tooltip_text a tooltip for this entry
	/// \param hotkey       a hotkey tip if any
	void add(const std::string& name,
	         Entry value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string(),
	         const std::string& hotkey = std::string()) {
		filtered_entries.push_back(std::unique_ptr<Entry>(new Entry(value)));
		unfiltered_entries.push_back(
		   {name, value, pic, tooltip_text, hotkey,
		    select_this && (type_.format >= DropdownType::Format::kCheckmark)});
		BaseDropdown::add(name, size(), pic, select_this, tooltip_text, hotkey, 0, true);
	}

	/// \return the selected element
	const Entry& get_selected() const {
		assert(BaseDropdown::has_selection());
		return *filtered_entries[BaseDropdown::get_selected()];
	}

	const ExtendedEntry& at(size_t index) const {
		assert(index < unfiltered_entries.size());
		return unfiltered_entries.at(index);
	}

	/// Select the entry if it exists. Does not trigger the 'selected' signal.
	void select(const Entry& entry) {
		for (uint32_t i = 0; i < filtered_entries.size(); ++i) {
			if (entry == *filtered_entries[i]) {
				selected_entry_ = entry;
				BaseDropdown::select(i);
			}
		}
	}

	/// Removes all elements from the list.
	void clear() {
		BaseDropdown::clear();
		filtered_entries.clear();
		unfiltered_entries.clear();
	}

	void clear_checked(bool notify) {
		BaseDropdown::clear_checked(notify);
		// update state stored in unfiltered: accounts for
		// omissions due to either not in filtered list or notify == false
		for (auto& ee : unfiltered_entries) {
			if (ee.checked) {
				ee.checked = false;
				if (notify) {
					checkmark_changed(ee.value, false);
				}
			}
		}
	}

	bool is_checked(const Entry& entry) const {
		assert(type_.format == DropdownType::Format::kMultiSelect);
		auto it = std::find_if(unfiltered_entries.begin(), unfiltered_entries.end(),
		                       [entry](auto& x) { return x.value == entry; });
		if (it != unfiltered_entries.end()) {
			return it->checked;
		}
		return false;
	}

	bool set_checked(const Entry& entry, bool newstate, bool notify = false) {
		assert(type_.format == DropdownType::Format::kMultiSelect);
		bool found = false;
		bool result = false;
		for (uint32_t i = 0; i < filtered_entries.size(); ++i) {
			if (entry == *filtered_entries[i]) {
				found = true;
				result = BaseDropdown::set_checked(i, newstate, notify);
			}
		}
		if (!found || (!notify && result)) {
			// update state stored in unfiltered: accounts for
			// omissions due to either not in filtered list or notify == false
			auto it = std::find_if(unfiltered_entries.begin(), unfiltered_entries.end(),
			                       [entry](auto& x) { return x.value == entry; });
			if (it != unfiltered_entries.end()) {
				result |= !found;
				bool oldstate = it->checked;
				it->checked = newstate;
				if (notify && oldstate != newstate) {
					checkmark_changed(entry, newstate);
				}
			}
		}
		return result;
	}

	bool toggle_checked(const Entry& entry, bool notify = false) {
		assert(type_.format == DropdownType::Format::kMultiSelect);
		bool found = false;
		bool result = false;
		for (uint32_t i = 0; i < filtered_entries.size(); ++i) {
			if (entry == *filtered_entries[i]) {
				found = true;
				result = BaseDropdown::toggle_checked(i, notify);
			}
		}
		if (!found || (!notify && result)) {
			// update state stored in unfiltered: accounts for
			// omissions due to either not in filtered list or notify == false
			auto it = std::find_if(unfiltered_entries.begin(), unfiltered_entries.end(),
			                       [entry](auto& x) { return x.value == entry; });
			if (it != unfiltered_entries.end()) {
				result |= !found;
				it->checked = !it->checked;
				if (notify) {
					checkmark_changed(entry, it->checked);
				}
			}
		}
		return result;
	}

private:
	void save_selected_entry(uint32_t index) override {
		selected_entry_ = index < filtered_entries.size() ? *filtered_entries[index] : Entry{};
	}
	void clear_filtered_list() {
		BaseDropdown::clear();
		filtered_entries.clear();
	}
	void restore_filtered_list() {
		clear_filtered_list();
		for (auto& ee : unfiltered_entries) {
			add_to_filtered_list(ee.name, ee.value, ee.img,
			                     ee.checked && type_.format == DropdownType::Format::kMultiSelect,
			                     ee.tooltip, ee.hotkey);
		}
	}

	// adds an element to the filtered list which is actually displayed
	void add_to_filtered_list(const std::string& name,
	                          Entry value,
	                          const Image* pic = nullptr,
	                          const bool select_this = false,
	                          const std::string& tooltip_text = std::string(),
	                          const std::string& hotkey = std::string()) {
		filtered_entries.push_back(std::unique_ptr<Entry>(new Entry(value)));
		BaseDropdown::add(name, size(), pic, select_this, tooltip_text, hotkey, 0, true);
	}
	bool check_hotkey_match(const std::string& input_text) {
		for (auto& x : unfiltered_entries) {
			if (" " == input_text) {
				SDL_Keycode c = SDLK_SPACE;
				const std::string localized_space =
				   to_lower(shortcut_string_for(SDL_Keysym{SDL_GetScancodeFromKey(c), c, 0, 0}, false));
				if (trigger_hotkey_on_match(localized_space, x)) {
					return true;
				}
			} else {
				if (trigger_hotkey_on_match(input_text, x)) {
					return true;
				}
			}
		}
		return false;
	}
	bool trigger_hotkey_on_match(const std::string& input_text, Dropdown::ExtendedEntry& x) {
		if (input_text == to_lower(x.hotkey) && is_in_filtered_list(x.value)) {
			if (hotkey_fn_) {
				hotkey_fn_(x.value);
			} else {
				verb_log_dbg("hotkey match: %s but no hotkey function available!", x.hotkey.c_str());
			}
			return true;
		}
		return false;
	}
	bool is_in_filtered_list(Entry entry) {
		for (uint32_t i = 0; i < filtered_entries.size(); ++i) {
			if (entry == *filtered_entries[i]) {
				return true;
			}
		}
		return false;
	}

	void update_filter(const std::string& input_text) {
		current_filter_ = current_filter_.append(input_text);
	}

	void apply_filter() override {
		clear_filtered_list();
		add_matching_entries();

		no_filter_matches_ = filtered_entries.empty();
		if (no_filter_matches_) {
			add_no_match_entry();
		}

		// highlight currently selected entry if it is in the filtered list
		if (type_.format < DropdownType::Format::kMenu ||
		    type_.format == DropdownType::Format::kCheckmark) {
			select(selected_entry_);
		}

		// force list to stay open even if mouse is now away due to smaller
		// dropdown list because of applied filter
		set_list_visibility(true);
	}

	void add_no_match_entry() {
		const Image* empty_icon = (type_.display == DropdownType::Display::kShowText) ?
                                   nullptr :
                                   g_image_cache->get("images/wui/editor/no_ware.png");
		BaseDropdown::add(
		   _("No matches"), 0, empty_icon, false,
		   /** TRANSLATORS: Tooltip shown when filtering a dropdown menu yields no matches. */
		   "No dropdown entries matched the given filter", std::string(), 0, false);
	}

	void add_matching_entries() {
		for (auto& x : unfiltered_entries) {
			if (to_lower(x.name).find(current_filter_) != std::string::npos) {
				add_to_filtered_list(x.name, x.value, x.img,
				                     x.checked && type_.format == DropdownType::Format::kMultiSelect,
				                     x.tooltip, x.hotkey);
			}
		}
	}

	// Contains the currently displayed (maybe filtered) elements. The BaseDropdown registers the
	// indices only.
	std::deque<std::unique_ptr<Entry>> filtered_entries;
	// Contains all the elements.
	std::deque<ExtendedEntry> unfiltered_entries;
	// remember the initial selected element to select it again
	// when filter yields no matches
	Entry selected_entry_{};
	const HotkeyFunction hotkey_fn_;
};

}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_DROPDOWN_H
