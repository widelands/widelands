/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "ui_basic/table.h"

#include <memory>

#include <SDL_mouse.h>
#include <SDL_timer.h>

#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"
#include "graphic/texture.h"
#include "ui_basic/mouse_constants.h"
#include "wlapplication_options.h"

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
                    PanelStyle style,
                    TableRows rowtype)
   : Panel(parent, style, x, y, w, h),
     total_width_(0),
     lineheight_(text_height(g_style_manager->table_style(style).enabled())),
     headerheight_(lineheight_ + 4),
     button_style_(style == UI::PanelStyle::kFsMenu ? UI::ButtonStyle::kFsMenuMenu :
                                                      UI::ButtonStyle::kWuiSecondary),
     scrollbar_(nullptr),
     scrollbar_filler_button_(
        new Button(this, "", 0, 0, Scrollbar::kSize, headerheight_, button_style_, "")),
     scrollpos_(0),
     selection_(no_selection_index()),
     last_multiselect_(no_selection_index()),
     last_click_time_(-10000),
     last_selection_(no_selection_index()),
     sort_column_(0),
     sort_descending_(rowtype == TableRows::kSingleDescending ||
                      rowtype == TableRows::kMultiDescending),
     flexible_column_idx_(std::numeric_limits<size_t>::max()),
     is_multiselect_(rowtype == TableRows::kMulti || rowtype == TableRows::kMultiDescending) {
	set_thinks(false);
	set_can_focus(true);
	scrollbar_filler_button_->set_visible(false);
	scrollbar_ = new Scrollbar(this, get_w() - Scrollbar::kSize, headerheight_, Scrollbar::kSize,
	                           get_h() - headerheight_, style);
	scrollbar_->moved.connect([this](int32_t a) { set_scrollpos(a); });
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

size_t Table<void*>::number_of_columns() const {
	return columns_.size();
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
		c.btn = new Button(this, title, complete_width, 0, width, headerheight_, button_style_, title,
		                   tooltip_string);
		const size_t col_index = columns_.size();
		c.btn->sigclicked.connect([this, col_index]() { header_button_clicked(col_index); });
		c.width = width;
		c.original_width = width;
		c.alignment = alignment;
		c.compare = [this, col_index](
		               uint32_t a, uint32_t b) { return default_compare_string(col_index, a, b); };
		columns_.push_back(c);
		if (column_type == TableColumnType::kFlexible) {
			assert(flexible_column_idx_ == std::numeric_limits<size_t>::max());
			flexible_column_idx_ = col_index;
		}
	}
}

void Table<void*>::set_column_title(uint8_t const col, const std::string& title) {
	assert(col < columns_.size());
	Column& column = columns_.at(col);
	assert(column.btn);
	column.btn->set_title(title);
}

void Table<void*>::set_column_tooltip(uint8_t col, const std::string& text) {
	assert(col < columns_.size());
	Column& column = columns_.at(col);
	assert(column.btn);
	column.btn->set_tooltip(text);
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
}

/**
 * Remove all entries from the table
 */
void Table<void*>::clear() {
	for (const EntryRecord* entry : entry_records_) {
		delete entry;
	}
	entry_records_.clear();

	if (scrollbar_ != nullptr) {
		scrollbar_->set_steps(1);
	}
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
	int tablewidth;
	int tableheight = 0;
	get_desired_size(&tablewidth, &tableheight);
	tableheight = headerheight_ + 2 + get_lineheight() * entries;
	set_desired_size(tablewidth, tableheight);
}

std::vector<Recti> Table<void*>::focus_overlay_rects() {
	if (!has_selection()) {
		return Panel::focus_overlay_rects();
	}
	const int f = g_style_manager->focus_border_thickness();
	const int32_t w = get_eff_w();
	const int32_t lineheight = get_lineheight();
	int32_t y = headerheight_ + lineheight * selection_index() - scrollpos_;
	int32_t h = lineheight;
	if (y < static_cast<int>(headerheight_)) {
		h -= (headerheight_ - y);
		y = headerheight_;
		if (h < f) {
			return Panel::focus_overlay_rects();
		}
	}
	if (y + h > get_h()) {
		h -= (y + h - get_h());
		if (h < f) {
			return Panel::focus_overlay_rects();
		}
	}
	return {Recti(0, y, w, f), Recti(0, y + h - f, w, f), Recti(0, y + f, f, h - 2 * f),
	        Recti(w - f, y + f, f, h - 2 * f)};
}

