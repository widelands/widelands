/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#include "constants.h"
#include "graphic.h"
#include "rendertarget.h"
#include "ui_button.h"
#include "ui_scrollbar.h"
#include "wlapplication.h"

namespace UI {
/**
Initialize a panel

Args: parent  parent panel
      x       coordinates of the Table
      y
      w       dimensions, in pixels, of the Table
      h
      align   alignment of text inside the Table
*/
Table<void *>::Table
	(Panel * const parent,
	 int32_t x, int32_t y, uint32_t w, uint32_t h,
	 const Align align,
	 const bool descending)
:
	Panel             (parent, x, y, w, h),
	m_max_pic_width   (0),
	m_lineheight      (g_fh->get_fontheight(UI_FONT_SMALL)),
	m_scrollbar       (0),
	m_scrollpos       (0),
	m_selection       (no_selection_index()),
	m_last_click_time (-10000),
	m_last_selection  (no_selection_index()),
	m_sort_column     (0),
	m_sort_descending  (descending)

{
	set_think(false);

	set_align(align);
}


/**
Free allocated resources
*/
Table<void *>::~Table()
{
	const Entry_Record_vector::const_iterator entry_records_end =
		m_entry_records.end();
	for
		(Entry_Record_vector::const_iterator it = m_entry_records.begin();
		 it != entry_records_end;
		 ++it)
		delete *it;
}

/*
 * Add a new colum to this table
 */
void Table<void *>::add_column(const std::string & name, const uint32_t width) {

	//  If there would be existing entries, they would not get the new column.
	assert(size() == 0);

	uint32_t complete_width = 0;
	const Columns::const_iterator columns_end = m_columns.end();
	for (Columns::const_iterator it = m_columns.begin(); it != columns_end; ++it)
		complete_width += it->btn->get_w();

	Column c = {
		name,
		new IDButton<Table, Columns::size_type>
		   (this,
		    complete_width, 0, width, 15,
		    3,
		    &Table::header_button_clicked, this, m_columns.size(),
		    name)
	};
   m_columns.push_back(c);
	if (not m_scrollbar) {
      m_scrollbar=new Scrollbar(get_parent(), get_x()+get_w()-24, get_y()+m_columns[0].btn->get_h(), 24, get_h()-m_columns[0].btn->get_h(), false);
      m_scrollbar->moved.set(this, &Table::set_scrollpos);
      m_scrollbar->set_steps(1);
	}

}

Table<void *>::Entry_Record * Table<void *>::find
	(const void * const entry) const
	throw ()
{
	const Entry_Record_vector::const_iterator entries_end = m_entry_records.end();
	for
		(Entry_Record_vector::const_iterator it = m_entry_records.begin();
		 it != entries_end;
		 ++it)
		if ((*it)->entry() == entry) return *it;

    return 0;
}

/*
 * A header button has been clicked
 */
void Table<void *>::header_button_clicked(Columns::size_type n) {
	if (get_sort_colum() == n) {
      // Change sort direction
		set_sort_descending(not get_sort_descending());
      sort();
      return;
	}

   set_sort_column(n);
   sort();
   return;
}

/**
Remove all entries from the table
*/
void Table<void *>::clear()
{
	const Entry_Record_vector::const_iterator entry_records_end =
		m_entry_records.end();
	for
		(Entry_Record_vector::const_iterator it = m_entry_records.begin();
		 it != entry_records_end;
		 ++it)
		delete *it;
	m_entry_records.clear();

	if (m_scrollbar)
		m_scrollbar->set_steps(1);
	m_scrollpos = 0;
	m_selection = no_selection_index();
	m_last_click_time = -10000;
	m_last_selection = no_selection_index();
}

/**
Redraw the table
*/
void Table<void *>::draw(RenderTarget * dst)
{
   // draw text lines
   int32_t lineheight = get_lineheight();
	uint32_t idx = m_scrollpos / lineheight;
   int32_t y = 1 + idx*lineheight - m_scrollpos + m_columns[0].btn->get_h();

	dst->brighten_rect(Rect(Point(0, 0), get_w(), get_h()), ms_darken_value);

	while (idx < m_entry_records.size()) {
		if (y >= get_h())
         return;

		const Entry_Record & er = *m_entry_records[idx];

		if (idx == m_selection) {
			assert(2 <= get_eff_w());
         // dst->fill_rect(1, y, get_eff_w()-2, g_font->get_fontheight(), m_selcolor);
			dst->brighten_rect
				(Rect(Point(1, y), get_eff_w() - 2, m_lineheight),
				 -ms_darken_value);
		}

      // First draw pictures
		if (er.get_picid() != -1) {
			uint32_t w, h;
			g_gr->get_picture_size(er.get_picid(), w, h);
			dst->blit(Point(1, y + (get_lineheight() - h) / 2), er.get_picid());
		}

		const RGBColor col = er.use_clr ? er.clr : UI_FONT_CLR_FG;

      int32_t curx=0;
      int32_t curw;
		for (uint32_t i = 0; i < get_nr_columns(); ++i) {
         curw=m_columns[i].btn->get_w();

         // Horizontal center the string
			g_fh->draw_string
				(*dst,
				 UI_FONT_SMALL,
				 col,
				 RGBColor(107, 87, 55),
				 Point(curx, y)
				 +
				 Point
				 (m_align & Align_Right     ? curw            -  1 :
				  m_align & Align_HCenter   ? curw            >> 1 :
				  // Pictures are always left aligned, leave some space here
				  m_max_pic_width && i == 0 ? m_max_pic_width + 10 :
				  1,
				  (get_lineheight() - g_fh->get_fontheight(UI_FONT_SMALL)) / 2),
				 er.get_string(i), m_align,
				 -1);

         curx+=curw;
		}

      y += lineheight;
      idx++;
	}
}

/**
 * Handle mouse presses: select the appropriate entry
 */
bool Table<void *>::handle_mousepress(const Uint8 btn, int32_t, int32_t y) {
	if (btn != SDL_BUTTON_LEFT) return false;

	   int32_t time=WLApplication::get()->get_time();

      // This hick hack is needed if any of the
      // callback functions calls clear to forget the last
      // clicked time.
      int32_t real_last_click_time=m_last_click_time;

      m_last_selection=m_selection;
      m_last_click_time=time;

      y = (y + m_scrollpos - m_columns[0].btn->get_h()) / get_lineheight();
	if (static_cast<size_t>(y) < m_entry_records.size()) select(y);

      // check if doubleclicked
	if
		(time - real_last_click_time < DOUBLE_CLICK_INTERVAL
		 and
		 m_last_selection == m_selection
		 and m_selection != no_selection_index())
         double_clicked.call(m_selection);


	return true;
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
Add a new entry to the table.
*/
Table<void *>::Entry_Record & Table<void *>::add
	(void * const entry, const int32_t picid, const bool do_select)
{
   int32_t entry_height=g_fh->get_fontheight(UI_FONT_SMALL);
	if (picid != -1) {
		uint32_t w, h;
		g_gr->get_picture_size(picid, w, h);
	   entry_height = std::max<uint32_t>(entry_height, h);
		if (m_max_pic_width < w)
			m_max_pic_width = w;
	}
	if (entry_height > m_lineheight)
		m_lineheight = entry_height;

	Entry_Record & result = *new Entry_Record(entry, picid);
	m_entry_records.push_back(&result);
	result.m_data.resize(get_nr_columns());

	m_scrollbar->set_steps(m_entry_records.size() * get_lineheight() - (get_h() - m_columns[0].btn->get_h() - 2));

	if (do_select)
      select(m_entry_records.size() - 1);

   update(0, 0, get_eff_w(), get_h());
	return result;
}

/**
Scroll to the given position, in pixels.
*/
void Table<void *>::set_scrollpos(int32_t i)
{
	m_scrollpos = i;

	update(0, 0, get_eff_w(), get_h());
}

/**
Set the list alignment (only horizontal alignment works)
*/
void Table<void *>::set_align(Align align)
{
	m_align = static_cast<Align>(align & Align_Horizontal);
}


void Table<void *>::remove(const uint32_t i) {
	assert(i < m_entry_records.size());

	const Entry_Record_vector::iterator it = m_entry_records.begin() + i;
	delete *it;
	m_entry_records.erase(it);
	if (m_selection == i) m_selection = no_selection_index();
}

/*
 * Sort the table alphabetically. make sure that the current selection stays
 * valid (though it might scroll out of visibility).
 * start and end defines the beginning and the end of a subarea to
 * sort, for example you might want to sort directorys for themselves at the
 * top of list and files at the bottom.
 */
void Table<void *>::sort(const uint32_t Begin, uint32_t End) {
	assert(m_sort_column < m_columns.size());
	if (End > size()) End = size();
	if (get_sort_descending())
		for (uint32_t i = Begin; i != End; ++i) for (uint32_t j = i; j != End; ++j) {
			Entry_Record * const eri = m_entry_records[i];
			Entry_Record * const erj = m_entry_records[j];
			if (eri->get_string(m_sort_column) > erj->get_string(m_sort_column)) {
				if      (m_selection == i) m_selection = j;
				else if (m_selection == j) m_selection = i;
				m_entry_records[i]=erj;
				m_entry_records[j]=eri;
			}
		}
	else
		for (uint32_t i = Begin; i != End; ++i) for (uint32_t j = i; j != End; ++j) {
			Entry_Record * const eri = m_entry_records[i];
			Entry_Record * const erj = m_entry_records[j];
			if (eri->get_string(m_sort_column) < erj->get_string(m_sort_column)) {
				if      (m_selection == i) m_selection = j;
				else if (m_selection == j) m_selection = i;
				m_entry_records[i]=erj;
				m_entry_records[j]=eri;
			}
		}
}


Table<void *>::Entry_Record::Entry_Record(void * const e, const int32_t picid) :
	m_entry(e), use_clr(false), m_picid(picid)
{}

void Table<void *>::Entry_Record::set_string
	(const uint32_t column, const std::string & str)
{
	assert(column < m_data.size());

   m_data[column].d_string = str;
}
const std::string & Table<void *>::Entry_Record::get_string
	(const uint32_t column) const
{
	assert(column < m_data.size());

	return m_data[column].d_string;
}
};
