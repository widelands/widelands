/*
 * Copyright (C) 2002, 2006-2011, 2015 by the Widelands Development Team
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

#include "ui_basic/listselect.h"

#include <iostream>

#include <boost/bind.hpp>

#include "base/log.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "wlapplication.h"

constexpr int kMargin = 2;

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
	m_lineheight(UI::g_fh1->render(as_uifont(UI::g_fh1->fontset()->representative_character()))->height()
					 + kMargin),
	m_scrollbar      (this, get_w() - 24, 0, 24, h, false),
	m_scrollpos     (0),
	m_selection     (no_selection_index()),
	m_last_click_time(-10000),
	m_last_selection(no_selection_index()),
	m_show_check(show_check)
{
	set_thinks(false);

	//  do not allow vertical alignment as it does not make sense
	m_align = static_cast<Align>(align & Align_Horizontal);

	m_scrollbar.moved.connect(boost::bind(&BaseListselect::set_scrollpos, this, _1));
	m_scrollbar.set_singlestepsize(m_lineheight);
	m_scrollbar.set_pagesize(h - 2 * m_lineheight);
	m_scrollbar.set_steps(1);

	if (show_check) {
		uint32_t pic_h;
		m_check_pic = g_gr->images().get("pics/list_selected.png");
		m_max_pic_width = m_check_pic->width();
		pic_h = m_check_pic->height();
		if (pic_h > m_lineheight)
			m_lineheight = pic_h;
	}
	else {
		m_max_pic_width = 0;
	}
	set_can_focus(true);
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
	for (EntryRecord * entry : m_entry_records) {
		delete entry;
	}
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
	(const std::string& name,
	 uint32_t             entry,
	 const Image*   pic,
	 bool         const   sel,
	 const std::string  & tooltip_text)
{
	EntryRecord * er = new EntryRecord();

	er->m_entry = entry;
	er->pic   = pic;
	er->use_clr = false;
	er->name    = name;
	er->tooltip = tooltip_text;
	uint32_t entry_height = m_lineheight;
	if (pic) {
		uint16_t w = pic->width();
		uint16_t h = pic->height();
		entry_height = (h >= entry_height) ? h : entry_height;
		if (m_max_pic_width < w)
			m_max_pic_width = w;
	}

	if (entry_height > m_lineheight)
		m_lineheight = entry_height;

	m_entry_records.push_back(er);

	m_scrollbar.set_steps(m_entry_records.size() * get_lineheight() - get_h());

	update(0, 0, get_w(), get_h());

	if (sel)
		select(m_entry_records.size() - 1);
}

void BaseListselect::add_front
	(const std::string& name,
	 const Image*   pic,
	 bool         const   sel,
	 const std::string  & tooltip_text)
{
	EntryRecord * er = new EntryRecord();

	er->m_entry = 0;
	for (EntryRecord * temp_entry : m_entry_records) {
		++(temp_entry)->m_entry;
	}

	er->pic   = pic;
	er->use_clr = false;
	er->name    = name;
	er->tooltip = tooltip_text;

	uint32_t entry_height = m_lineheight;
	if (pic) {
		uint16_t w = pic->width();
		uint16_t h = pic->height();
		entry_height = (h >= entry_height) ? h : entry_height;
		if (m_max_pic_width < w)
			m_max_pic_width = w;
	}

	if (entry_height > m_lineheight)
		m_lineheight = entry_height;

	m_entry_records.push_front(er);

	m_scrollbar.set_steps(m_entry_records.size() * get_lineheight() - get_h());

	update(0, 0, get_w(), get_h());

	if (sel)
		select(0);
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
		selected(n);
	} else if (m_selection == n) {
		m_selection = m;
		selected(m);
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
	if (End > size())
		End = size();
	for (uint32_t i = Begin; i < End; ++i)
		for (uint32_t j = i + 1; j < End; ++j) {
			EntryRecord * const eri = m_entry_records[i];
			EntryRecord * const erj = m_entry_records[j];
			if (strcmp(eri->name.c_str(), erj->name.c_str()) > 0) {
				if      (m_selection == i)
					m_selection = j;
				else if (m_selection == j)
					m_selection = i;
				m_entry_records[i] = erj;
				m_entry_records[j] = eri;
			}
		}
}

/**
 * Scroll to the given position, in pixels.
*/
void BaseListselect::set_scrollpos(const int32_t i)
{
	if (m_scrollpos == uint32_t(i))
		return;

	m_scrollpos = i;

	update(0, 0, get_w(), get_h());
}