/**
 * Redraw the table
 */
void Table<void*>::draw(RenderTarget& dst) {
	//  draw text lines
	int32_t lineheight = get_lineheight();
	uint32_t idx = scrollpos_ / lineheight;
	int32_t y = 1 + idx * lineheight - scrollpos_ + headerheight_;

	dst.brighten_rect(Recti(0, 0, get_eff_w(), get_h()), ms_darken_value);

	while (idx < entry_records_.size()) {
		if (y >= static_cast<int32_t>(get_h())) {
			return;
		}

		const EntryRecord& er = *entry_records_[idx];

		if (idx == selection_ || (multiselect_.count(idx) != 0u)) {
			assert(2 <= get_eff_w());
			dst.brighten_rect(Recti(1, y, get_eff_w() - 2, lineheight_), -ms_darken_value);
		}

		Columns::size_type const nr_columns = columns_.size();
		for (uint32_t i = 0, curx = 0; i < nr_columns; ++i) {
			const Column& column = columns_[i];
			const int curw = column.width;
			Align alignment = mirror_alignment(column.alignment, g_fh->fontset()->is_rtl());

			const Image* entry_picture = er.get_picture(i);
			const std::string& entry_string = er.get_string(i);

			Vector2i point(curx, y);
			int picw = 0;

			if (entry_picture != nullptr) {
				picw = entry_picture->width();
				const int pich = entry_picture->height();

				int draw_x = point.x;

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
							draw_x = point.x + (curw - blit_width) / 2;
						}
					}

					if (alignment == UI::Align::kRight) {
						draw_x += curw - blit_width;
					}

					// Create the scaled image
					dst.blitrect_scale(Rectf(draw_x, point.y + 1, blit_width, max_pic_height),
					                   entry_picture, Recti(0, 0, picw, pich), 1., BlendMode::UseAlpha);

					// For text alignment below
					picw = blit_width;
				} else {
					if (entry_string.empty()) {
						if (i == nr_columns - 1 && scrollbar_->is_enabled()) {
							draw_x = point.x + (curw - picw - scrollbar_->get_w()) / 2.f;
						} else {
							draw_x = point.x + (curw - picw) / 2;
						}
					} else if (alignment == UI::Align::kRight) {
						draw_x += curw - picw;
					}
					dst.blit(Vector2i(draw_x, point.y + (lineheight - pich) / 2), entry_picture);
				}
				if (alignment != Align::kRight) {
					point.x += picw;
				}
			}

			if (entry_string.empty()) {
				curx += curw;
				continue;
			}

			const UI::FontStyleInfo& font_style = get_column_fontstyle(er);
			std::shared_ptr<const RenderedText> rendered_text =
			   g_fh->render(as_richtext_paragraph(richtext_escape(entry_string), font_style));

			// Fix text alignment for BiDi languages if the entry contains an RTL character. We want
			// this always on, e.g. for mixed language savegame filenames.
			alignment =
			   mirror_alignment(column.alignment, i18n::has_rtl_character(entry_string.c_str(), 20));

			// Position the text according to alignment
			switch (alignment) {
			case UI::Align::kCenter:
				point.x += (curw - picw) / 2;
				break;
			case UI::Align::kRight:
				point.x += curw - picw;
				break;
			case UI::Align::kLeft:
				break;
			}

			constexpr int kMargin = 1;
			rendered_text->draw(dst, point, Recti(kMargin, 0, curw - picw - 2 * kMargin, lineheight),
			                    alignment, RenderedText::CropMode::kSelf);
			curx += curw;
		}

		y += lineheight;
		++idx;
	}
}

