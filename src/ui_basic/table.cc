/*
 * Copyright (C) 2002, 2006-2011 by the Widelands Development Team
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
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
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
Table<void *>::Table
	(Panel * const parent,
	 int32_t x, int32_t y, uint32_t w, uint32_t h,
	 const bool descending)
:
	Panel             (parent, x, y, w, h),
	m_total_width     (0),
	m_fontsize        (UI_FONT_SIZE_SMALL),
	m_headerheight    (UI::g_fh1->render(as_uifont(".", m_fontsize))->height() + 4),
	m_lineheight      (UI::g_fh1->render(as_uifont(".", m_fontsize))->height()),
	m_scrollbar       (nullptr),
	m_scrollpos       (0),
	m_selection       (no_selection_index()),
	m_last_click_time (-10000),
	m_last_selection  (no_selection_index()),
	m_sort_column     (0),
	m_sort_descending (descending)
{
	set_thinks(false);
	set_can_focus(true);
}


/**
 * Free allocated resources
*/
Table<void *>::~Table()
{
	for (const EntryRecord * entry : m_entry_records) {
		delete entry;
	}
}

/// Add a new column to this table.
void Table<void *>::add_column
	(uint32_t            const width,
	 const std::string &       title,
	 const std::string &       tooltip_string,
	 Align               const alignment,
	 bool                const is_checkbox_column)
{
	//  If there would be existing entries, they would not get the new column.
	assert(size() == 0);

	uint32_t complete_width = 0;
	for (const Column& col : m_columns) {
		complete_width += col.width;
	}

	m_total_width += width;
	set_desired_size(m_total_width, get_h());

	{
		Column c;
		c.btn = nullptr;
		if (title.size()) {
			c.btn =
				new Button
					(this, title,
					 complete_width, 0, width, m_headerheight,
					 g_gr->images().get("pics/but3.png"),
					 title, tooltip_string, true, false);
			c.btn->sigclicked.connect
				(boost::bind(&Table::header_button_clicked, boost::ref(*this), m_columns.size()));
		}
		c.width = width;
		c.alignment = alignment;
		c.is_checkbox_column = is_checkbox_column;

		if (is_checkbox_column) {
			c.compare = boost::bind
				(&Table<void *>::default_compare_checkbox,
				 this, m_columns.size(), _1, _2);
		} else {
			c.compare = boost::bind
				(&Table<void *>::default_compare_string,
				 this, m_columns.size(), _1, _2);
		}

		m_columns.push_back(c);
	}
	if (!m_scrollbar) {
		m_scrollbar =
			new Scrollbar
				(get_parent(),
				 get_x() + get_w() - 24, get_y() + m_headerheight,
				 24,                     get_h() - m_headerheight,
				 false);
		m_scrollbar->moved.connect(boost::bind(&Table::set_scrollpos, this, _1));
		m_scrollbar->set_steps(1);
		m_scrollbar->set_singlestepsize(m_lineheight);
		m_scrollbar->set_pagesize(get_h() - m_lineheight);
	}
}

void Table<void *>::set_column_title(uint8_t const col, const std::string & title)
{
	assert(col < m_columns.size());
	Column & column = m_columns.at(col);
	if (!column.btn && !title.empty()) { //  no title before, but now
		uint32_t complete_width = 0;
		for (uint8_t i = 0; i < col; ++i)
			complete_width += m_columns.at(i).width;
		column.btn =
			new Button
				(this, title,
				 complete_width, 0, column.width, m_headerheight,
				 g_gr->images().get("pics/but3.png"),
				 title, "", true, false);
		column.btn->sigclicked.connect
			(boost::bind(&Table::header_button_clicked, boost::ref(*this), col));
	} else if (title.empty()) { //  had title before, not now
		if (column.btn) {
			delete column.btn;
			column.btn = nullptr;
		}
	} else
		column.btn->set_title(title);
}

/**
 * Set a custom comparison function for sorting of the given column.
 */
void Table<void *>::set_column_compare
	(uint8_t col, const Table<void *>::CompareFn & fn)
{
	assert(col < m_columns.size());
	Column & column = m_columns.at(col);
	column.compare = fn;
}

void Table<void *>::EntryRecord::set_checked
	(uint8_t const col, bool const checked)
{
	_data & cell = m_data.at(col);

	cell.d_checked = checked;
	cell.d_picture =
		g_gr->images().get(checked ? "pics/checkbox_checked.png" : "pics/checkbox_empty.png");
}

