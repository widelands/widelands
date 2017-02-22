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

#ifndef WL_UI_BASIC_TABLE_H
#define WL_UI_BASIC_TABLE_H

#include <limits>
#include <vector>

#include <boost/function.hpp>
#include <boost/signals2.hpp>

#include "graphic/align.h"
#include "graphic/color.h"
#include "graphic/graphic.h"
#include "ui_basic/panel.h"

namespace UI {
struct Scrollbar;
struct Button;

enum class TableColumnType { kFixed, kFlexible };

/** A table with columns and lines.
 *
 * The entries can be sorted by columns by
 * clicking on the column header button.
 *
 *  Entry can be
 *    1. a reference type,
 *    2. a pointer type or
 *    3. uintptr_t.
 */
template <typename Entry> class Table {
public:
	struct EntryRecord {};

	Table(Panel* parent,
	      int32_t x,
	      int32_t y,
	      uint32_t w,
	      uint32_t h,
	      const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"),
	      bool descending = false);
	~Table();

	boost::signals2::signal<void(uint32_t)> selected;
	boost::signals2::signal<void(uint32_t)> double_clicked;

	/// A column that has a title is sortable (by clicking on the title).
	void add_column(uint32_t width,
	                const std::string& title = std::string(),
	                const std::string& tooltip = std::string(),
	                Align = UI::Align::kLeft,
	                TableColumnType column_type = TableColumnType::kFixed,
	                bool is_checkbox_column = false);

	void set_column_title(uint8_t col, const std::string& title);

	void clear();
	void set_sort_column(uint8_t col);
	uint8_t get_sort_colum() const;
	bool get_sort_descending() const;

	void sort(uint32_t Begin = 0, uint32_t End = std::numeric_limits<uint32_t>::max());
	void remove(uint32_t);
	void remove_entry(Entry);

	EntryRecord& add(void* const entry, const bool select_this = false);

	uint32_t size() const;
	bool empty() const;
	Entry operator[](uint32_t) const;
	static uint32_t no_selection_index();
	bool has_selection() const;
	uint32_t selection_index() const;
	EntryRecord& get_record(uint32_t) const;
	static Entry get(const EntryRecord&);
	EntryRecord* find(Entry) const;

	void select(uint32_t);
	void move_selection(int32_t offset);
	struct NoSelection : public std::exception {
		char const* what() const noexcept override {
			return "UI::Table<Entry>: No selection";
		}
	};
	EntryRecord& get_selected_record() const;
	Entry get_selected() const;

	///  Return the total height (text + spacing) occupied by a single line.
	uint32_t get_lineheight() const;

	uint32_t get_eff_w() const;

	// Drawing and event handling
	void draw(RenderTarget&);
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y);
	bool handle_mousewheel(uint32_t which, int32_t x, int32_t y);
	virtual bool handle_key(bool down, SDL_Keysym code);
};

