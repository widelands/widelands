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

#include "ui_basic/table.h"

#include <boost/bind.hpp>

#include "graphic/font.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "graphic/texture.h"
#include "ui_basic/button.h"
#include "ui_basic/mouse_constants.h"
#include "ui_basic/scrollbar.h"
#include "wlapplication.h"

namespace UI {

/**
 * Args: parent  parent panel
 *       x       coordinates of the Table
 *       y
 *       w       dimensions, in pixels, of the Table
 *       h
*/
Table<void*>::Table(Panel* const parent,
                    int32_t x,
                    int32_t y,
                    uint32_t w,
                    uint32_t h,
                    const Image* button_background,
                    TableRows rowtype)
   : Panel(parent, x, y, w, h),
     total_width_(0),
     headerheight_(
        UI::g_fh1->render(as_uifont(UI::g_fh1->fontset()->representative_character()))->height() +
        4),
     lineheight_(
        UI::g_fh1->render(as_uifont(UI::g_fh1->fontset()->representative_character()))->height()),
     button_background_(button_background),
     scrollbar_(nullptr),
     scrollbar_filler_button_(
        new Button(this, "", 0, 0, Scrollbar::kSize, headerheight_, button_background, "")),
     scrollpos_(0),
     selection_(no_selection_index()),
     last_multiselect_(no_selection_index()),
     last_click_time_(-10000),
     last_selection_(no_selection_index()),
     sort_column_(0),
     sort_descending_(rowtype == TableRows::kSingleDescending ||
                      rowtype == TableRows::kMultiDescending),
     flexible_column_(std::numeric_limits<size_t>::max()),
     is_multiselect_(rowtype == TableRows::kMulti || rowtype == TableRows::kMultiDescending),
     ctrl_down_(false),
     shift_down_(false) {
	set_thinks(false);
	set_can_focus(true);
	scrollbar_filler_button_->set_visible(false);
	scrollbar_ = new Scrollbar(this, get_w() - Scrollbar::kSize, headerheight_, Scrollbar::kSize,
	                           get_h() - headerheight_, button_background);
	scrollbar_->moved.connect(boost::bind(&Table::set_scrollpos, this, _1));
	scrollbar_->set_steps(1);
	scrollbar_->set_singlestepsize(lineheight_);
	scrollbar_->set_pagesize(get_h() - lineheight_);
	scrollbar_filler_button_->set_enabled(false);
}

/**
 * Free allocated resources
*/
Table<void*>::~Table() {
	for (const EntryRecord* entry : entry_records_) {
		delete entry;
	}
	for (Column& column : columns_) {
		delete column.btn;
	}
	multiselect_.clear();
}

/// Add a new column to this table.
void Table<void*>::add_column(uint32_t const width,
                              const std::string& title,
                              const std::string& tooltip_string,
                              Align const alignment,
                              TableColumnType column_type) {
	//  If there would be existing entries, they would not get the new column.
	assert(size() == 0);

	int complete_width = 0;
	for (const Column& col : columns_) {
		complete_width += col.width;
	}

	total_width_ += width;
	set_desired_size(total_width_, get_h());

	{
		Column c;
		// All columns have a title button that is clickable for sorting.
		// The title text can be empty.
		c.btn = new Button(this, title, complete_width, 0, width, headerheight_, button_background_,
		                   title, tooltip_string);
		c.btn->sigclicked.connect(
		   boost::bind(&Table::header_button_clicked, boost::ref(*this), columns_.size()));
		c.width = width;
		c.alignment = alignment;
		c.compare = boost::bind(&Table<void*>::default_compare_string, this, columns_.size(), _1, _2);
		columns_.push_back(c);
		if (column_type == TableColumnType::kFlexible) {
			assert(flexible_column_ == std::numeric_limits<size_t>::max());
			flexible_column_ = columns_.size() - 1;
		}
	}
}

void Table<void*>::set_column_title(uint8_t const col, const std::string& title) {
	assert(col < columns_.size());
	Column& column = columns_.at(col);
	assert(column.btn);
	column.btn->set_title(title);
}

/**
 * Set a custom comparison function for sorting of the given column.
 */
void Table<void*>::set_column_compare(uint8_t col, const Table<void*>::CompareFn& fn) {
	assert(col < columns_.size());
	Column& column = columns_.at(col);
	column.compare = fn;
}

Table<void*>::EntryRecord* Table<void*>::find(const void* const entry) const

{
	for (EntryRecord* temp_entry : entry_records_) {
		if (temp_entry->entry() == entry) {
			return temp_entry;
		}
	}
	return nullptr;
}

/**
 * A header button has been clicked
 */
void Table<void*>::header_button_clicked(Columns::size_type const n) {
	assert(columns_.at(n).btn);
	if (get_sort_colum() == n) {
		set_sort_descending(!get_sort_descending());  //  change sort direction
		sort();
		return;
	}

	set_sort_column(n);
	sort();
	return;
}

/**
 * Remove all entries from the table
*/
void Table<void*>::clear() {
	for (const EntryRecord* entry : entry_records_) {
		delete entry;
	}
	entry_records_.clear();

	if (scrollbar_)
		scrollbar_->set_steps(1);
	scrollpos_ = 0;
	last_click_time_ = -10000;
	clear_selections();
}

void Table<void*>::clear_selections() {
	multiselect_.clear();
	selection_ = no_selection_index();
	last_selection_ = no_selection_index();
}

uint32_t Table<void*>::get_eff_w() const {
	return scrollbar_->is_enabled() ? get_w() - scrollbar_->get_w() : get_w();
}

void Table<void*>::fit_height(uint32_t entries) {
	if (entries == 0) {
		entries = size();
	}
	int tablewidth, tableheight = 0;
	get_desired_size(&tablewidth, &tableheight);
	tableheight = headerheight_ + 2 + get_lineheight() * entries;
	set_desired_size(tablewidth, tableheight);
}

/**
 * Redraw the table
*/
void Table<void*>::draw(RenderTarget& dst) {
	//  draw text lines
	int32_t lineheight = get_lineheight();
	uint32_t idx = scrollpos_ / lineheight;
	int32_t y = 1 + idx * lineheight - scrollpos_ + headerheight_;

	dst.brighten_rect(Rectf(0.f, 0.f, get_eff_w(), get_h()), ms_darken_value);

	while (idx < entry_records_.size()) {
		if (y >= static_cast<int32_t>(get_h()))
			return;

		const EntryRecord& er = *entry_records_[idx];

		if (idx == selection_ || multiselect_.count(idx)) {
			assert(2 <= get_eff_w());
			dst.brighten_rect(Rectf(1.f, y, get_eff_w() - 2, lineheight_), -ms_darken_value);
		}

		Columns::size_type const nr_columns = columns_.size();
		for (uint32_t i = 0, curx = 0; i < nr_columns; ++i) {
			const Column& column = columns_[i];
			int const curw = column.width;
			Align alignment = mirror_alignment(column.alignment);

			const Image* entry_picture = er.get_picture(i);
			const std::string& entry_string = er.get_string(i);

			Vector2f point(curx, y);
			int picw = 0;

			if (entry_picture != nullptr) {
				picw = entry_picture->width();
				const int pich = entry_picture->height();

				float draw_x = point.x;

				// We want a bit of margin
				int max_pic_height = lineheight - 3;

				if (pich > 0 && pich > max_pic_height) {
					// Scale image to fit lineheight
					double image_scale = static_cast<double>(max_pic_height) / pich;
					int blit_width = image_scale * picw;

					if (entry_string.empty()) {
						if (i == nr_columns - 1 && scrollbar_->is_enabled()) {
							draw_x = point.x + (curw - blit_width - scrollbar_->get_w()) / 2.f;
						} else {
							draw_x = point.x + (curw - blit_width) / 2.f;
						}
					}

					if (alignment == UI::Align::kRight) {
						draw_x += curw - blit_width;
					}

					// Create the scaled image
					dst.blitrect_scale(Rectf(draw_x, point.y + 1.f, blit_width, max_pic_height),
					                   entry_picture, Recti(0, 0, picw, pich), 1., BlendMode::UseAlpha);

					// For text alignment below
					picw = blit_width;
				} else {
					if (entry_string.empty()) {
						if (i == nr_columns - 1 && scrollbar_->is_enabled()) {
							draw_x = point.x + (curw - picw - scrollbar_->get_w()) / 2.f;
						} else {
							draw_x = point.x + (curw - picw) / 2.f;
						}
					} else if (alignment == UI::Align::kRight) {
						draw_x += curw - picw;
					}
					dst.blit(Vector2f(draw_x, point.y + (lineheight - pich) / 2.f), entry_picture);
				}
				point.x += picw;
			}

			++picw;  // A bit of margin between image and text

			if (entry_string.empty()) {
				curx += curw;
				continue;
			}
			const Image* entry_text_im = UI::g_fh1->render(as_uifont(richtext_escape(entry_string)));

			switch (alignment) {
			case UI::Align::kCenter:
				point.x += (curw - picw) / 2;
				break;
			case UI::Align::kRight:
				point.x += curw - 2 * picw;
				break;
			case UI::Align::kLeft:
				break;
			}

			// Add an offset for rightmost column when the scrollbar is shown.
			int text_width = entry_text_im->width();
			if (i == nr_columns - 1 && scrollbar_->is_enabled()) {
				text_width = text_width + scrollbar_->get_w();
			}
			UI::correct_for_align(alignment, text_width, &point);

			// Crop to column width while blitting
			if ((curw + picw) < text_width) {
				// Fix positioning for BiDi languages.
				if (UI::g_fh1->fontset()->is_rtl()) {
					point.x = (alignment == UI::Align::kRight) ? curx : curx + picw;
				}
				// We want this always on, e.g. for mixed language savegame filenames
				if (i18n::has_rtl_character(
				       entry_string.c_str(), 20)) {  // Restrict check for efficiency
					dst.blitrect(
					   point, entry_text_im, Recti(text_width - curw + picw, 0, text_width, lineheight));
				} else {
					dst.blitrect(point, entry_text_im, Recti(0, 0, curw - picw, lineheight));
				}
			} else {
				dst.blitrect(point, entry_text_im, Recti(0, 0, curw - picw, lineheight));
			}
			curx += curw;
		}

		y += lineheight;
		++idx;
	}
}

/**
 * handle key presses
 */
bool Table<void*>::handle_key(bool down, SDL_Keysym code) {
	if (is_multiselect_) {
		switch (code.sym) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			shift_down_ = down;
			break;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			ctrl_down_ = down;
			break;
		default:
			break;
		}
	}
	if (down) {
		switch (code.sym) {
		case SDLK_a:
			if (is_multiselect_ && ctrl_down_ && !empty()) {
				multiselect_.clear();
				for (uint32_t i = 0; i < size(); ++i) {
					toggle_entry(i);
				}
				selection_ = 0;
				selected(0);
				return true;
			}
			break;
		case SDLK_UP:
		case SDLK_KP_8:
			move_selection(-1);
			return true;

		case SDLK_DOWN:
		case SDLK_KP_2:
			move_selection(1);
			return true;

		default:
			break;  // not handled
		}
	}

