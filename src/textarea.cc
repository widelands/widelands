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

#include "widelands.h"
#include "ui.h"

////////////////////////////////////////////////////////////////////////////////////////7

/** class Textarea
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 *
 * Depends: class Graph::Pic
 * 			class Font_Handler
 */

/** Textarea::Textarea(Panel *parent, int x, int y, const char *text, Align align = 0, uint font = 0)
 *
 * Initialize a Textarea. The dimension are set automatically, depending on the text.
 *
 * Args: parent	parent panel
 *       x		coordinates of the textarea
 *       y
 *       text	text on the Textarea (can be 0)
 *       align	alignment for the text
 *       font	font to be used
 */
Textarea::Textarea(Panel *parent, int x, int y, const char *text, Align align, uint font)
	: Panel(parent, x, y, 0, 0)
{
	set_handle_mouse(false);
	set_think(false);

	_font = font;
	_align = align;
	_textpic = 0;

	if (text)
		set_text(text);
}

/** Textarea::~Textarea()
 *
 * Free allocated resources
 */
Textarea::~Textarea()
{
	if (_textpic)
		delete _textpic;
}

/** Textarea::set_text(const char *text)
 *
 * Set the text of the Textarea. Size is automatically adjusted
 *
 * Args: text	the text string
 */
void Textarea::set_text(const char *text)
{
	if (_textpic) {
		delete _textpic;
		_textpic = 0;
	}

	if (text) {
		collapse(); // collapse() implicitly updates
		_textpic = g_fh.get_string(text, _font);
		expand();
	} else
		update(0, 0, get_w(), get_h());
}

/** Textarea::set_align(Align align)
 *
 * Change the alignment
 *
 * Args: align	new alignment
 */
void Textarea::set_align(Align align)
{
	collapse();
	_align = align;
	expand();
}

/** Textarea::draw(Bitmap *dst, int ofsx, int ofsy)
 *
 * Redraw the Textarea
 */
void Textarea::draw(Bitmap *dst, int ofsx, int ofsy)
{
	if (_textpic)
		copy_pic(dst, _textpic, ofsx, ofsy, 0, 0, _textpic->get_w(), _textpic->get_h());
}

/** Textarea::collapse()
 *
 * Reduce the Textarea to size 0x0 without messing up the alignment
 */
void Textarea::collapse()
{
	int x = get_x();
	int y = get_y();
	int w = get_w();
	int h = get_h();

	if (_align & H_CENTER)
		x += w >> 1;
	else if (_align & H_RIGHT)
		x += w;

	if (_align & V_CENTER)
		y += h >> 1;
	else if (_align & V_BOTTOM)
		y += h;

	set_pos(x, y);
	set_size(0, 0);
}

/** Textarea::expand()
 *
 * Expand the size of the Textarea until it fits the size of the text
 */
void Textarea::expand()
{
	if (!_textpic)
		return;

	int x = get_x();
	int y = get_y();
	int w = _textpic->get_w();
	int h = _textpic->get_h();

	if (_align & H_CENTER)
		x -= w >> 1;
	else if (_align & H_RIGHT)
		x -= w;

	if (_align & V_CENTER)
		y -= h >> 1;
	else if (_align & V_BOTTOM)
		y -= h;

	set_pos(x, y);
	set_size(w, h);
}