bool Table<void*>::handle_tooltip() {
	int32_t lineheight = get_lineheight();
	uint32_t idx = scrollpos_ / lineheight;
	int32_t y = 1 + idx * lineheight - scrollpos_ + headerheight_;

	Vector2i cursor_pos = get_mouse_position();
	for (uint32_t row = idx; row < entry_records_.size(); ++row) {
		const EntryRecord& er = *entry_records_[row];
		for (uint32_t c = 0, column_x = 0; c < columns_.size(); ++c) {

			const int column_w = columns_[c].width;
			Vector2i point(column_x, y);
			if (is_mouse_in(cursor_pos, point, column_w)) {
				const std::string& entry_string = er.get_string(c);
				FontStyleInfo& font_style = get_column_fontstyle(er);
				std::shared_ptr<const UI::RenderedText> rendered_text =
				   UI::g_fh->render(as_richtext_paragraph(richtext_escape(entry_string), font_style));

				if (rendered_text->width() > column_w) {
					return Panel::draw_tooltip(entry_string, panel_style_);
				}
			}
			column_x += column_w;
		}
		y += lineheight;
	}

	return true;
}

UI::FontStyleInfo& Table<void*>::get_column_fontstyle(const Table<void*>::EntryRecord& er) {
	return const_cast<FontStyleInfo&>(
	   er.font_style() != nullptr ? *er.font_style() :
	   er.is_disabled()           ? g_style_manager->table_style(panel_style_).disabled() :
                                   g_style_manager->table_style(panel_style_).enabled());
}
bool Table<void*>::is_mouse_in(const Vector2i& cursor_pos,
                               const Vector2i& point,
                               const int column_width) const {
	const int line = get_lineheight();

	return cursor_pos.x >= point.x && cursor_pos.x <= point.x + column_width &&
	       cursor_pos.y > point.y && cursor_pos.y < point.y + line;
}

/**
 * handle key presses
 */
bool Table<void*>::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		if (is_multiselect_ && !empty() &&
		    matches_shortcut(KeyboardShortcut::kCommonSelectAll, code)) {
			multiselect_.clear();
			for (uint32_t i = 0; i < size(); ++i) {
				toggle_entry(i);
			}
			selection_ = 0;
			selected(0);
			return true;
		}

		switch (code.sym) {
		case SDLK_ESCAPE:
			cancel();
			return true;

		case SDLK_TAB:
			// Let the panel handle the tab key
			return get_parent()->handle_key(true, code);

		case SDLK_RETURN:
			if (selection_ != no_selection_index()) {
				double_clicked(selection_);
			}
			return true;

		case SDLK_UP:
			move_selection(-1);
			return true;

		case SDLK_DOWN:
			move_selection(1);
			return true;

		case SDLK_PAGEDOWN:
			move_selection(get_h() / get_lineheight());
			return true;

		case SDLK_PAGEUP: {
			const int32_t sel = get_h() / get_lineheight();
			move_selection(-1 * sel);
			return true;
		}

		case SDLK_HOME:
			multiselect(0);
			scroll_to_item(0);
			return true;

		case SDLK_END: {
			const uint32_t sel = entry_records_.size() - 1;
			multiselect(sel);
			scroll_to_item(sel);
			return true;
		}

		default:
			break;  // not handled
		}
	}

	return UI::Panel::handle_key(down, code);
}

bool Table<void*>::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	return scrollbar_->handle_mousewheel(x, y, modstate);
}

/**
 * Handle mouse presses: select the appropriate entry
 */