	return UI::Panel::handle_key(down, code);
}

bool Table<void*>::handle_mousewheel(uint32_t which, int32_t x, int32_t y) {
	return scrollbar_->handle_mousewheel(which, x, y);
}

/**
 * Handle mouse presses: select the appropriate entry
 */
bool Table<void*>::handle_mousepress(uint8_t const btn, int32_t, int32_t const y) {
	if (get_can_focus())
		focus();

	switch (btn) {
	case SDL_BUTTON_LEFT: {
		uint32_t const time = SDL_GetTicks();

		//  This hick hack is needed if any of the callback functions calls clear
		//  to forget the last clicked time.
		uint32_t const real_last_click_time = last_click_time_;

		last_selection_ = selection_;
		last_click_time_ = time;

		uint32_t const row = (y + scrollpos_ - headerheight_) / get_lineheight();
		if (row < entry_records_.size()) {
			play_click();
			multiselect(row);
		}

		// Check if doubleclicked
		if (!ctrl_down_ && !shift_down_ && time - real_last_click_time < DOUBLE_CLICK_INTERVAL &&
		    last_selection_ == selection_ && selection_ != no_selection_index()) {
			double_clicked(selection_);
		}
		return true;
	}
	default:
		return false;
	}
}

/**
 * move the currently selected entry up or down.
 * \param offset positive value move the selection down and
 *        negative values up.
 */
