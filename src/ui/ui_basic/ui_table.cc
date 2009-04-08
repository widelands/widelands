/*
 * Copyright (C) 2002, 2006-2009 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "ui_table.h"

#include "graphic.h"
#include "rendertarget.h"
#include "ui_button.h"
#include "ui_constants.h"
#include "ui_scrollbar.h"
#include "wlapplication.h"

namespace UI {
/**
 * Initialize a panel
 *
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
	m_max_pic_width   (0),
	m_fontname        (UI_FONT_NAME),
	m_fontsize        (UI_FONT_SIZE_SMALL),
	m_headerheight    (15),
	m_lineheight      (g_fh->get_fontheight(m_fontname, m_fontsize)),
	m_scrollbar       (0),
	m_scrollpos       (0),
	m_selection       (no_selection_index()),
	m_last_click_time (-10000),
	m_last_selection  (no_selection_index()),
	m_sort_column     (0),
	m_sort_descending (descending)
{
	set_think(false);
}


/**
 * Free allocated resources
*/
Table<void *>::~Table()
{
	container_iterate_const(Entry_Record_vector, m_entry_records, i)
		delete *i.current;
}

/// Add a new column to this table.
void Table<void *>::add_column
	(uint32_t const width, std::string const & title, Align const alignment)
{

	//  If there would be existing entries, they would not get the new column.
	assert(size() == 0);

	uint32_t complete_width = 0;
	container_iterate_const(Columns, m_columns, i)
		complete_width += i.current->width;

	{
		Column const c = {
			title.size() ?
				new Callback_IDButton<Table, Columns::size_type>
					(this,
					 complete_width, 0, width, m_headerheight,
					 3,
					 &Table::header_button_clicked, *this, m_columns.size(),
					 title, "", true, false, m_fontname, m_fontsize)
				:
				0,
				width,
				alignment
		};
		m_columns.push_back(c);
	}
	if (not m_scrollbar) {
		m_scrollbar =
			new Scrollbar
				(get_parent(),
				 get_x() + get_w() - 24, get_y() + m_headerheight,
				 24,                     get_h() - m_headerheight,
				 false);
		m_scrollbar->moved.set(this, &Table::set_scrollpos);
		m_scrollbar->set_steps(1);
		uint32_t lineheight = g_fh->get_fontheight(m_fontname, m_fontsize);
		m_scrollbar->set_singlestepsize(lineheight);
		m_scrollbar->set_pagesize(get_h() - lineheight);
	}
}

Table<void *>::Entry_Record * Table<void *>::find
	(const void * const entry) const
	throw ()
{
	container_iterate_const(Entry_Record_vector, m_entry_records, i)
		if ((*i.current)->entry() == entry)
			return *i.current;

	return 0;
}

/**
 * A header button has been clicked
 */
void Table<void *>::header_button_clicked(Columns::size_type const n) {
	assert(m_columns.at(n).btn);
	if (get_sort_colum() == n) {
		set_sort_descending(not get_sort_descending()); //  change sort direction
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
	container_iterate_const(Entry_Record_vector, m_entry_records, i)
		delete *i.current;
	m_entry_records.clear();

	if (m_scrollbar)
		m_scrollbar->set_steps(1);
	m_scrollpos = 0;
	m_selection = no_selection_index();
	m_last_click_time = -10000;
	m_last_selection = no_selection_index();
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
		if (y >= get_h())
			return;

		const Entry_Record & er = *m_entry_records[idx];

		if (idx == m_selection) {
			assert(2 <= get_eff_w());
			dst.brighten_rect
				(Rect(Point(1, y), get_eff_w() - 2, m_lineheight),
				 -ms_darken_value);
		}

		const RGBColor col = er.use_clr ? er.clr : UI_FONT_CLR_FG;

		Columns::size_type const nr_columns = m_columns.size();
		for (uint32_t i = 0, curx = 0; i < nr_columns; ++i) {
			uint32_t const curw      = m_columns[i].width;
			Align    const alignment = m_columns[i].alignment;

			int32_t             const entry_picture = er.get_picture(i);
			std::string const &       entry_string  = er.get_string (i);
			uint32_t w = 0, h = g_fh->get_fontheight(m_fontname, m_fontsize);
			if (entry_picture != -1)
				g_gr->get_picture_size(entry_picture, w, h);
			Point point =
				Point(curx, y)
				+
				Point
					(alignment & Align_Right   ?  curw - w  - 1 :
					 alignment & Align_HCenter ? (curw - w) / 2 :
					 1,
					 (static_cast<int32_t>(get_lineheight())
					  -
					  static_cast<int32_t>(h))
					 /
					 2);
			if (entry_picture != -1)
				dst.blit(point, entry_picture);
			else
				g_fh->draw_string
					(dst,
					 m_fontname, m_fontsize,
					 col,
					 RGBColor(107, 87, 55),
					 point,
					 entry_string, alignment,
					 -1);

			curx += curw;
		}

		y += lineheight;
		++idx;
	}
}

/**
 * Handle mouse presses: select the appropriate entry
 */
