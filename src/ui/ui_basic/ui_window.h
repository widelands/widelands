/*
 * Copyright (C) 2002, 2006-2007 by the Widelands Development Team
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

#ifndef __S__WINDOW_H
#define __S__WINDOW_H

#include "ui_panel.h"

#include <string>

namespace UI {
/**
 *
 * Windows are cached by default.
 *
 * The graphics (see m_pic_*) are used in the following manner: (Example)
 *
 *  <--20leftmostpixel_of_top--><60Pixels as often as possible to reach window with from top><20rightmost pixel of top>
 *  ^
 *  20 topmost pixels of l_border                       <--- > same for r_border
 *  as often as needed: 60 pixels of l_border           <--- > same for r_border
 *  20 bottom pixels of l_border                        <--- > same for r_border
 *  <--20leftmostpixel_of_bot--><60Pixels as often as possible to reach window with from bot><20rightmost pixel of bot>
 *
 * So: the l_border and the r_border pics MUST have a height of 100, while the
 *     width must be 20 and the top and bot pics MUST have a width of 100, while
 *     the height must be 20
 *
 * A click with the middle mouse button (or STRG+LClick) minimizes a window. Minimize means, that
 * the window is only the caption bar, nothing inside. Another click on this bar resizes the window again
 */

struct Window : public Panel {
	Window(Panel *parent, int x, int y, uint w, uint h, const char *title);
	~Window();

	void set_title(const char *text);

	void move_to_mouse();
	virtual void move_inside_parent();
	void center_to_parent();

	bool is_minimal() const throw () {return _is_minimal;}
   void restore ();
   void minimize();
   bool is_snap_target() const {return true;}

	// Drawing and event handlers
	void draw_border(RenderTarget* dst);

	void think();

	bool handle_mousepress  (const Uint8 btn, int mx, int my);
	bool handle_mouserelease(const Uint8 btn, int mx, int my);
	bool handle_mousemove
		(const Uint8 state, int mx, int my, int xdiff, int ydiff);

private:
	void dock_left();
	void undock_left();
	void dock_right();
	void undock_right();
	void dock_bottom();
	void undock_bottom();
	bool _is_minimal;
	uint _oldw, _oldh;  // if it is, these are the old formats
	bool _dragging, _docked_left, _docked_right, _docked_bottom;
	int _drag_start_win_x, _drag_start_win_y, _drag_start_mouse_x, _drag_start_mouse_y;

	std::string m_title;

	uint        m_pic_lborder;
	uint        m_pic_rborder;
	uint        m_pic_top;
	uint        m_pic_bottom;
	uint        m_pic_background;
};
};

#endif
