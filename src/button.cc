/*
 * Copyright (C) 2002 by Widelands Development Team
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

/** class Button
 *
 * This defines a button.
 *
 * Depends: g_gr
 * 			class Graph::Pic
 * 			class Font_Handler
 */

AutoPic Button::bg0("but0.bmp");
AutoPic Button::bg1("but1.bmp");
AutoPic Button::bg2("but2.bmp");
Pic Button::bg0e;
Pic Button::bg1e;
Pic Button::bg2e;

/** Button::setup_ui() [static]
 *
 * Create the highlighted background pics. Called once by setup_ui().
 */
void Button::setup_ui()
{
	bg0e = bg0;
	bg0e.brighten_rect(0, 0, bg0e.get_w(), bg0e.get_h(), MOUSE_OVER_BRIGHT_FACTOR);

	bg1e = bg1;
	bg1e.brighten_rect(0, 0, bg1e.get_w(), bg1e.get_h(), MOUSE_OVER_BRIGHT_FACTOR);

	bg2e = bg2;
	bg2e.brighten_rect(0, 0, bg2e.get_w(), bg2e.get_h(), MOUSE_OVER_BRIGHT_FACTOR);
}

/** Button::Button(Panel *parent, int x, int y, uint w, uint h, uint background)
 *
 * Initialize a Button
 *
 * Args: parent			parent panel
 *       x, y, w, h		button dimensions
 *       background		number of the button style (0..2)
 */
Button::Button(Panel *parent, int x, int y, uint w, uint h, uint background, int id)
	: Panel(parent, x, y, w, h)
{
	set_think(false);

	switch(background) {
	default:
	case 0:
		_mybg = &bg0;
		_mybge = &bg0e;
		break;

	case 1:
		_mybg = &bg1;
		_mybge = &bg1e;
		break;

	case 2:
		_mybg = &bg2;
		_mybge = &bg2e;
		break;
	}

	_id = id;
	_mypic = 0;
	_highlighted = _pressed = false;
	_enabled = true;
}

/** Button::~Button()
 *
 * Free any resources associated with the button
 */
Button::~Button()
{
	if (_mypic)
		delete _mypic;
}

/** Button::set_pic(Pic *pic)
 *
 * Sets a new picture for the button.
 *
 * Args: pic	the picture
 */
void Button::set_pic(Pic *pic)
{
	if (_mypic)
		delete _mypic;
	_mypic = pic;

	update(0, 0, get_w(), get_h());
}

/** Button::set_enabled(bool on)
 *
 * Enable/Disable the button (disabled buttons can't be clicked).
 * Buttons are enabled by default
 *
 * Args: on	true if button should be enabled
 */
void Button::set_enabled(bool on)
{
	// disabled buttons should look different...
	if (on)
		_enabled = true;
	else {
		_enabled = false;
		_pressed = false;
		_highlighted = false;
	}
	update(0, 0, get_w(), get_h());
}

/** Button::draw(Bitmap *dst, int ofsx, int ofsy)
 *
 * Redraw the button
 */
void Button::draw(Bitmap *dst, int ofsx, int ofsy)
{
	Pic *bg;

	if (!_enabled || !_highlighted)
		bg = _mybg;
	else
		bg = _mybge;

	int srcy = get_y() % bg->get_h();
	int h = bg->get_h() - srcy;

	for(int y = 0; y < get_h(); y += h, srcy = 0, h = bg->get_h()) {
		int srcx = get_x() % bg->get_w();
		int w = bg->get_w() - srcx;
		for(int x = 0; x < get_w(); x += w, srcx = 0, w = bg->get_w())
			copy_pic(dst, bg, x+ofsx, y+ofsy, srcx, srcy, w, h);
	}

	// if we got a picture, draw it centered
	if (_mypic) {
		int x = (get_w() - _mypic->get_w()) >> 1;
		int y = (get_h() - _mypic->get_h()) >> 1;

		copy_pic(dst, _mypic, x+ofsx, y+ofsy, 0, 0, _mypic->get_w(), _mypic->get_h());
	}

	// draw border
#define BLACK 0x000000	//TEMP
	// a pressed but not highlighted button occurs when the user has pressed
	// the left mouse button and then left the area of the button
	if (!_pressed || !_highlighted)
	{
		// top edge
		dst->brighten_rect(ofsx, ofsy, get_w(), 2, BUTTON_EDGE_BRIGHT_FACTOR);
		// left edge
		dst->brighten_rect(ofsx, ofsy+2, 2, get_h()-2, BUTTON_EDGE_BRIGHT_FACTOR);
		// bottom edge
		dst->fill_rect(ofsx+2, ofsy+get_h()-2, get_w()-2, 1, BLACK);
		dst->fill_rect(ofsx+1, ofsy+get_h()-1, get_w()-1, 1, BLACK);
		// right edge
		dst->fill_rect(ofsx+get_w()-2, ofsy+2, 1, get_h()-2, BLACK);
		dst->fill_rect(ofsx+get_w()-1, ofsy+1, 1, get_h()-1, BLACK);
	}
	else
	{
		// bottom edge
		dst->brighten_rect(ofsx, ofsy+get_h()-2, get_w(), 2, BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dst->brighten_rect(ofsx+get_w()-2, ofsy, 2, get_h()-2, BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dst->fill_rect(ofsx, ofsy+0, get_w()-1, 1, BLACK);
		dst->fill_rect(ofsx, ofsy+1, get_w()-2, 1, BLACK);
		// left edge
		dst->fill_rect(ofsx+0, ofsy, 1, get_h()-1, BLACK);
		dst->fill_rect(ofsx+1, ofsy, 1, get_h()-2, BLACK);
	}
#undef BLACK
}

/** Button::handle_mousein(bool inside)
 *
 * Update highlighted & pressed status
 */
void Button::handle_mousein(bool inside)
{
	if (inside && _enabled)
		_highlighted = true;
	else
		_highlighted = false;
	update(0, 0, get_w(), get_h());
}

/** Button::handle_mouseclick(uint btn, bool down, int x, int y)
 *
 * Update the pressed status of the button
 */
bool Button::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn != 0) // only react on left button
		return false;

	if (down && _enabled) {
		grab_mouse(true);
		_pressed = true;
	} else {
		if (_pressed) {
			grab_mouse(false);
			if (_highlighted && _enabled) {
				clicked.call();
				clickedid.call(_id);
			}
			_pressed = false;
		}
	}
	update(0, 0, get_w(), get_h());

	return true;
}