bool Table<void *>::handle_mousepress(const Uint8 btn, int32_t, int32_t y) {
	switch (btn) {
	case SDL_BUTTON_WHEELDOWN:
	case SDL_BUTTON_WHEELUP:
		return m_scrollbar ? m_scrollbar->handle_mousepress(btn, 0, y) : false;
	case SDL_BUTTON_LEFT: {
		int32_t const time = WLApplication::get()->get_time();

		//  This hick hack is needed if any of the callback functions calls clear
		//  to forget the last clicked time.
		int32_t const real_last_click_time = m_last_click_time;

		m_last_selection  = m_selection;
		m_last_click_time = time;

		y = (y + m_scrollpos - m_headerheight) / get_lineheight();
		if (static_cast<size_t>(y) < m_entry_records.size()) select(y);

		if //  check if doubleclicked
			(time - real_last_click_time < DOUBLE_CLICK_INTERVAL
			 and
			 m_last_selection == m_selection
			 and m_selection != no_selection_index())
			double_clicked.call(m_selection);

		return true;
	}
	default:
		return false;
	}
}
bool Table<void *>::handle_mouserelease(const Uint8 btn, int32_t, int32_t)
{return btn == SDL_BUTTON_LEFT;}

/**
 * Change the currently selected entry
 *
 * Args: i  the entry to select
 */
void Table<void *>::select(const uint32_t i)
{
	if (m_selection == i)
		return;

	m_selection = i;

	selected.call(m_selection);
	update(0, 0, get_eff_w(), get_h());
}

/**
 * Add a new entry to the table.
*/
Table<void *>::Entry_Record & Table<void *>::add
	(void * const entry, const bool do_select)
{
	int32_t entry_height = g_fh->get_fontheight(m_fontname, m_fontsize);
	if (entry_height > m_lineheight)
		m_lineheight = entry_height;

	Entry_Record & result = *new Entry_Record(entry);
	m_entry_records.push_back(&result);
	result.m_data.resize(m_columns.size());

	m_scrollbar->set_steps
		(m_entry_records.size() * get_lineheight()
		 -
		 (get_h() - m_headerheight - 2));

	if (do_select)
		select(m_entry_records.size() - 1);

	update(0, 0, get_eff_w(), get_h());
	return result;
}

/**
 * Scroll to the given position, in pixels.
*/
void Table<void *>::set_scrollpos(int32_t i)
{
	m_scrollpos = i;

	update(0, 0, get_eff_w(), get_h());
}


void Table<void *>::remove(const uint32_t i) {
	assert(i < m_entry_records.size());

	const Entry_Record_vector::iterator it = m_entry_records.begin() + i;
	delete *it;
	m_entry_records.erase(it);
	if (m_selection == i) m_selection = no_selection_index();
}

/**
 * Sort the table alphabetically. make sure that the current selection stays
 * valid (though it might scroll out of visibility).
 * start and end defines the beginning and the end of a subarea to
 * sort, for example you might want to sort directories for themselves at the
 * top of list and files at the bottom.
 */
void Table<void *>::sort(const uint32_t Begin, uint32_t End) {
	assert(m_columns.at(m_sort_column).btn);
	assert(m_sort_column < m_columns.size());
	if (End > size()) End = size();
	if (get_sort_descending())
		for (uint32_t i = Begin; i != End; ++i) for (uint32_t j = i; j != End; ++j) {
			Entry_Record * const eri = m_entry_records[i];
			Entry_Record * const erj = m_entry_records[j];
			if (eri->get_string(m_sort_column) > erj->get_string(m_sort_column)) {
				if      (m_selection == i) m_selection = j;
				else if (m_selection == j) m_selection = i;
				m_entry_records[i] = erj;
				m_entry_records[j] = eri;
			}
		}
	else
		for (uint32_t i = Begin; i != End; ++i) for (uint32_t j = i; j != End; ++j) {
			Entry_Record * const eri = m_entry_records[i];
			Entry_Record * const erj = m_entry_records[j];
			if (eri->get_string(m_sort_column) < erj->get_string(m_sort_column)) {
				if      (m_selection == i) m_selection = j;
				else if (m_selection == j) m_selection = i;
				m_entry_records[i] = erj;
				m_entry_records[j] = eri;
			}
		}
}


Table<void *>::Entry_Record::Entry_Record(void * const e)
	: m_entry(e), use_clr(false)
{}

void Table<void *>::Entry_Record::set_picture
	(uint32_t const column, uint32_t const picid, std::string const & str)
{
	assert(column < m_data.size());

	m_data[column].d_picture = picid;
	m_data[column].d_string = str;
}
void Table<void *>::Entry_Record::set_string
	(const uint32_t column, const std::string & str)
{
	assert(column < m_data.size());

	m_data[column].d_picture = -1;
	m_data[column].d_string = str;
}
int32_t Table<void *>::Entry_Record::get_picture
	(uint32_t const column) const
{
	assert(column < m_data.size());

	return m_data[column].d_picture;
}
const std::string & Table<void *>::Entry_Record::get_string
	(const uint32_t column) const
{
	assert(column < m_data.size());

	return m_data[column].d_string;
}
};