template <> class Table<void*> : public Panel {
public:
	struct EntryRecord {
		EntryRecord(void* entry);

		void set_picture(uint8_t col, const Image* pic, const std::string& = std::string());
		void set_string(uint8_t col, const std::string&);
		const Image* get_picture(uint8_t col) const;
		const std::string& get_string(uint8_t col) const;
		void* entry() const {
			return entry_;
		}
		void set_color(const RGBColor& c) {
			use_clr = true;
			clr = c;
		}

		bool use_color() const {
			return use_clr;
		}
		RGBColor get_color() const {
			return clr;
		}

		void set_checked(uint8_t col, bool checked);
		void toggle(uint8_t col);
		bool is_checked(uint8_t col) const;

	private:
		friend class Table<void*>;
		void* entry_;
		bool use_clr;
		RGBColor clr;
		struct Data {
			const Image* d_picture;
			std::string d_string;
			bool d_checked;

			Data() : d_checked(false) {
			}
		};
		std::vector<Data> data_;
	};

	/**
	 * Compare the two items at the given indices in the list.
	 *
	 * return true if the first item is strictly less than the second
	 */
	using CompareFn = boost::function<bool(uint32_t, uint32_t)>;

	Table(Panel* parent,
	      int32_t x,
	      int32_t y,
	      uint32_t w,
	      uint32_t h,
	      const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"),
	      bool descending = false);
	~Table();

	boost::signals2::signal<void(uint32_t)> selected;
	boost::signals2::signal<void(uint32_t)> double_clicked;

	void add_column(uint32_t width,
	                const std::string& title = std::string(),
	                const std::string& tooltip = std::string(),
	                Align = UI::Align::kLeft,
	                TableColumnType column_type = TableColumnType::kFixed,
	                bool is_checkbox_column = false);

	void set_column_title(uint8_t col, const std::string& title);
	void set_column_compare(uint8_t col, const CompareFn& fn);

	void clear();
	void set_sort_column(uint8_t const col) {
		assert(col < columns_.size());
		sort_column_ = col;
	}
	uint8_t get_sort_colum() const {
		return sort_column_;
	}
	bool get_sort_descending() const {
		return sort_descending_;
	}
	void set_sort_descending(bool const descending) {
		sort_descending_ = descending;
	}

	void sort(uint32_t Begin = 0, uint32_t End = std::numeric_limits<uint32_t>::max());
	void remove(uint32_t);
	void remove_entry(const void* const entry);

	EntryRecord& add(void* entry = nullptr, bool select = false);

	uint32_t size() const {
		return entry_records_.size();
	}
	bool empty() const {
		return entry_records_.empty();
	}
	void* operator[](uint32_t const i) const {
		assert(i < entry_records_.size());
		return entry_records_[i]->entry();
	}
	static uint32_t no_selection_index() {
		return std::numeric_limits<uint32_t>::max();
	}
	bool has_selection() const {
		return selection_ != no_selection_index();
	}
	uint32_t selection_index() const {
		return selection_;
	}
	EntryRecord& get_record(uint32_t const n) const {
		assert(n < entry_records_.size());
		return *entry_records_[n];
	}
	static void* get(const EntryRecord& er) {
		return er.entry();
	}
	EntryRecord* find(const void* entry) const;

	void select(uint32_t);
	void move_selection(int32_t offset);
	struct NoSelection : public std::exception {
		char const* what() const noexcept override {
			return "UI::Table<void *>: No selection";
		}
	};
	EntryRecord& get_selected_record() const {
		if (selection_ == no_selection_index())
			throw NoSelection();
		assert(selection_ < entry_records_.size());
		return *entry_records_.at(selection_);
	}
	void remove_selected() {
		if (selection_ == no_selection_index())
			throw NoSelection();
		remove(selection_);
	}
	void* get_selected() const {
		return get_selected_record().entry();
	}

	uint32_t get_lineheight() const {
		return lineheight_ + 2;
	}
	uint32_t get_eff_w() const;

	/// Adjust the desired size to fit the height needed for the number of entries.
	/// If entries == 0, the current entries are used.
	void fit_height(uint32_t entries = 0);

	// Drawing and event handling
	void draw(RenderTarget&) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mousewheel(uint32_t which, int32_t x, int32_t y) override;
	bool handle_key(bool down, SDL_Keysym code) override;

private:
	bool default_compare_checkbox(uint32_t column, uint32_t a, uint32_t b);
	bool default_compare_string(uint32_t column, uint32_t a, uint32_t b);
	bool sort_helper(uint32_t a, uint32_t b);
	void layout() override;

	struct Column {
		Button* btn;
		uint32_t width;
		Align alignment;
		bool is_checkbox_column;
		CompareFn compare;
	};
	using Columns = std::vector<Column>;

	static const int32_t ms_darken_value = -20;

	Columns columns_;
	uint32_t total_width_;
	uint32_t headerheight_;
	int32_t lineheight_;
	const Image* button_background_;
	Scrollbar* scrollbar_;
	// A disabled button that will fill the space above the scroll bar
	UI::Button* scrollbar_filler_button_;
	int32_t scrollpos_;  //  in pixels
	uint32_t selection_;
	uint32_t last_click_time_;
	uint32_t last_selection_;  // for double clicks
	Columns::size_type sort_column_;
	bool sort_descending_;
	// This column will grow/shrink depending on the scrollbar being present
	size_t flexible_column_;

	void header_button_clicked(Columns::size_type);
	using EntryRecordVector = std::vector<EntryRecord*>;
	EntryRecordVector entry_records_;
	void set_scrollpos(int32_t pos);
};

template <typename Entry> class Table<const Entry* const> : public Table<void*> {
public:
	using Base = Table<void*>;
	Table(Panel* parent,
	      int32_t x,
	      int32_t y,
	      uint32_t w,
	      uint32_t h,
	      const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"),
	      const bool descending = false)
	   : Base(parent, x, y, w, h, button_background, descending) {
	}

	void remove_entry(Entry const* const entry) {
		Base::remove_entry(const_cast<Entry*>(entry));
	}

	EntryRecord& add(Entry const* const entry = 0, bool const select_this = false) {
		return Base::add(const_cast<Entry*>(entry), select_this);
	}

	Entry const* operator[](uint32_t const i) const {
		return static_cast<Entry const*>(Base::operator[](i));
	}

	static Entry const* get(const EntryRecord& er) {
		return static_cast<Entry const*>(er.entry());
	}

	Entry const* get_selected() const {
		return static_cast<Entry const*>(Base::get_selected());
	}
};