bool Table<void*>::handle_mousepress(uint8_t const btn, int32_t /*x*/, int32_t const y) {
	if (get_can_focus()) {
		focus();
	}

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
		if (((SDL_GetModState() & (KMOD_CTRL | KMOD_SHIFT)) == 0) &&
		    time - real_last_click_time < DOUBLE_CLICK_INTERVAL && last_selection_ == selection_ &&
		    selection_ != no_selection_index()) {
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
	int32_t new_selection =
	   has_selection() ? (is_multiselect_ ? last_multiselect_ : selection_) + offset : 0;

	if (new_selection < 0) {
		new_selection = 0;
	} else if (static_cast<uint32_t>(new_selection) > entry_records_.size() - 1) {
		new_selection = entry_records_.size() - 1;
	}

	multiselect(new_selection);
	// Scroll to newly selected entry
	scroll_to_item(new_selection);
}

/**
 * Change the currently selected entry
 *
 * Args: i  the entry to select
 */
void Table<void*>::select(const uint32_t i) {
	if (empty() || i == no_selection_index()) {
		return;
	}

	selection_ = i;
	if (is_multiselect_) {
		multiselect_.insert(selection_);
		last_multiselect_ = selection_;
	}

	selected(selection_);
}

/**
 * If 'force' is true, adds the given 'row' to the selection, ignoring everything else.
 */
void Table<void*>::multiselect(uint32_t row, bool force) {
	if (force) {
		select(row);
		return;
	}
	if (is_multiselect_) {
		// Ranged selection with Shift
		if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
			multiselect_.clear();
			if (has_selection()) {
				const uint32_t last_selected = selection_index();
				const uint32_t lower_bound = std::min(row, selection_);
				const uint32_t upper_bound = std::max(row, selection_);
				for (uint32_t i = lower_bound; i <= upper_bound; ++i) {
					toggle_entry(i);
				}
				select(last_selected);
				selected(last_selected);
			} else {
				select(toggle_entry(row));
			}
		} else {
			// Single selection without Ctrl
			if ((SDL_GetModState() & KMOD_CTRL) == 0) {
				multiselect_.clear();
			}
			select(toggle_entry(row));
		}
		last_multiselect_ = row;
	} else {
		select(row);
	}
}

// Scroll to the given item. Out of range items will be corrected automatically.
void Table<void*>::scroll_to_item(int32_t item) {
	if (scrollbar_ != nullptr) {
		// Correct out of range items
		if (item < 0) {
			item = 0;
		} else if (item >= static_cast<int32_t>(entry_records_.size())) {
			item = entry_records_.size() - 1;
		}

		// Ensure item is visible
		if (static_cast<int32_t>(item * get_lineheight()) < scrollpos_) {
			scrollbar_->set_scrollpos(item * get_lineheight());
		} else if (static_cast<int32_t>((item + 2) * get_lineheight() - get_inner_h()) > scrollpos_) {
			scrollbar_->set_scrollpos((item + 3) * get_lineheight() - get_inner_h());
		}
	}
}

/**
 * Adds/removes the row from multiselect.
 * Returns the row that should be selected afterwards, or no_selection_index() if
 * the multiselect is empty.
 */