void Table<void*>::move_selection(const int32_t offset) {
	if (!has_selection())
		return;
	int32_t new_selection = (is_multiselect_ ? last_multiselect_ : selection_) + offset;

	if (new_selection < 0)
		new_selection = 0;
	else if (static_cast<uint32_t>(new_selection) > entry_records_.size() - 1)
		new_selection = entry_records_.size() - 1;

	multiselect(new_selection);

	// Scroll to newly selected entry
	if (scrollbar_) {
		// Keep an unselected item above or below
		int32_t scroll_item = new_selection + offset;
		if (scroll_item < 0)
			scroll_item = 0;
		if (scroll_item > static_cast<int32_t>(entry_records_.size())) {
			scroll_item = entry_records_.size();
		}

		// Ensure scroll_item is visible
		if (static_cast<int32_t>(scroll_item * get_lineheight()) < scrollpos_) {
			scrollbar_->set_scrollpos(scroll_item * get_lineheight());
		} else if (static_cast<int32_t>((scroll_item + 1) * get_lineheight() - get_inner_h()) >
		           scrollpos_) {
			scrollbar_->set_scrollpos((scroll_item + 1) * get_lineheight() - get_inner_h());
		}
	}
}

/**
 * Change the currently selected entry
 *
 * Args: i  the entry to select
 */
