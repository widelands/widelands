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

#ifndef __S__WINDOW_H
#define __S__WINDOW_H

#include <string>
#include "types.h"
#include "ui_panel.h"

/**
 *
 * UIWindows are cached by default.
 *
 * The graphics (see m_pic_*) are used in the following manner: (Example)
 *
 *  <--20leftmostpixel_of_top--><60Pixels as often as possible to reach window with from top><20rightmost pixel of top>
 *  ^
 *  20 topmost pixels of l_border                                      <--- > same for r_border
 *  as often as needed: 60 pixels of l_border								  <--- > same for r_border
 *  20 bottom pixels of l_border													  <--- > same for r_borde
 *  <--20leftmostpixel_of_bot--><60Pixels as often as possible to reach window with from bot><20rightmost pixel of bot>
 *
 * So: the l_border and the r_border pics MUST have a height of 100, while the width must be  20
 * 	 and the top and bot pics MUST have a width of 100, while the height must be 20
 */

// widht/height the graphs above must have
#define MUST_HAVE_NPIX	100
// width/height to use as the corner
#define CORNER			20
#define MIDDLE			(MUST_HAVE_NPIX-(CORNER*2))

#define WINDOW_BORDER	20
class UIWindow : public UIPanel {
public:
	UIWindow(UIPanel *parent, int x, int y, uint w, uint h, const char *title);
	~UIWindow();

	void set_title(const char *text);

	void move_to_mouse();
	void center_to_parent();

	// Drawing and event handlers
	void draw_border(RenderTarget* dst);

	bool handle_mouseclick(uint btn, bool down, int mx, int my);
	void handle_mousemove(int mx, int my, int xdiff, int ydiff, uint btns);

private:

	bool _dragging;

	std::string		m_title;

	uint	m_pic_lborder;
	uint	m_pic_rborder;
	uint	m_pic_top;
	uint	m_pic_bottom;
	uint	m_pic_background;
};

#endif
