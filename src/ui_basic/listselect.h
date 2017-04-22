/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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
#include <limits>

#include <boost/signals2.hpp>

#include "graphic/color.h"
#include "graphic/graphic.h"
#include "ui_basic/panel.h"
#include "ui_basic/scrollbar.h"

namespace UI {
struct Scrollbar;

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
	               const Image* button_background,
	               ListselectLayout selection_mode = ListselectLayout::kPlain);
	~BaseListselect();

	boost::signals2::signal<void(uint32_t)> selected;
	boost::signals2::signal<void(uint32_t)> clicked;
	boost::signals2::signal<void(uint32_t)> double_clicked;

	void clear();
	void sort(const uint32_t Begin = 0, uint32_t End = std::numeric_limits<uint32_t>::max());
	void add(const std::string& name,
	         uint32_t value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string());
	void add_front(const std::string& name,
	               const Image* pic = nullptr,
	               const bool select_this = false,
	               const std::string& tooltip_text = std::string());
	void remove(uint32_t);
	void remove(const char* name);

	void switch_entries(uint32_t, uint32_t);

	void set_entry_color(uint32_t, const RGBColor&);

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
	void remove_selected();

	const std::string& get_selected_name() const;
	const std::string& get_selected_tooltip() const;
	const Image* get_selected_image() const;

	void set_background(const Image* background) {
		background_ = background;
	}

	///  Return the total height (text + spacing) occupied by a single line.
	int get_lineheight() const;

	uint32_t get_eff_w() const;

	void layout() override;

	// Drawing and event handling
	void draw(RenderTarget&) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousewheel(uint32_t which, int32_t x, int32_t y) override;
	bool handle_key(bool down, SDL_Keysym) override;

private:
	static const int32_t DOUBLE_CLICK_INTERVAL = 500;  // half a second

	void set_scrollpos(int32_t);

private:
	static const int32_t ms_darken_value = -20;

	struct EntryRecord {
		uint32_t entry_;
		bool use_clr;
		RGBColor clr;
		const Image* pic;
		std::string name;
		std::string tooltip;
	};
	using EntryRecordDeque = std::deque<EntryRecord*>;

	int max_pic_width_;
	int lineheight_;
	EntryRecordDeque entry_records_;
	Scrollbar scrollbar_;
	uint32_t scrollpos_;  //  in pixels
	uint32_t selection_;
	uint32_t last_click_time_;
	uint32_t last_selection_;  // for double clicks
	ListselectLayout selection_mode_;
	const Image* check_pic_;
	const Image* background_;
	std::string current_tooltip_;
};

template <typename Entry> struct Listselect : public BaseListselect {
	Listselect(Panel* parent,
	           int32_t x,
	           int32_t y,
	           uint32_t w,
	           uint32_t h,
	           const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"),
	           ListselectLayout selection_mode = ListselectLayout::kPlain)
	   : BaseListselect(parent, x, y, w, h, button_background, selection_mode) {
	}

	void add(const std::string& name,
	         Entry value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string()) {
		entry_cache_.push_back(value);
		BaseListselect::add(name, entry_cache_.size() - 1, pic, select_this, tooltip_text);
	}
	void add_front(const std::string& name,
	               Entry value,
	               const Image* pic = nullptr,
	               const bool select_this = false,
	               const std::string& tooltip_text = std::string()) {
		entry_cache_.push_front(value);
		BaseListselect::add_front(name, pic, select_this, tooltip_text);
	}

	const Entry& operator[](uint32_t const i) const {
		return entry_cache_[BaseListselect::operator[](i)];
	}

	const Entry& get_selected() const {
		return entry_cache_[BaseListselect::get_selected()];
	}

	void set_background(const Image* background) {
		BaseListselect::set_background(background);
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
	           const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"),
	           ListselectLayout selection_mode = ListselectLayout::kPlain)
	   : Base(parent, x, y, w, h, button_background, selection_mode) {
	}

	void add(const std::string& name,
	         Entry& value,
	         const Image* pic = nullptr,
	         const bool select_this = false,
	         const std::string& tooltip_text = std::string()) {
		Base::add(name, &value, pic, select_this, tooltip_text);
	}
	void add_front(const std::string& name,
	               Entry& value,
	               const Image* pic = nullptr,
	               const bool select_this = false,
	               const std::string& tooltip_text = std::string()) {
		Base::add_front(name, &value, pic, select_this, tooltip_text);
	}

	Entry& operator[](uint32_t const i) const {
		return *Base::operator[](i);
	}

	Entry& get_selected() const {
		return *Base::get_selected();
	}

	void set_background(const Image* background) {
		*Base::set_background(background);
	}
};
}

#endif  // end of include guard: WL_UI_BASIC_LISTSELECT_H
