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

#ifndef __S__BUTTON_H
#define __S__BUTTON_H

/** 
 * This defines a button.
 */
#define BUTTON_EDGE_BRIGHT_FACTOR 60
#define MOUSE_OVER_BRIGHT_FACTOR  15

class Button : public Panel {
public:
	Button(Panel *parent, int x, int y, uint w, uint h, uint background, int id = 0, bool flat = false);
	~Button();

	UISignal clicked;
	UISignal1<int> clickedid;

	void remove_title();
	void set_pic(uint picid);
	void set_title(const char* title);
	void set_enabled(bool on);

	// Drawing and event handlers
	void draw(RenderTarget* dst);

	void handle_mousein(bool inside);
	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
   int	m_id;
	bool	m_highlighted; // mouse is over the button
	bool	m_pressed;
	bool	m_enabled;
   bool	m_flat;

	std::string		m_title;		// title string used when _mypic == 0

	uint		m_pic_background; // background texture (picture ID)
	uint		m_pic_custom; // custom icon on the button

	RGBColor	m_clr_down;		// color of border while a flat button is "down"
};

#endif