void Table<void*>::select(const uint32_t i) {
	if (empty() || selection_ == i || i == no_selection_index())
		return;

	selection_ = i;
	if (is_multiselect_) {
		multiselect_.insert(selection_);
	}

	selected(selection_);
}

void Table<void*>::multiselect(uint32_t row) {
	if (is_multiselect_) {
		// Ranged selection with Shift
		if (shift_down_) {
			multiselect_.clear();
			if (has_selection()) {
				const uint32_t last_selected = selection_index();
				const uint32_t lower_bound = std::min(row, selection_);
				const uint32_t upper_bound = std::max(row, selection_);
				for (uint32_t i = lower_bound; i <= upper_bound; ++i) {
					toggle_entry(i);
				}
				select(last_selected);
			} else {
				select(toggle_entry(row));
			}
		} else {
			// Single selection without Ctrl
			if (!ctrl_down_) {
				multiselect_.clear();
			}
			select(toggle_entry(row));
		}
		last_multiselect_ = row;
	} else {
		select(row);
	}
}

/**
 * Adds/removes the row from multiselect.
 * Returns the row that should be selected afterwards, or no_selection_index() if
 * the multiselect is empty.
 */
uint32_t Table<void*>::toggle_entry(uint32_t row) {
	assert(is_multiselect_);
	if (multiselect_.count(row)) {
		multiselect_.erase(row);
		// Find last selection
		if (multiselect_.empty()) {
			return no_selection_index();
		} else {
			return *multiselect_.lower_bound(0);
		}
	} else {
		multiselect_.insert(row);
		return row;
	}
}

/**
 * Add a new entry to the table.
*/
Table<void*>::EntryRecord& Table<void*>::add(void* const entry, const bool do_select) {
	EntryRecord& result = *new EntryRecord(entry);
	entry_records_.push_back(&result);
	result.data_.resize(columns_.size());

	if (do_select) {
		select(entry_records_.size() - 1);
		scrollbar_->set_scrollpos(std::numeric_limits<int32_t>::max());
	}
	layout();
	return result;
}

/**
 * Scroll to the given position, in pixels.
*/
void Table<void*>::set_scrollpos(int32_t const i) {
	scrollpos_ = i;
}

/**
 * Remove the table entry at the given (zero-based) index.
 */
void Table<void*>::remove(const uint32_t i) {
	assert(i < entry_records_.size());
	multiselect_.clear();

	const EntryRecordVector::iterator it = entry_records_.begin() + i;
	delete *it;
	entry_records_.erase(it);
	if (selection_ == i) {
		selection_ = no_selection_index();
	} else if (selection_ > i && selection_ != no_selection_index()) {
		selection_--;
	}
	if (is_multiselect_ && selection_ != no_selection_index()) {
		multiselect_.insert(selection_);
	}
	layout();
}

bool Table<void*>::sort_helper(uint32_t a, uint32_t b) {
	if (sort_descending_) {
		return columns_[sort_column_].compare(b, a);
	} else {
		return columns_[sort_column_].compare(a, b);
	}
}

