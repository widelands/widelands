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

#include "ui_listselect.h"

#include "constants.h"
#include "graphic.h"
#include "rendertarget.h"
#include "wlapplication.h"

namespace UI {
/**
 * Initialize a list select panel
 *
 * Args: parent  parent panel
 *       x       coordinates of the Listselect
 *       y
 *       w       dimensions, in pixels, of the Listselect
 *       h
 *       align   alignment of text inside the Listselect
*/
BaseListselect::BaseListselect
	(Panel * const parent,
	 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h,
	 Align const align, bool const show_check)
:
Panel(parent, x, y, w, h),
m_lineheight(g_fh->get_fontheight(UI_FONT_SMALL)),
m_scrollbar      (this, get_w() - 24, 0, 24, h, false),
m_scrollpos     (0),
m_selection     (no_selection_index()),
m_last_click_time(-10000),
m_last_selection(no_selection_index()),
m_show_check(show_check)
{
	set_think(false);

	set_align(align);

	m_scrollbar.moved.set(this, &BaseListselect::set_scrollpos);
	m_scrollbar.set_singlestepsize(g_fh->get_fontheight(UI_FONT_SMALL));
	m_scrollbar.set_pagesize(h - 2 * g_fh->get_fontheight(UI_FONT_SMALL));
	m_scrollbar.set_steps(1);

	if (show_check) {
		uint32_t pic_h;
		m_check_picid = g_gr->get_picture(PicMod_Game,  "pics/list_selected.png");
		g_gr->get_picture_size(m_check_picid, m_max_pic_width, pic_h);
		if (pic_h > m_lineheight) m_lineheight = pic_h;
	}
	else {
		m_max_pic_width=0;
	}

}


/**
 * Free allocated resources
*/
BaseListselect::~BaseListselect()
{
	clear();
}


/**
 * Remove all entries from the listselect
*/
void BaseListselect::clear() {
	const Entry_Record_vector::const_iterator entry_records_end =
		m_entry_records.end();
	for
		(Entry_Record_vector::const_iterator it = m_entry_records.begin();
		 it != entry_records_end;
		 ++it)
		free(*it);
	m_entry_records.clear();

	m_scrollbar.set_steps(1);
	m_scrollpos = 0;
	m_selection = no_selection_index();
	m_last_click_time = -10000;
	m_last_selection = no_selection_index();
}


/**
 * Add a new entry to the listselect.
 *
 * Args: name   name that will be displayed
 * entry  value returned by get_select()
 *       sel    if true, directly select the new entry
*/
void BaseListselect::add
	(char const * const name,
	 uint32_t           entry,
	 int32_t      const picid,
	 bool         const sel)
{
	Entry_Record & er = *static_cast<Entry_Record *>
		(malloc(sizeof(Entry_Record) + strlen(name)));

	er.m_entry = entry;
	er.picid = picid;
	er.use_clr = false;
	strcpy(er.name, name);

	uint32_t entry_height = 0;
	if (picid==-1) {
		entry_height=g_fh->get_fontheight(UI_FONT_SMALL);
	} else {
		uint32_t w, h;
		g_gr->get_picture_size(picid, w, h);
		entry_height = (h >= g_fh->get_fontheight(UI_FONT_SMALL))
			? h : g_fh->get_fontheight(UI_FONT_SMALL);
		if (m_max_pic_width < w)
			m_max_pic_width = w;
	}

	if (entry_height>m_lineheight) m_lineheight=entry_height;

	m_entry_records.push_back(&er);

	m_scrollbar.set_steps(m_entry_records.size() * get_lineheight() - get_h());

	update(0, 0, get_eff_w(), get_h());

	if (sel)
		select(m_entry_records.size() - 1);
}

/**
 * Switch two entries
 */
void BaseListselect::switch_entries(const uint32_t m, const uint32_t n)
{
	assert(m < size());
	assert(n < size());

	std::swap(m_entry_records[m], m_entry_records[n]);

	if (m_selection == m) {
		m_selection = n;
		selected.call(n);
	} else if (m_selection == n) {
		m_selection = m;
		selected.call(m);
	}
}

/**
 * Sort the listbox alphabetically. make sure that the current selection stays
 * valid (though it might scroll out of visibility).
 * start and end defines the beginning and the end of a subarea to
 * sort, for example you might want to sort directories for themselves at the
 * top of list and files at the bottom.
 */
void BaseListselect::sort(const uint32_t Begin, uint32_t End)
{
	if (End > size()) End = size();
	for (uint32_t i = Begin; i < End; ++i)
		for (uint32_t j = i + 1; j < End; ++j) {
			Entry_Record * const eri = m_entry_records[i];
			Entry_Record * const erj = m_entry_records[j];
			if (strcmp(eri->name, erj->name) > 0)  {
				if      (m_selection == i) m_selection = j;
				else if (m_selection == j) m_selection = i;
				m_entry_records[i]=erj;
				m_entry_records[j]=eri;
			}
		}
}

/**
 * Set the list alignment (only horizontal alignment works)
*/
void BaseListselect::set_align(const Align align)
{
	m_align = static_cast<Align>(align & Align_Horizontal);
}


/**
 * Scroll to the given position, in pixels.
*/
void BaseListselect::set_scrollpos(const int32_t i)
{
	m_scrollpos = i;

	update(0, 0, get_eff_w(), get_h());
}


/**
 * Define a special color that will be used to display the item at the given
 * index.
 */
void BaseListselect::set_entry_color(const uint32_t n, const RGBColor col) throw ()
{
	assert(n < m_entry_records.size());

	m_entry_records[n]->use_clr = true;
	m_entry_records[n]->clr = col;
}


/**
 * Change the currently selected entry
 *
 * Args: i  the entry to select
 */
void BaseListselect::select(const uint32_t i)
{
	if (m_selection == i)
		return;

	if (m_show_check) {
		if (m_selection != no_selection_index())
			m_entry_records[m_selection]->picid = -1;
		m_entry_records[i]->picid = m_check_picid;
	}
	m_selection = i;

	selected.call(m_selection);
	update(0, 0, get_eff_w(), get_h());
}

/**
 * \return \c true if an item is select, or \c false if there is no current
 * selection
 */
bool BaseListselect::has_selection() const throw ()
{
	return m_selection != no_selection_index();
}


/**
 * \return the ID/entry value of the currently selected item.
 * The entry value is given as a parameter to \ref add
 *
 * Throws an exception when no item is selected.
 */
uint32_t BaseListselect::get_selected() const throw (No_Selection)
{
	if (m_selection == no_selection_index())
		throw No_Selection();

	return m_entry_records[m_selection]->m_entry;
}


/**
 * Remove the currently selected item. Throws an exception when no
 * item is selected.
 */
void BaseListselect::remove_selected() throw (No_Selection)
{
	if (m_selection == no_selection_index())
		throw No_Selection();

	remove(m_selection);
}


uint32_t BaseListselect::get_lineheight() const throw ()
{
	return m_lineheight + 2;
}

uint32_t BaseListselect::get_eff_w() const throw ()
{
	return get_w();
}


/**
Redraw the listselect box
*/
void BaseListselect::draw(RenderTarget* dst)
{
	// draw text lines
	const uint32_t lineheight = get_lineheight();
	uint32_t idx = m_scrollpos / lineheight;
	int32_t y = 1 + idx*lineheight - m_scrollpos;

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

		int32_t x;
		if (m_align & Align_Right)
			x = get_eff_w() - 1;
		else if (m_align & Align_HCenter)
			x = get_eff_w()>>1;
		else {
			// Pictures are always left aligned, leave some space here
			if (m_max_pic_width)
				x= m_max_pic_width + 10;
			else
			x=1;
		}

		const RGBColor col = er.use_clr ? er.clr : UI_FONT_CLR_FG;

		// Horizontal center the string
		g_fh->draw_string
			(*dst,
			 UI_FONT_SMALL,
			 col,
			 RGBColor(107, 87, 55),
			 Point
			 	(x,
			 	 y +
			 	 (get_lineheight() - g_fh->get_fontheight(UI_FONT_SMALL)) / 2),
			 er.name, m_align,
			 -1);

		// Now draw pictures
		if (er.picid != -1) {
			uint32_t w, h;
			g_gr->get_picture_size(er.picid, w, h);
			dst->blit(Point(1, y + (get_lineheight() - h) / 2), er.picid);
		}
		y += lineheight;
		++idx;
	}
}


