/*
 * Copyright (C) 2002 by Holger Rapp,
 *                       Nicolai Haehnle
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

#include "ui.h"
#include "font.h"
#include <string.h>

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

/** Listselect(Panel *parent, int x, int y, uint w, uint h, Align align = 0, uint font = 0)
 *
 * Initialize a list select panel
 *
 * Args: parent	parent panel
 *       x		coordinates of the Listselect
 *       y
 *       w		dimensions, in pixels, of the Listselect
 *       h
 *       align	alignment of text inside the Listselect
 *       font	font to be used for listselect
 */
Listselect::Listselect(Panel *parent, int x, int y, uint w, uint h, Align align, uint font)
	: Panel(parent, x, y, w-24, h), _entries(5, 5)
{
	set_think(false);

	_bgcolor = dflt_bgcolor;
	_framecolor = dflt_framecolor;
	_selcolor = dflt_selcolor;
	_font = font;
	_align = align;

	_firstvis = 0;
	_selection = -1;

	Scrollbar *sb = new Scrollbar(parent, x+get_w(), y, 24, h, false);
	sb->up.set(this, &Listselect::move_up);
	sb->down.set(this, &Listselect::move_down);
}

/** Listselect::~Listselect()
 *
 * Free allocated resources
 */
Listselect::~Listselect()
{
	clear();
}

/** Listselect::clear()
 *
 * Remove all entries from the listselect
 */
void Listselect::clear()
{
	for(int i = 0; i < _entries.elements(); i++) {
		Entry *e = (Entry *)_entries.element_at(i);
		delete e->pic;
		free(e);
	}
	_entries.flush(1);

	_firstvis = 0;
	_selection = -1;
}

/** Listselect::add_entry(const char *name, const char *value = 0)
 *
 * Add a new entry to the listselect.
 *
 * Args: name	name that will be displayed
 *       value	string returned by get_select() (can be 0, in which case name is used)
 */
void Listselect::add_entry(const char *name, const char *value = 0)
{
	if (!value)
		value = name;

	Entry *e = (Entry *)malloc(sizeof(Entry) + strlen(value));
	e->pic = g_fh.get_string(name, _font);
	strcpy(e->str, value);
	_entries.add(e);

	update(0, 0, get_eff_w(), get_h());
}

/** Listselect::move_up(int i)
 *
 * Scroll the listselect up
 *
 * Args: i	number of lines to scroll
 */
void Listselect::move_up(int i)
{
	if (i < 0)
		return;
	if (i > (int)_firstvis)
		i = _firstvis;
	if (!i)
		return;

	_firstvis -= i;
	update(0, 0, get_eff_w(), get_h());
}

/** Listselect::move_down(int i)
 *
 * Scroll the listselect down
 *
 * Args: i	number of lines to scroll
 */
void Listselect::move_down(int i)
{
	if (i < 0)
		return;
	if ((int)(_firstvis+i) >= _entries.elements())
		i = _entries.elements()-_firstvis-1;
	if (!i)
		return;

	_firstvis += i;
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
	if (_selection == i)
		return;

	_selection = i;

	selected.call(_selection);
	update(0, 0, get_eff_w(), get_h());
}

/** Listselect::draw(Bitmap *dst, int ofsx, int ofsy)
 *
 * Redraw the listselect box
 */
void Listselect::draw(Bitmap *dst, int ofsx, int ofsy)
{
	// draw frame and fill with background color
	dst->draw_rect(ofsx, ofsy, get_eff_w(), get_h(), _framecolor);
	dst->fill_rect(ofsx+1, ofsy+1, get_eff_w()-2, get_h()-2, _bgcolor);

	// draw text lines
	int y = 1;
	for(int i = _firstvis; i < _entries.elements(); i++) {
		if (y >= get_h())
			return;

		Entry *e = (Entry *)_entries.element_at(i);

		if (i == _selection)
			dst->fill_rect(ofsx+1, ofsy+y, get_eff_w()-2, e->pic->get_h(), _selcolor);

		int x = 1;
		if (_align == H_RIGHT)
			x = get_eff_w()-e->pic->get_w()-2;
		else if (_align == H_CENTER)
			x = (get_eff_w()-e->pic->get_w()-2) >> 1;
		copy_pic(dst, e->pic, x+ofsx, y+ofsx, 0, 0, e->pic->get_w(), e->pic->get_h());
		y += e->pic->get_h() + 2;
	}
}

/** Listselect::handle_mouseclick(uint btn, bool down, int x, int y)
 *
 * Handle mouse clicks: select the appropriate entry
 */
void Listselect::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn || !down) // only left-click
		return;

	y = y / (g_fh.get_fh(_font)+2);
	y += _firstvis;
	if ((int)y < _entries.elements())
		select(y);
}