void Table<void*>::layout() {
	if (columns_.empty()) {
		return;
	}

	// Position and update the scrollbar
	scrollbar_->set_pos(Vector2i(get_w() - Scrollbar::kSize, headerheight_));
	scrollbar_->set_size(scrollbar_->get_w(), get_h() - headerheight_);
	scrollbar_->set_pagesize(get_h() - 2 * get_lineheight() - headerheight_);
	scrollbar_->set_steps(entry_records_.size() * get_lineheight() - (get_h() - headerheight_ - 2));

	// Find a column to resize
	size_t resizeable_column = std::numeric_limits<size_t>::max();
	if (flexible_column_ < columns_.size()) {
		resizeable_column = flexible_column_;
	} else {
		// Use the widest column
		uint32_t widest_width = columns_[resizeable_column].width;
		for (size_t i = 1; i < columns_.size(); ++i) {
			const uint32_t width = columns_[i].width;
			if (width > widest_width) {
				widest_width = width;
				resizeable_column = i;
			}
		}
	}

	// If we have a resizeable column, adjust the column sizes.
	if (resizeable_column != std::numeric_limits<size_t>::max()) {
		int all_columns_width = scrollbar_->is_enabled() ? scrollbar_->get_w() : 0;
		for (const auto& column : columns_) {
			all_columns_width += column.width;
		}
		if (all_columns_width != get_w()) {
			Column& column = columns_.at(resizeable_column);
			column.width = std::max(0, column.width + get_w() - all_columns_width);
			column.btn->set_size(column.width, column.btn->get_h());

			int offset = 0;
			for (const auto& col : columns_) {
				col.btn->set_pos(Vector2i(offset, col.btn->get_y()));
				offset = col.btn->get_x() + col.btn->get_w();
			}

			if (scrollbar_->is_enabled()) {
				const UI::Button* last_column_btn = columns_.back().btn;
				scrollbar_filler_button_->set_pos(
				   Vector2i(last_column_btn->get_x() + last_column_btn->get_w(), 0));
				scrollbar_filler_button_->set_visible(true);
			} else {
				scrollbar_filler_button_->set_visible(false);
			}
		}
	}
}

/**
 * Sort the table alphabetically, or if set_column_compare has been set,
 * according to its compare function.
 * Make sure that the current selection stays
 * valid (though it might scroll out of visibility).
 * Only the subarea [lower_bound, upper_bound) is sorted.
 * For example, you might want to sort directories for themselves at the
 * top of the list, and files at the bottom.
 */
void Table<void*>::sort(const uint32_t lower_bound, uint32_t upper_bound) {
	assert(columns_.at(sort_column_).btn);
	assert(sort_column_ < columns_.size());

	if (upper_bound > size())
		upper_bound = size();

	std::vector<uint32_t> indices;
	std::vector<EntryRecord*> copy;

	indices.reserve(upper_bound - lower_bound);
	copy.reserve(upper_bound - lower_bound);
	for (uint32_t i = lower_bound; i < upper_bound; ++i) {
		indices.push_back(i);
		copy.push_back(entry_records_[i]);
	}

	std::stable_sort(
	   indices.begin(), indices.end(), boost::bind(&Table<void*>::sort_helper, this, _1, _2));

	uint32_t newselection = selection_;
	std::set<uint32_t> new_multiselect;
	for (uint32_t i = lower_bound; i < upper_bound; ++i) {
		uint32_t from = indices[i - lower_bound];
		entry_records_[i] = copy[from - lower_bound];
		if (selection_ == from) {
			newselection = i;
		}
		if (is_multiselect_ && multiselect_.count(from) == 1) {
			new_multiselect.insert(i);
		}
	}
	selection_ = newselection;
	if (is_multiselect_) {
		multiselect_.clear();
		for (const uint32_t entry : new_multiselect) {
			multiselect_.insert(entry);
		}
	}
}

bool Table<void*>::default_compare_string(uint32_t column, uint32_t a, uint32_t b) {
	EntryRecord& ea = get_record(a);
	EntryRecord& eb = get_record(b);
	return ea.get_string(column) < eb.get_string(column);
}

Table<void*>::EntryRecord::EntryRecord(void* const e) : entry_(e) {
}

void Table<void*>::EntryRecord::set_picture(uint8_t const col,
                                            const Image* pic,
                                            const std::string& str) {
	assert(col < data_.size());

	data_.at(col).d_picture = pic;
	data_.at(col).d_string = str;
}
void Table<void*>::EntryRecord::set_string(uint8_t const col, const std::string& str) {
	assert(col < data_.size());

	data_.at(col).d_picture = nullptr;
	data_.at(col).d_string = str;
}
const Image* Table<void*>::EntryRecord::get_picture(uint8_t const col) const {
	assert(col < data_.size());

	return data_.at(col).d_picture;
}
const std::string& Table<void*>::EntryRecord::get_string(uint8_t const col) const {
	assert(col < data_.size());

	return data_.at(col).d_string;
}
}