/**
 * Define a special color that will be used to display the item at the given
 * index.
 */
void BaseListselect::set_entry_color
	(const uint32_t n, const RGBColor col)
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
			m_entry_records[m_selection]->pic = nullptr;
		m_entry_records[i]->pic = m_check_pic;
	}
	m_selection = i;

	selected(m_selection);
	update(0, 0, get_w(), get_h());
}

/**
 * \return \c true if an item is select, or \c false if there is no current
 * selection
 */
bool BaseListselect::has_selection() const
{
	return m_selection != no_selection_index();
}


/**
 * \return the ID/entry value of the currently selected item.
 * The entry value is given as a parameter to \ref add
 *
 * Throws an exception when no item is selected.
 */
uint32_t BaseListselect::get_selected() const
{
	if (m_selection == no_selection_index())
		throw NoSelection();

	return m_entry_records[m_selection]->m_entry;
}


/**
 * Remove the currently selected item. Throws an exception when no
 * item is selected.
 */
void BaseListselect::remove_selected()
{
	if (m_selection == no_selection_index())
		throw NoSelection();

	remove(m_selection);
}


uint32_t BaseListselect::get_lineheight() const
{
	return m_lineheight + kMargin;
}

uint32_t BaseListselect::get_eff_w() const
{
	return m_scrollbar.is_enabled() ? get_w() - m_scrollbar.get_w() : get_w();
}

/**
Redraw the listselect box
*/
void BaseListselect::draw(RenderTarget & dst)
{
	// draw text lines
	const uint32_t lineheight = get_lineheight();
	uint32_t idx = m_scrollpos / lineheight;
	int32_t y = 1 + idx * lineheight - m_scrollpos;

	dst.brighten_rect(Rect(Point(0, 0), get_w(), get_h()), ms_darken_value);

	while (idx < m_entry_records.size()) {
		assert
			(get_h()
			 <
			 static_cast<int32_t>(std::numeric_limits<int32_t>::max()));

		if (y >= static_cast<int32_t>(get_h()))
			break;

		const EntryRecord & er = *m_entry_records[idx];

		Point point(1, y);
		uint32_t maxw = get_eff_w() - 2;

		// Highlight the current selected entry
		if (idx == m_selection) {
			Rect r = Rect(point, maxw, m_lineheight);
			if (r.x < 0) {
				r.w += r.x; r.x = 0;
			}
			if (r.y < 0) {
				r.h += r.y; r.y = 0;
			}
			assert(2 <= get_eff_w());
			// Make the area a bit more white and more transparent
			if (r.w > 0 && r.h > 0)
			{
				dst.brighten_rect(r, - ms_darken_value * 2);
			}
		}

		uint32_t picw = m_max_pic_width ? m_max_pic_width + 10 : 0;

		// Now draw pictures
		if (er.pic) {
			dst.blit(Point(UI::g_fh1->fontset()->is_rtl() ? get_eff_w() - er.pic->width() - 1 : 1,
								y + (get_lineheight() - er.pic->height()) / 2),
						er.pic);
		}

		const Image* entry_text_im = UI::g_fh1->render(as_uifont(er.name, UI_FONT_SIZE_SMALL,
																					er.use_clr ? er.clr : UI_FONT_CLR_FG));

		Align alignment = mirror_alignment(m_align);
		if (alignment & Align_Right) {
			point.x += maxw - picw;
		} else if (alignment & Align_HCenter) {
			point.x += (maxw - picw) / 2;
		}


		UI::correct_for_align(alignment, entry_text_im->width(), entry_text_im->height(), &point);

		// Shift for image width
		if (!UI::g_fh1->fontset()->is_rtl()) {
			point.x += picw;
		}

		// Fix vertical position for mixed font heights
		if (get_lineheight() > static_cast<uint32_t>(entry_text_im->height())) {
			point.y += (get_lineheight() - entry_text_im->height()) / 2;
		} else {
			point.y -= (entry_text_im->height() - get_lineheight()) / 2;
		}

		dst.blitrect(point, entry_text_im, Rect(0, 0, get_eff_w(), m_lineheight));

		y += lineheight;
		++idx;
	}
}

