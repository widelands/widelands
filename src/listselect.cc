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

#include "widelands.h"
#include "ui.h"
#include "font.h"

/** class Listselect
 *
 * This class defines a list-select box.
 *
 * Depends: class Pic
 * 			g_fh
 * 			class Button
 */
ushort Listselect::dflt_bgcolor, Listselect::dflt_framecolor, Listselect::dflt_selcolor;

/** Listselect::setup_ui() [static]
 *
 * Initialize global variables; called once from setup_ui
 */
void Listselect::setup_ui()
{
	dflt_bgcolor = pack_rgb(67, 32, 10);
	dflt_framecolor = pack_rgb(0, 0, 0);
	dflt_selcolor = pack_rgb(248, 201, 135);
}

/*
===============
Listselect::Listselect

Initialize a list select panel

Args: parent	parent panel
      x		coordinates of the Listselect
      y
      w		dimensions, in pixels, of the Listselect
      h
      align	alignment of text inside the Listselect
===============
*/
Listselect::Listselect(Panel *parent, int x, int y, uint w, uint h, Align align)
	: Panel(parent, x, y, w-24, h)
{
	set_think(false);

	_bgcolor = dflt_bgcolor;
	_framecolor = dflt_framecolor;
	_selcolor = dflt_selcolor;
	
	set_align(align);

	m_scrollpos = 0;
	m_selection = -1;

	Scrollbar *sb = new Scrollbar(parent, x+get_w(), y, 24, h, false);
	sb->up.set(this, &Listselect::move_up);
	sb->down.set(this, &Listselect::move_down);
}


/*
===============
Listselect::~Listselect

Free allocated resources
===============
*/
Listselect::~Listselect()
{
	clear();
}


/*
===============
Listselect::clear

Remove all entries from the listselect
===============
*/
void Listselect::clear()
{
	for(uint i = 0; i < m_entries.size(); i++)
		free(m_entries[i]);
	m_entries.clear();

	m_scrollpos = 0;
	m_selection = -1;
}


/*
===============
Listselect::add_entry

Add a new entry to the listselect.

Args: name	name that will be displayed
      value	value returned by get_select()
===============
*/
void Listselect::add_entry(const char *name, void* value)
{
	Entry *e = (Entry *)malloc(sizeof(Entry) + strlen(name));
	
	e->value = value;
	strcpy(e->name, name);
	
	m_entries.push_back(e);

	update(0, 0, get_eff_w(), get_h());
}


/*
===============
Listselect::set_align

Set the list alignment (only horizontal alignment works)
===============
*/
void Listselect::set_align(Align align)
{
	m_align = (Align)(align & Align_Horizontal);
}


/*
===============
Listselect::move_up

Scroll the listselect up

Args: i	number of lines to scroll
===============
*/
void Listselect::move_up(int i)
{
	int delta = i * g_font->get_fontheight();

	if (delta > m_scrollpos)
		delta = m_scrollpos;
	if (delta <= 0)
		return;

	m_scrollpos -= delta;
	update(0, 0, get_eff_w(), get_h());
}


/*
===============
Listselect::move_down

Scroll the listselect down
  
Args: i	number of lines to scroll
===============
*/
void Listselect::move_down(int i)
{
	int delta = i * get_lineheight();
	int max = (m_entries.size()+1) * get_lineheight() - get_h();
	if (max < 0)
		max = 0;
	
	if (m_scrollpos + delta > max)
		delta = max - m_scrollpos;
	if (delta <= 0)
		return;
	
	m_scrollpos += delta;
	update(0, 0, get_eff_w(), get_h());
}

/** Listselect::set_colors(ushort bg, ushort frame, ushort sel)
 *
 * Overwrite the default colors used by the listselect
 *
 * Args: bg		background color
 *       frame	frame color
 *       sel	selection color
 */
void Listselect::set_colors(ushort bg, ushort frame, ushort sel)
{
	_bgcolor = bg;
	_framecolor = frame;
	_selcolor = sel;
	update(0, 0, get_eff_w(), get_h());
}

/** Listselect::select(int i)
 *
 * Change the currently selected entry
 *
 * Args: i	the entry to select
 */
void Listselect::select(int i)
{
	if (m_selection == i)
		return;

	m_selection = i;

	selected.call(m_selection);
	update(0, 0, get_eff_w(), get_h());
}


/*
===============
Listselect::get_lineheight

Return the total height (text + spacing) occupied by a single line
===============
*/
int Listselect::get_lineheight()
{
	return g_font->get_fontheight() + 2;
}


/*
===============
Listselect::draw

Redraw the listselect box
===============
*/
void Listselect::draw(Bitmap *dst, int ofsx, int ofsy)
{
	// draw frame and fill with background color
	dst->draw_rect(ofsx, ofsy, get_eff_w(), get_h(), _framecolor);
	dst->fill_rect(ofsx+1, ofsy+1, get_eff_w()-2, get_h()-2, _bgcolor);

	// draw text lines
	int lineheight = get_lineheight();
	int idx = m_scrollpos / lineheight;
	int y = 1 + idx*lineheight - m_scrollpos;
	
	while(idx < (int)m_entries.size())
		{
		if (y >= get_h())
			return;
		
		Entry* e = m_entries[idx];
		
		if (idx == m_selection)
			dst->fill_rect(ofsx+1, ofsy+y, get_eff_w()-2, g_font->get_fontheight(), _selcolor);

		int x;
		if (m_align & Align_Right)
			x = get_eff_w() - 1;
		else if (m_align & Align_HCenter)
			x = get_eff_w()>>1;
		else
			x = 1;
		
		g_font->draw_string(dst, x+ofsx, y+ofsy, e->name, m_align, -1);
		
		y += lineheight;
		idx++;
	}
}


/** Listselect::handle_mouseclick(uint btn, bool down, int x, int y)
 *
 * Handle mouse clicks: select the appropriate entry
 */
bool Listselect::handle_mouseclick(uint btn, bool down, int x, int y)
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
