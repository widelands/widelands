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
#include "font.h"

////////////////////////////////////////////////////////////////////////////////////////7

/** class Multiline_textarea
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 *
 * Depends: class Graph::Pic
 * 			class Font_Handler
 */

/** Multiline_Textarea(Panel *parent, int x, int y, uint w, uint h,
 *                     const char *text, Align align = 0, uint font = 0)
 *
 * Initialize a textarea that supports multiline strings.
 *
 * Args: parent	parent panel
 *       x		coordinates of the textarea
 *       y
 *       w		size of the textarea
 *       h
 *       text	text for the textarea (can be 0)
 *       align	text alignment
 *       font	font number
 */
Multiline_Textarea::Multiline_Textarea(Panel *parent, int x, int y, uint w, uint h,
                                       const char *text, Align align, uint font)
	: Panel(parent, x, y, w-24, h), _lines(1, 3)
{
	set_handle_mouse(false);
	set_think(false);

	_font = font;
	_align = align;
	_firstvis = 0;

	if (text)
		set_text(text);

	Scrollbar *sb = new Scrollbar(parent, x+get_w(), y, 24, h, false);
	sb->up.set(this, &Multiline_Textarea::move_up);
	sb->down.set(this, &Multiline_Textarea::move_down);
}

/** Multiline_Textarea::~Multiline_Textarea()
 *
 * Free allocated resources
 */
Multiline_Textarea::~Multiline_Textarea()
{
	clear();
}

/** Multiline_Textarea::clear()
 *
 * Clear the text
 */
void Multiline_Textarea::clear()
{
	for(int i = 0; i < _lines.elements(); i++)
		delete (Pic *)_lines.element_at(i);
	_lines.flush(1);
	_firstvis = 0;

	update(0, 0, get_w(), get_h());
}

/** Multiline_Textarea::set_text(const char *text)
 *
 * Replace the current text with a new one.
 * New text is broken into multiple lines if necessary, '\n' is recognized.
 *
 * Args: text	the new text
 */
void Multiline_Textarea::set_text(const char *text)
{
	clear();

	if (!text) // consistency with Textarea
		return;

	int i=0;
	int n=0;
	int len=strlen(text);
	char *str = new char[len+1];
	strcpy(str, text);
	str[len]='\n';
	char *buf = new char[len+1];
	do {
		buf[n]=str[i];
		if(buf[n]=='\n' || str[i+1]=='\0') {
			buf[n]='\0';
			Pic* add=g_fh.get_string(buf, _font);
			if(add->get_w() > get_eff_w()) {
up:
				// Big trouble, line doesn't fitt in one, we must do a break
				while(buf[n]!=' ' && buf[n]!='.' && buf[n]!=',' && n) { --n; }
				if(!n) {
					// well, this huge line is just one word, so we just have
					// to take it over, the last few chars are then cut in draw()
					_lines.add(add);
				} else {
					i-=strlen(buf);
					if(buf[n]==' ') buf[n]='\0';
					else buf[n+1]='\0';
					i+=strlen(buf);
					delete add;
					add=(g_fh.get_string(buf, _font));
					if(add->get_w() > get_eff_w()) {
						--n;
						goto up;
					}

					_lines.add(g_fh.get_string(buf, _font));
				}
			} else {
				// Everything is fine
				_lines.add(add);
			}
			n=-1;
		}
		++n;
		++i;
	} while(i<=len);

	delete[] str;
	delete[] buf;

	update(0, 0, get_eff_w(), get_h());
}

/** Multiline_Textarea::move_up(int i)
 *
 * Scroll the area up i lines
 *
 * Args: i	number of lines to scroll
 */
void Multiline_Textarea::move_up(int i)
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

/** Multiline_Textarea::move_down(int i)
 *
 * Scroll down i lines
 *
 * Args: i	number of lines to scroll
 */
void Multiline_Textarea::move_down(int i)
{
	if (i < 0)
		return;
	if ((int)(_firstvis+i) >= _lines.elements())
		i = _lines.elements()-_firstvis-1;
	if (!i)
		return;

	_firstvis += i;
	update(0, 0, get_eff_w(), get_h());
}

/** Multiline_Textarea::draw(Bitmap *bmp, int ofsx, int ofsy)
 *
 * Redraw the textarea
 */
void Multiline_Textarea::draw(Bitmap *bmp, int ofsx, int ofsy)
{
	if (!_lines.elements())
		return;

	int y = 0;
	for(int i = _firstvis; i < _lines.elements(); i++) {
		if (y >= get_h())
			return;

		Pic *txt = (Pic *)_lines.element_at(i);

		int x = 0;
		if (_align == H_RIGHT)
			x = get_eff_w() - txt->get_w();
		else if (_align == H_CENTER)
			x = (get_eff_w() - txt->get_w()) >> 1;

		copy_pic(bmp, txt, x+ofsx, y+ofsy, 0, 0, txt->get_w(), txt->get_h());
		y += txt->get_h() + 2;
	}
}