template <typename Entry> class Table<Entry* const> : public Table<void*> {
public:
	using Base = Table<void*>;
	Table(Panel* parent,
	      int32_t x,
	      int32_t y,
	      uint32_t w,
	      uint32_t h,
	      const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"),
	      const bool descending = false)
	   : Base(parent, x, y, w, h, button_background, descending) {
	}

	void remove_entry(Entry const* entry) {
		Base::remove_entry(entry);
	}

	EntryRecord& add(Entry* const entry = 0, bool const select_this = false) {
		return Base::add(entry, select_this);
	}

	Entry* operator[](uint32_t const i) const {
		return static_cast<Entry*>(Base::operator[](i));
	}

	static Entry* get(const EntryRecord& er) {
		return static_cast<Entry*>(er.entry());
	}

	Entry* get_selected() const {
		return static_cast<Entry*>(Base::get_selected());
	}
};

template <typename Entry> class Table<const Entry&> : public Table<void*> {
public:
	using Base = Table<void*>;
	Table(Panel* parent,
	      int32_t x,
	      int32_t y,
	      uint32_t w,
	      uint32_t h,
	      const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"),
	      const bool descending = false)
	   : Base(parent, x, y, w, h, button_background, descending) {
	}

	void remove_entry(const Entry& entry) {
		Base::remove_entry(&const_cast<Entry&>(entry));
	}

	EntryRecord& add(const Entry& entry, bool const select_this = false) {
		return Base::add(&const_cast<Entry&>(entry), select_this);
	}

	const Entry& operator[](uint32_t const i) const {
		return *static_cast<Entry const*>(Base::operator[](i));
	}

	static const Entry& get(const EntryRecord& er) {
		return *static_cast<Entry const*>(er.entry());
	}

	EntryRecord* find(const Entry& entry) const {
		return Base::find(&entry);
	}

	const Entry& get_selected() const {
		return *static_cast<Entry const*>(Base::get_selected());
	}
};

template <typename Entry> class Table<Entry&> : public Table<void*> {
public:
	using Base = Table<void*>;
	Table(Panel* parent,
	      int32_t x,
	      int32_t y,
	      uint32_t w,
	      uint32_t h,
	      const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"),
	      const bool descending = false)
	   : Base(parent, x, y, w, h, button_background, descending) {
	}

	void remove_entry(Entry& entry) {
		Base::remove_entry(&entry);
	}

	EntryRecord& add(Entry& entry, bool const select_this = false) {
		return Base::add(&entry, select_this);
	}

	Entry& operator[](uint32_t const i) const {
		return *static_cast<Entry*>(Base::operator[](i));
	}

	static Entry& get(const EntryRecord& er) {
		return *static_cast<Entry*>(er.entry());
	}

	EntryRecord* find(Entry& entry) const {
		return Base::find(&entry);
	}

	Entry& get_selected() const {
		return *static_cast<Entry*>(Base::get_selected());
	}
};

static_assert(sizeof(void*) == sizeof(uintptr_t),
              "assert(sizeof(void *) == sizeof(uintptr_t)) failed.");
template <> class Table<uintptr_t> : public Table<void*> {
public:
	using Base = Table<void*>;
	Table(Panel* parent,
	      int32_t x,
	      int32_t y,
	      uint32_t w,
	      uint32_t h,
	      const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"),
	      const bool descending = false)
	   : Base(parent, x, y, w, h, button_background, descending) {
	}

	void remove_entry(uintptr_t const entry) {
		Base::remove_entry(reinterpret_cast<void*>(entry));
	}

	EntryRecord& add(uintptr_t const entry, bool const select_this = false) {
		return Base::add(reinterpret_cast<void*>(entry), select_this);
	}

	uintptr_t operator[](uint32_t const i) const {
		return reinterpret_cast<uintptr_t>(Base::operator[](i));
	}
	static uintptr_t get(const EntryRecord& er) {
		return reinterpret_cast<uintptr_t>(er.entry());
	}

	EntryRecord* find(uintptr_t const entry) const {
		return Base::find(reinterpret_cast<void const*>(entry));
	}

	uintptr_t get_selected() const {
		return reinterpret_cast<uintptr_t>(Base::get_selected());
	}
};
template <> class Table<uintptr_t const> : public Table<uintptr_t> {
public:
	using Base = Table<uintptr_t>;
	Table(Panel* parent,
	      int32_t x,
	      int32_t y,
	      uint32_t w,
	      uint32_t h,
	      const Image* button_background = g_gr->images().get("images/ui_basic/but3.png"),
	      const bool descending = false)
	   : Base(parent, x, y, w, h, button_background, descending) {
	}
};
}

#endif  // end of include guard: WL_UI_BASIC_TABLE_H