uint32_t Table<void*>::toggle_entry(uint32_t row) {
	assert(is_multiselect_);
	if (multiselect_.count(row) != 0u) {
		multiselect_.erase(row);
		// Find last selection
		if (multiselect_.empty()) {
			return no_selection_index();
		}
		return *multiselect_.lower_bound(0);
	}
	multiselect_.insert(row);
	return row;
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

void Table<void*>::scroll_to_top() {
	scrollbar_->set_scrollpos(0);
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
	if (is_multiselect_) {
		if (selection_ != no_selection_index()) {
			multiselect_.insert(selection_);
		}
		last_multiselect_ = selection_;
	}
	layout();
}

/**
 * Remove the given table entry if it exists.
 */
void Table<void*>::remove_entry(const void* const entry) {
	for (uint32_t i = 0; i < entry_records_.size(); ++i) {
		if (entry_records_[i]->entry() == entry) {
			remove(i);
			return;
		}
	}
}

bool Table<void*>::sort_helper(uint32_t a, uint32_t b) {
	if (sort_descending_) {
		return columns_[sort_column_].compare(b, a);
	}
	return columns_[sort_column_].compare(a, b);
}

void Table<void*>::layout() {
	if (columns_.empty() || get_h() == 0 || get_w() == 0) {
		return;
	}

	reposition_scrollbar();

	size_t resizeable_column_idx = find_resizable_column_idx();

	int all_columns_width = total_columns_width();

	if (all_columns_width != get_w()) {
		adjust_column_sizes(all_columns_width, resizeable_column_idx);
		update_scrollbar_filler();
	}
}

void Table<void*>::reposition_scrollbar() {
	scrollbar_->set_pos(Vector2i(get_w() - Scrollbar::kSize, headerheight_));
	scrollbar_->set_size(scrollbar_->get_w(), get_h() - headerheight_);
	scrollbar_->set_pagesize(get_h() - 2 * get_lineheight() - headerheight_);
	scrollbar_->set_steps(entry_records_.size() * get_lineheight() - (get_h() - headerheight_ - 2));
}
size_t Table<void*>::find_resizable_column_idx() {

	if (flexible_column_idx_ < columns_.size()) {
		return flexible_column_idx_;
	}  // Use the widest column
	size_t widest_column_idx = 0;
	uint32_t widest_width = columns_[0].width;
	for (size_t i = 1; i < columns_.size(); ++i) {
		const uint32_t width = columns_[i].width;
		if (width > widest_width) {
			widest_width = width;
			widest_column_idx = i;
		}
	}
	return widest_column_idx;
}

int Table<void*>::total_columns_width() {
	int all_columns_width = scrollbar_->is_enabled() ? scrollbar_->get_w() : 0;
	for (const auto& column : columns_) {
		all_columns_width += column.width;
	}
	return all_columns_width;
}

void Table<void*>::adjust_column_sizes(int all_columns_width, size_t resizeable_column_idx) {
	Column& resizable_col = columns_.at(resizeable_column_idx);
	resizable_col.width = std::max(0, resizable_col.width + get_w() - all_columns_width);
	resizable_col.btn->set_size(resizable_col.width, resizable_col.btn->get_h());

	int offset = 0;
	for (size_t i = 0; i < columns_.size(); ++i) {
		Column& c = columns_.at(i);
		// make sure all columns which were not intended to be resizable get their original width back
		if (c.width != c.original_width && i != resizeable_column_idx) {
			c.width = c.original_width;
			c.btn->set_size(c.width, c.btn->get_h());
		}

		c.btn->set_pos(Vector2i(offset, c.btn->get_y()));
		offset = c.btn->get_x() + c.btn->get_w();
	}
}

void Table<void*>::update_scrollbar_filler() {
	if (scrollbar_->is_enabled()) {
		const UI::Button* last_column_btn = columns_.back().btn;
		scrollbar_filler_button_->set_pos(
		   Vector2i(last_column_btn->get_x() + last_column_btn->get_w(), 0));
		scrollbar_filler_button_->set_visible(true);
	} else {
		scrollbar_filler_button_->set_visible(false);
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

	if (upper_bound > size()) {
		upper_bound = size();
	}

	std::vector<uint32_t> indices;
	std::vector<EntryRecord*> copy;

	indices.reserve(upper_bound - lower_bound);
	copy.reserve(upper_bound - lower_bound);
	for (uint32_t i = lower_bound; i < upper_bound; ++i) {
		indices.push_back(i);
		copy.push_back(entry_records_[i]);
	}

	std::stable_sort(
	   indices.begin(), indices.end(), [this](uint32_t a, uint32_t b) { return sort_helper(a, b); });

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

bool Table<void*>::default_compare_string(uint32_t column, uint32_t a, uint32_t b) const {
	const EntryRecord& ea = get_record(a);
	const EntryRecord& eb = get_record(b);
	return ea.get_string(column) < eb.get_string(column);
}
bool Table<void*>::handle_mousemove(
   uint8_t /*state*/, int32_t /*x*/, int32_t /*y*/, int32_t /*xdiff*/, int32_t /*ydiff*/) {
	// needed to activate tooltip rendering without providing tooltiptext to parent (panel) class
	return true;
}

Table<void*>::EntryRecord::EntryRecord(void* const e)
   : entry_(e), font_style_(nullptr), disabled_(false) {
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

}  // namespace UI