/**
 * Handle mouse presses: select the appropriate entry
 */
bool BaseListselect::handle_mousepress(const Uint8 btn, int32_t, int32_t y)
{
	switch (btn) {
	case SDL_BUTTON_WHEELDOWN:
	case SDL_BUTTON_WHEELUP:
		return m_scrollbar.handle_mousepress(btn, 0, y);
	case SDL_BUTTON_LEFT: {
		int32_t const time = WLApplication::get()->get_time();

		//  This hick hack is needed if any of the callback functions calls clear
		//  to forget the last clicked time.
		int32_t const real_last_click_time = m_last_click_time;

		m_last_selection  = m_selection;
		m_last_click_time = time;
		play_click();

		y = (y + m_scrollpos) / get_lineheight();
		if (y >= 0 and y < static_cast<int32_t>(m_entry_records.size()))
			select(y);


		if //  check if doubleclicked
			(time-real_last_click_time < DOUBLE_CLICK_INTERVAL
			 and
			 m_last_selection == m_selection
			 and
			 m_selection != no_selection_index())
			double_clicked.call(m_selection);

		return true;
	}
	default:
		return false;
	}
}

bool BaseListselect::handle_mouserelease(const Uint8 btn, int32_t, int32_t)
{
	return btn == SDL_BUTTON_LEFT;
}

/**
 * Remove entry
 */
void BaseListselect::remove(const uint32_t i)
{
	assert(i < m_entry_records.size());

	free(m_entry_records[i]);
	m_entry_records.erase(m_entry_records.begin() + i);
	if (m_selection == i)
		selected.call(m_selection = no_selection_index());
	else if (i <  m_selection)
		--m_selection;
}

/**
 * Remove an entry by name. This only removes
 * the first entry with this name. If none is found, nothing
 * is done
 */
void BaseListselect::remove(const char * const str)
{
	for (uint32_t i=0; i<m_entry_records.size(); ++i) {
		if (!strcmp(m_entry_records[i]->name, str)) {
			remove(i);
			return;
		}
	}
}

};
