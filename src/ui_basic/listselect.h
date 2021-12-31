/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_LISTSELECT_H
#define WL_UI_BASIC_LISTSELECT_H

#include <deque>
#include <memory>

#include "graphic/styles/table_style.h"
#include "graphic/text/rendered_text.h"
#include "ui_basic/panel.h"
#include "ui_basic/scrollbar.h"

namespace UI {

class BaseDropdown;

enum class ListselectLayout {
	kPlain,     // Highlight the selected element
	kDropdown,  // When the mouse moves, instantly select the element that the mouse hovers over
	kShowCheck  // Show a green arrow in front of the selected element
};

/**
 * This class defines a list-select box whose entries are defined by a name
 * and an associated numeric ID.
 *
 * Use the \ref Listselect template to use arbitrary IDs.
 */
struct BaseListselect : public Panel {
	BaseListselect(Panel* parent,
	               int32_t x,
	               int32_t y,
	               uint32_t w,
	               uint32_t h,
	               PanelStyle style,
	               ListselectLayout selection_mode = ListselectLayout::kPlain);
	~BaseListselect() override;

	Notifications::Signal<uint32_t> selected;
	Notifications::Signal<uint32_t> double_clicked;

	void clear();
	void sort(const uint32_t Begin = 0, uint32_t End = std::numeric_limits<uint32_t>::max());
	/**
	 * Text conventions: Title Case for the 'name', Sentence case for the 'tooltip_text'
	 */
	void add(const std::string& name,
	         uint32_t value,
	         const Image* pic,
	         const bool select_this,
	         const std::string& tooltip_text,
	         const std::string& hotkey,
	         const unsigned indent);

	void remove(uint32_t);
	void remove(const char* name);

	uint32_t size() const {
		return entry_records_.size();
	}
	bool empty() const {
		return entry_records_.empty();
	}

	uint32_t operator[](const uint32_t i) const {
		assert(i < size());
		return entry_records_[i]->entry_;
	}

	static uint32_t no_selection_index() {
		return std::numeric_limits<uint32_t>::max();
	}

	uint32_t selection_index() const {
		return selection_;
	}

	void select(uint32_t i);
	bool has_selection() const;

	uint32_t get_selected() const;

	const std::string& get_selected_name() const;
	const std::string& get_selected_tooltip() const;
	const Image* get_selected_image() const;

	///  Return the total height (text + spacing) occupied by a single line.
	int get_lineheight() const;

	uint32_t get_eff_w() const;

	int calculate_desired_width();

	void layout() override;

	// Drawing and event handling
	void draw(RenderTarget&) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;
	bool handle_key(bool down, SDL_Keysym) override;

	void set_linked_dropdown(UI::BaseDropdown* d) {
		linked_dropdown = d;
	}

private:
	static const int32_t ms_darken_value = -20;

	void set_scrollpos(int32_t);

	struct EntryRecord {
		explicit EntryRecord(const std::string& init_name,
		                     uint32_t init_entry,
		                     const Image* init_pic,
		                     const std::string& tooltip_text,
		                     const std::string& hotkey_text,
		                     const unsigned indent,
		                     const UI::TableStyleInfo& style);
		~EntryRecord();

		const std::string name;
		const uint32_t entry_;
		const Image* pic;
		const std::string tooltip;
		const Align name_alignment;
		const Align hotkey_alignment;
		const unsigned indent;
		std::shared_ptr<const UI::RenderedText> rendered_name;
		std::shared_ptr<const UI::RenderedText> rendered_hotkey;
	};

	int max_pic_width_;
	int widest_text_;
	int widest_hotkey_;

	std::deque<EntryRecord*> entry_records_;
	Scrollbar scrollbar_;
	uint32_t scrollpos_;  //  in pixels
	uint32_t selection_;
	uint32_t last_click_time_;
	uint32_t last_selection_;  // for double clicks
	ListselectLayout selection_mode_;
	const Image* check_pic_;

	const UI::TableStyleInfo& table_style() const;
	const UI::PanelStyleInfo* background_style() const;  // Background color and texture

	int lineheight_;
	std::string current_tooltip_;

	UI::BaseDropdown* linked_dropdown;
};

template <typename Entry> struct Listselect : public BaseListselect {
	Listselect(Panel* parent,
	           int32_t x,
	           int32_t y,
	           uint32_t w,
	           uint32_t h,
	           UI::PanelStyle style,
	           ListselectLayout selection_mode = ListselectLayout::kPlain)
	   : BaseListselect(parent, x, y, w, h, style, selection_mode) {
	}

	void add(const std::string& name,
	         Entry value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string(),
	         const std::string& hotkey = std::string(),
	         const unsigned indent = 0) {
		entry_cache_.push_back(value);
		BaseListselect::add(
		   name, entry_cache_.size() - 1, pic, select_this, tooltip_text, hotkey, indent);
	}

	const Entry& operator[](uint32_t const i) const {
		return entry_cache_[BaseListselect::operator[](i)];
	}

	const Entry& get_selected() const {
		return entry_cache_[BaseListselect::get_selected()];
	}

private:
	std::deque<Entry> entry_cache_;
};

/**
 * This template specialization is for backwards compatibility and convenience
 * only. Allowing references as template parameter is not a good idea
 * (e.g. STL containers don't allow it), you should really use pointers instead
 * because they are more explicit, and that's what this specialization does
 * internally.
 */
template <typename Entry> struct Listselect<Entry&> : public Listselect<Entry*> {
	using Base = Listselect<Entry*>;

	Listselect(Panel* parent,
	           int32_t x,
	           int32_t y,
	           uint32_t w,
	           uint32_t h,
	           UI::PanelStyle style,
	           ListselectLayout selection_mode = ListselectLayout::kPlain)
	   : Base(parent, x, y, w, h, style, selection_mode) {
	}

	void add(const std::string& name,
	         Entry& value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string(),
	         const std::string& hotkey = std::string(),
	         const unsigned indent = 0) {
		Base::add(name, &value, pic, select_this, tooltip_text, hotkey, indent);
	}

	Entry& operator[](uint32_t const i) const {
		return *Base::operator[](i);
	}

	Entry& get_selected() const {
		return *Base::get_selected();
	}
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_LISTSELECT_H