/**
 * Handle mouse wheel events
 */
bool BaseListselect::handle_mousewheel(uint32_t which, int32_t x, int32_t y) {
	return m_scrollbar.handle_mousewheel(which, x, y);
}

/**
 * Handle mouse presses: select the appropriate entry
 */
bool BaseListselect::handle_mousepress(const uint8_t btn, int32_t, int32_t y)
{
	switch (btn) {

	case SDL_BUTTON_LEFT: {
		uint32_t const time = SDL_GetTicks();

		//  This hick hack is needed if any of the callback functions calls clear
		//  to forget the last clicked time.
		uint32_t const real_last_click_time = m_last_click_time;

		m_last_selection  = m_selection;
		m_last_click_time = time;

		y = (y + m_scrollpos) / get_lineheight();
		if (y < 0 || static_cast<int32_t>(m_entry_records.size()) <= y)
			return false;
		play_click();
		select(y);
		clicked(m_selection);

		if //  check if doubleclicked
			(time - real_last_click_time < DOUBLE_CLICK_INTERVAL
			 &&
			 m_last_selection == m_selection
			 &&
			 m_selection != no_selection_index())
			double_clicked(m_selection);

		return true;
	}
	default:
		return false;
	}
}

bool BaseListselect::handle_mouserelease(const uint8_t btn, int32_t, int32_t)
{
	return btn == SDL_BUTTON_LEFT;
}

bool BaseListselect::handle_mousemove(uint8_t, int32_t, int32_t y, int32_t, int32_t) {
	y = (y + m_scrollpos) / get_lineheight();
	if (y < 0 || static_cast<int32_t>(m_entry_records.size()) <= y) {
		set_tooltip("");
		return false;
	}
	set_tooltip(m_entry_records.at(y)->tooltip);
	return true;
}

bool BaseListselect::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		uint32_t selected_idx;
		switch (code.sym) {
		case SDLK_KP_2:
			if (code.mod & KMOD_NUM)
				break;
			/* no break */
		case SDLK_DOWN:
			selected_idx = selection_index() + 1;
			if (selected_idx < size())
				select(selected_idx);
			if ((selection_index() + 1) * get_lineheight() - get_inner_h() > m_scrollpos) {
				int32_t scrollpos = (selection_index() + 1) * get_lineheight() - get_inner_h();
				m_scrollpos = (scrollpos < 0) ? 0 : scrollpos;
				m_scrollbar.set_scrollpos(m_scrollpos);
			}
			return true;
		case SDLK_KP_8:
			if (code.mod & KMOD_NUM)
				break;
			/* no break */
		case SDLK_UP:
			selected_idx = selection_index();
			if (selected_idx > 0)
				select(selected_idx - 1);
			if (selection_index() * get_lineheight() < m_scrollpos) {
				m_scrollpos = selection_index() * get_lineheight();
				m_scrollbar.set_scrollpos(m_scrollpos);
			}
			return true;
		default:
			break; // not handled
		}
	}

	return UI::Panel::handle_key(down, code);
}

/**
 * Remove entry
 */
void BaseListselect::remove(const uint32_t i)
{
	assert(i < m_entry_records.size());

	delete (m_entry_records[i]);
	m_entry_records.erase(m_entry_records.begin() + i);
	if (m_selection == i)
		selected(m_selection = no_selection_index());
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
	for (uint32_t i = 0; i < m_entry_records.size(); ++i) {
		if (!strcmp(m_entry_records[i]->name.c_str(), str)) {
			remove(i);
			return;
		}
	}
}

}
