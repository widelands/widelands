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
 */

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
	case 0: m_pic_background = g_gr->get_picture(PicMod_UI, "pics/but0.bmp"); break;
	case 1: m_pic_background = g_gr->get_picture(PicMod_UI, "pics/but1.bmp"); break;
	case 2: m_pic_background = g_gr->get_picture(PicMod_UI, "pics/but2.bmp"); break;
	}

	_id = id;
	_highlighted = _pressed = false;
	_enabled = true;
	
	m_pic_custom = 0;
}

/*
===============
Button::~Button

Free any resources associated with the button
===============
*/
Button::~Button()
{
	remove_title();
}


/*
===============
Button::remove_title

Remove any title the button currently carries.
===============
*/
void Button::remove_title()
{
	m_pic_custom = 0;
	m_title = "";
}


/*
===============
Button::set_pic

Sets a new picture for the button.
===============
*/
void Button::set_pic(uint picid)
{
	remove_title();
	
	m_pic_custom = picid;

	update(0, 0, get_w(), get_h());
}


/*
===============
void set_title(const char* title);

Set a text title for the button
===============
*/
void Button::set_title(const char* title)
{
	remove_title();
	
	if (title)
		m_title = title;
	
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


/*
===============
Button::draw

Redraw the button
===============
*/
void Button::draw(RenderTarget* dst)
{
	// Draw the background
	int bgw, bgh;
	int srcy;
	int h;
	
	g_gr->get_picture_size(m_pic_background, &bgw, &bgh);
	
	srcy = get_y() % bgh;
	h = bgh - srcy;

	for(int y = 0; y < get_h(); y += h, srcy = 0, h = bgh) {
		int srcx = get_x() % bgw;
		int w = bgw - srcx;
		for(int x = 0; x < get_w(); x += w, srcx = 0, w = bgw)
			dst->blitrect(x, y, m_pic_background, srcx, srcy, w, h);
	}

	if (_enabled && _highlighted)
		dst->brighten_rect(0, 0, get_w(), get_h(), MOUSE_OVER_BRIGHT_FACTOR);

	// if we got a picture, draw it centered
	if (m_pic_custom)
		{
		int cpw, cph;
		
		g_gr->get_picture_size(m_pic_custom, &cpw, &cph);
		
		dst->blit((get_w() - cpw) >> 1, (get_h() - cph) >> 1, m_pic_custom);
		}
	else if (m_title.length()) // otherwise draw the title string centered
		{
		g_font->draw_string(dst, get_w()>>1, get_h()>>1, 
		                    m_title.c_str(), Align_Center);
		}

	// draw border
	// a pressed but not highlighted button occurs when the user has pressed
	// the left mouse button and then left the area of the button
	RGBColor black(0,0,0);
	
	if (!_pressed || !_highlighted)
	{
		// top edge
		dst->brighten_rect(0, 0, get_w(), 2, BUTTON_EDGE_BRIGHT_FACTOR);
		// left edge
		dst->brighten_rect(0, 2, 2, get_h()-2, BUTTON_EDGE_BRIGHT_FACTOR);
		// bottom edge
		dst->fill_rect(2, get_h()-2, get_w()-2, 1, black);
		dst->fill_rect(1, get_h()-1, get_w()-1, 1, black);
		// right edge
		dst->fill_rect(get_w()-2, 2, 1, get_h()-2, black);
		dst->fill_rect(get_w()-1, 1, 1, get_h()-1, black);
	}
	else
	{
		// bottom edge
		dst->brighten_rect(0, get_h()-2, get_w(), 2, BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dst->brighten_rect(get_w()-2, 0, 2, get_h()-2, BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dst->fill_rect(0, 0, get_w()-1, 1, black);
		dst->fill_rect(0, 1, get_w()-2, 1, black);
		// left edge
		dst->fill_rect(0, 0, 1, get_h()-1, black);
		dst->fill_rect(1, 0, 1, get_h()-2, black);
	}
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