void Table<void *>::EntryRecord::toggle(uint8_t const col)
{
	set_checked(col, !is_checked(col));
}


bool Table<void *>::EntryRecord::is_checked(uint8_t const col) const {
	const _data & cell = m_data.at(col);

	return cell.d_checked;
}

Table<void *>::EntryRecord * Table<void *>::find
	(const void * const entry) const

{
	for (EntryRecord * temp_entry : m_entry_records) {
		if (temp_entry->entry() == entry) {
			return temp_entry;
		}
	}
	return nullptr;
}

/**
 * A header button has been clicked
 */
void Table<void *>::header_button_clicked(Columns::size_type const n) {
	assert(m_columns.at(n).btn);
	if (get_sort_colum() == n) {
		set_sort_descending(!get_sort_descending()); //  change sort direction
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
void Table<void *>::clear()
{
	for (const EntryRecord * entry : m_entry_records) {
		delete entry;
	}
	m_entry_records.clear();

	if (m_scrollbar)
		m_scrollbar->set_steps(1);
	m_scrollpos = 0;
	m_selection = no_selection_index();
	m_last_click_time = -10000;
	m_last_selection = no_selection_index();
}


void Table<void *>::fit_height(uint32_t entries) {
	if (entries == 0) {
		entries = size();
	}
	uint32_t tablewidth;
	uint32_t tableheight;
	get_desired_size(tablewidth, tableheight);
	tableheight = m_headerheight + 2 + get_lineheight() * entries;
	set_desired_size(tablewidth, tableheight);
}

/**
 * Redraw the table
*/
void Table<void *>::draw(RenderTarget & dst)
{
	//  draw text lines
	int32_t lineheight = get_lineheight();
	uint32_t idx = m_scrollpos / lineheight;
	int32_t y = 1 + idx * lineheight - m_scrollpos + m_headerheight;

	dst.brighten_rect(Rect(Point(0, 0), get_w(), get_h()), ms_darken_value);

	while (idx < m_entry_records.size()) {
		if (y >= static_cast<int32_t>(get_h()))
			return;

		const EntryRecord & er = *m_entry_records[idx];

		if (idx == m_selection) {
			assert(2 <= get_eff_w());
			dst.brighten_rect
				(Rect(Point(1, y), get_eff_w() - 2, m_lineheight),
				 -ms_darken_value);
		}

		Columns::size_type const nr_columns = m_columns.size();
		for (uint32_t i = 0, curx = 0; i < nr_columns; ++i) {
			const Column & column    = m_columns[i];
			uint32_t const curw      = column.width;
			Align alignment = mirror_alignment(column.alignment);

			const Image* entry_picture = er.get_picture(i);
			const std::string& entry_string = er.get_string(i);

			Point point(curx, y);
			int picw = 0;
			int pich = 0;

			if (entry_picture) {
				picw = entry_picture->width();
				pich = entry_picture->height();

				int draw_x = point.x;

				if (pich > 0 && pich > lineheight) {
					// Scale image to fit lineheight
					double image_scale = static_cast<double>(lineheight) / pich;
					int blit_width = image_scale * picw;

					if (entry_string.empty()) {
						if (i == nr_columns - 1 && m_scrollbar->is_enabled()) {
							draw_x = point.x + (curw - blit_width - m_scrollbar->get_w()) / 2;
						} else {
							draw_x = point.x + (curw - blit_width) / 2;
						}
					}

					dst.blitrect_scale(
								// Center align if text is empty
								Rect(draw_x,
									  point.y,
									  blit_width,
									  lineheight),
								entry_picture,
								Rect(0, 0, picw, pich),
								1.,
								BlendMode::UseAlpha);
					picw = blit_width;
				} else {
					if (entry_string.empty()) {
						if (i == nr_columns - 1 && m_scrollbar->is_enabled()) {
							draw_x = point.x + (curw - picw - m_scrollbar->get_w()) / 2;
						} else {
							draw_x = point.x + (curw - picw) / 2;
						}
					}
					dst.blit(Point(draw_x, point.y + (lineheight - pich) / 2), entry_picture);
				}
				point.x += picw;
			}

			if (entry_string.empty()) {
				curx += curw;
				continue;
			}
			const Image* entry_text_im = UI::g_fh1->render(as_uifont(entry_string, m_fontsize));

			if (alignment & Align_Right) {
				point.x += curw - picw;
			} else if (alignment & Align_HCenter) {
				point.x += (curw - picw) / 2;
			}

			// Add an offset for rightmost column when the scrollbar is shown.
			uint16_t text_width = entry_text_im->width();
			if (i == nr_columns - 1 && m_scrollbar->is_enabled()) {
				text_width = text_width + m_scrollbar->get_w();
			}
			UI::correct_for_align(alignment, text_width, entry_text_im->height(), &point);
			// Crop to column width
			dst.blitrect(point, entry_text_im, Rect(0, 0, curw - picw, lineheight));
			curx += curw;
		}

		y += lineheight;
		++idx;
	}
}

/**
 * handle key presses
 */
bool Table<void *>::handle_key(bool down, SDL_Keysym code)
{
	if (down) {
		switch (code.sym) {
		case SDLK_UP:
		case SDLK_KP_8:
			move_selection(-1);
			return true;

		case SDLK_DOWN:
		case SDLK_KP_2:
			move_selection(1);
			return true;

		default:
			break; // not handled
		}
	}

	return UI::Panel::handle_key(down, code);
}


bool Table<void *>::handle_mousewheel(uint32_t which, int32_t x, int32_t y) {
	return m_scrollbar->handle_mousewheel(which, x, y);
}

/**
 * Handle mouse presses: select the appropriate entry
 */
bool Table<void *>::handle_mousepress
	(uint8_t const btn, int32_t x, int32_t const y)
{
	if (get_can_focus())
		focus();

	switch (btn) {
	case SDL_BUTTON_LEFT: {
		int32_t const time = WLApplication::get()->get_time();

		//  This hick hack is needed if any of the callback functions calls clear
		//  to forget the last clicked time.
		int32_t const real_last_click_time = m_last_click_time;

		m_last_selection  = m_selection;
		m_last_click_time = time;

		uint32_t const row =
			(y + m_scrollpos - m_headerheight) / get_lineheight();
		if (row < m_entry_records.size()) {
			select(row);
			Columns::size_type const nr_cols = m_columns.size();
			for (uint8_t col = 0; col < nr_cols; ++col) {
				const Column & column = m_columns.at(col);
				x -= column.width;
				if (x <= 0) {
					if (column.is_checkbox_column) {
						play_click();
						m_entry_records.at(row)->toggle(col);
						update(0, 0, get_eff_w(), get_h());
					}
					break;
				}
			}
		}

		if //  check if doubleclicked
			(time - real_last_click_time < DOUBLE_CLICK_INTERVAL
			 &&
			 m_last_selection == m_selection
			 && m_selection != no_selection_index())
			double_clicked(m_selection);

		return true;
	}
	default:
		return false;
	}
}
bool Table<void *>::handle_mouserelease(const uint8_t btn, int32_t, int32_t)
{
	return btn == SDL_BUTTON_LEFT;
}

/**
 * move the currently selected entry up or down.
 * \param offset positive value move the selection down and
 *        negative values up.
 */
void Table<void *>::move_selection(const int32_t offset)
{
	if (!has_selection()) return;
	int32_t new_selection = m_selection + offset;

	if (new_selection < 0) new_selection = 0;
	else if (static_cast<uint32_t>(new_selection) > m_entry_records.size() - 1)
		new_selection = m_entry_records.size() - 1;

	select(static_cast<uint32_t>(new_selection));

	//scroll to newly selected entry
	if (m_scrollbar)
	{
		// Keep an unselected item above or below
		int32_t scroll_item = new_selection + offset;
		if (scroll_item < 0) scroll_item = 0;
		if (scroll_item > static_cast<int32_t>(m_entry_records.size())) {
			scroll_item = m_entry_records.size();
		}

		// Ensure scroll_item is visible
		if (static_cast<int32_t>(scroll_item * get_lineheight()) < m_scrollpos) {
			m_scrollbar->set_scrollpos(scroll_item * get_lineheight());
		} else if
			(static_cast<int32_t>((scroll_item + 1) * get_lineheight() - get_inner_h())
			 > m_scrollpos)
		{
			m_scrollbar->set_scrollpos((scroll_item + 1) * get_lineheight() - get_inner_h());
		}
	}
}

/**
 * Change the currently selected entry
 *
 * Args: i  the entry to select
 */
void Table<void *>::select(const uint32_t i)
{
	if (empty() || m_selection == i)
		return;

	m_selection = i;

	selected(m_selection);
	update(0, 0, get_eff_w(), get_h());
}

/**
 * Add a new entry to the table.
*/
Table<void *>::EntryRecord & Table<void *>::add
	(void * const entry, const bool do_select)
{
	EntryRecord & result = *new EntryRecord(entry);
	m_entry_records.push_back(&result);
	result.m_data.resize(m_columns.size());
	for (size_t i = 0; i < m_columns.size(); ++i)
		if (m_columns.at(i).is_checkbox_column) {
			result.m_data.at(i).d_picture =
				g_gr->images().get("pics/checkbox_empty.png");
		}

	m_scrollbar->set_steps
		(m_entry_records.size() * get_lineheight()
		 -
		 (get_h() - m_headerheight - 2));

	if (do_select) {
		select(m_entry_records.size() - 1);
		m_scrollbar->set_scrollpos(std::numeric_limits<int32_t>::max());
	}

	update(0, 0, get_eff_w(), get_h());
	return result;
}

/**
 * Scroll to the given position, in pixels.
*/
void Table<void *>::set_scrollpos(int32_t const i)
{
	m_scrollpos = i;

	update(0, 0, get_eff_w(), get_h());
}

/**
 * Remove the table entry at the given (zero-based) index.
 */
void Table<void *>::remove(const uint32_t i) {
	assert(i < m_entry_records.size());

	const EntryRecordVector::iterator it = m_entry_records.begin() + i;
	delete *it;
	m_entry_records.erase(it);
	if (m_selection == i)
		m_selection = no_selection_index();
	else if (m_selection > i && m_selection != no_selection_index())
		m_selection--;

	m_scrollbar->set_steps
		(m_entry_records.size() * get_lineheight()
		 -
		 (get_h() - m_headerheight - 2));
}

bool Table<void *>::sort_helper(uint32_t a, uint32_t b)
{
	if (m_sort_descending)
		return m_columns[m_sort_column].compare(b, a);
	else
		return m_columns[m_sort_column].compare(a, b);
}

/**
 * Sort the table alphabetically. Make sure that the current selection stays
 * valid (though it might scroll out of visibility).
 * Only the subarea [start,end) is sorted.
 * For example you might want to sort directories for themselves at the
 * top of list and files at the bottom.
 */
void Table<void *>::sort(const uint32_t Begin, uint32_t End)
{
	assert(m_columns.at(m_sort_column).btn);
	assert(m_sort_column < m_columns.size());

	if (End > size())
		End = size();

	std::vector<uint32_t> indices;
	std::vector<EntryRecord *> copy;

	indices.reserve(End - Begin);
	copy.reserve(End - Begin);
	for (uint32_t i = Begin; i < End; ++i) {
		indices.push_back(i);
		copy.push_back(m_entry_records[i]);
	}

	std::stable_sort
		(indices.begin(), indices.end(),
		 boost::bind(&Table<void *>::sort_helper, this, _1, _2));

	uint32_t newselection = m_selection;
	for (uint32_t i = Begin; i < End; ++i) {
		uint32_t from = indices[i - Begin];
		m_entry_records[i] = copy[from - Begin];
		if (m_selection == from)
			newselection = i;
	}
	m_selection = newselection;

	update();
}

/**
 * Default comparison for checkbox columns:
 * checked items come before unchecked ones.
 */
bool Table<void *>::default_compare_checkbox
	(uint32_t column, uint32_t a, uint32_t b)
{
	EntryRecord & ea = get_record(a);
	EntryRecord & eb = get_record(b);
	return ea.is_checked(column) && !eb.is_checked(column);
}

bool Table<void *>::default_compare_string
	(uint32_t column, uint32_t a, uint32_t b)
{
	EntryRecord & ea = get_record(a);
	EntryRecord & eb = get_record(b);
	return ea.get_string(column) < eb.get_string(column);
}

Table<void *>::EntryRecord::EntryRecord(void * const e)
	: m_entry(e), use_clr(false)
{}

void Table<void *>::EntryRecord::set_picture
	(uint8_t const col, const Image* pic, const std::string & str)
{
	assert(col < m_data.size());

	m_data.at(col).d_picture = pic;
	m_data.at(col).d_string  = str;
}
void Table<void *>::EntryRecord::set_string
	(uint8_t const col, const std::string & str)
{
	assert(col < m_data.size());

	m_data.at(col).d_picture = nullptr;
	m_data.at(col).d_string  = str;
}
const Image* Table<void *>::EntryRecord::get_picture(uint8_t const col) const
{
	assert(col < m_data.size());

	return m_data.at(col).d_picture;
}
const std::string & Table<void *>::EntryRecord::get_string
	(uint8_t const col) const
{
	assert(col < m_data.size());

	return m_data.at(col).d_string;
}

}
