/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "rendertarget.h"
#include "types.h"
#include "ui_listselect.h"
#include "ui_scrollbar.h"
#include "constants.h"

/**
Initialize a list select panel

Args: parent	parent panel
      x		coordinates of the UIListselect
      y
      w		dimensions, in pixels, of the UIListselect
      h
      align	alignment of text inside the UIListselect
*/
UIListselect::UIListselect(UIPanel *parent, int x, int y, uint w, uint h, Align align)
	: UIPanel(parent, x, y, w-24, h)
{
	set_think(false);

	set_align(align);

	m_scrollpos = 0;
	m_selection = -1;

	m_scrollbar = new UIScrollbar(parent, x+get_w(), y, 24, h, false);
	m_scrollbar->moved.set(this, &UIListselect::set_scrollpos);

	m_scrollbar->set_pagesize(h - 2*g_fh->get_fontheight(UI_FONT_SMALL));
	m_scrollbar->set_steps(1);
}


/**
Free allocated resources
*/
UIListselect::~UIListselect()
{
	m_scrollbar = 0;
	clear();
}


/**
Remove all entries from the listselect
*/
void UIListselect::clear()
{
	for(uint i = 0; i < m_entries.size(); i++)
		free(m_entries[i]);
	m_entries.clear();

	if (m_scrollbar)
		m_scrollbar->set_steps(1);
	m_scrollpos = 0;
	m_selection = -1;
}


/**
Add a new entry to the listselect.

Args: name	name that will be displayed
      value	value returned by get_select()
*/
void UIListselect::add_entry(const char *name, void* value)
{
	Entry *e = (Entry *)malloc(sizeof(Entry) + strlen(name));

	e->value = value;
	strcpy(e->name, name);

	m_entries.push_back(e);

	m_scrollbar->set_steps(m_entries.size() * g_fh->get_fontheight(UI_FONT_SMALL) - get_h());

	update(0, 0, get_eff_w(), get_h());
}


/**
Set the list alignment (only horizontal alignment works)
*/
void UIListselect::set_align(Align align)
{
	m_align = (Align)(align & Align_Horizontal);
}


/**
Scroll to the given position, in pixels.
*/
void UIListselect::set_scrollpos(int i)
{
	m_scrollpos = i;

	update(0, 0, get_eff_w(), get_h());
}


/**
 *
 * Change the currently selected entry
 *
 * Args: i	the entry to select
 */
void UIListselect::select(int i)
{
	if (m_selection == i)
		return;

	m_selection = i;

	selected.call(m_selection);
	update(0, 0, get_eff_w(), get_h());
}


/**
Return the total height (text + spacing) occupied by a single line
*/
int UIListselect::get_lineheight()
{
	return g_fh->get_fontheight(UI_FONT_SMALL) + 2;
}


/**
Redraw the listselect box
*/
void UIListselect::draw(RenderTarget* dst)
{
	// draw text lines
	int lineheight = get_lineheight();
	int idx = m_scrollpos / lineheight;
	int y = 1 + idx*lineheight - m_scrollpos;

	while(idx < (int)m_entries.size())
		{
		if (y >= get_h())
			return;

		Entry* e = m_entries[idx];

		if (idx == m_selection) {
			// dst->fill_rect(1, y, get_eff_w()-2, g_font->get_fontheight(), m_selcolor);
			dst->brighten_rect(1, y, get_eff_w()-2, g_fh->get_fontheight(UI_FONT_SMALL), ms_darken_value);
      }

		int x;
		if (m_align & Align_Right)
			x = get_eff_w() - 1;
		else if (m_align & Align_HCenter)
			x = get_eff_w()>>1;
		else
			x = 1;

		g_fh->draw_string(dst, UI_FONT_SMALL, UI_FONT_SMALL_CLR, x, y, e->name, m_align, -1);

		y += lineheight;
		idx++;
	}
}


/**
 * Handle mouse clicks: select the appropriate entry
 */
bool UIListselect::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn != 0) // only left-click
		return false;

	if (down) {
		y = (y + m_scrollpos) / get_lineheight();
		if (y >= 0 && y < (int)m_entries.size())
			select(y);
	}

	return true;
}
