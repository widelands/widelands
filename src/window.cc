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


/////////////////////////////////////////////////////////////////////////////////////////////////


/** class Window
 *
 * The graphics (see pictures) are used in the following manner: (Example)
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


/** Window::Window(Panel *parent, int x, int y, uint w, uint h, const char *title)
 *
 * Initialize a framed window.
 *
 * Args: parent	parent panel
 *       x		coordinates of the window relative to the parent (refers to outer rect!)
 *       y
 *       w		size of the inner rectangle of the window
 *       h
 *       title	string to display in the window title
 */
Window::Window(Panel *parent, int x, int y, uint w, uint h, const char *title)
	: Panel(parent, x, y, w+WINDOW_BORDER*2, h+WINDOW_BORDER*2)
{
	_dragging = false;

	if (title)
		set_title(title);

	set_border(WINDOW_BORDER, WINDOW_BORDER, WINDOW_BORDER, WINDOW_BORDER);
	set_cache(true);
	set_top_on_click(true);

	m_pic_lborder = g_gr->get_picture(PicMod_UI, "pics/win_l_border.bmp", WINDOW_CLRKEY);
	m_pic_rborder = g_gr->get_picture(PicMod_UI, "pics/win_r_border.bmp", WINDOW_CLRKEY);
	m_pic_top = g_gr->get_picture(PicMod_UI, "pics/win_top.bmp", WINDOW_CLRKEY);
	m_pic_bottom = g_gr->get_picture(PicMod_UI, "pics/win_bot.bmp", WINDOW_CLRKEY);
	m_pic_background = g_gr->get_picture(PicMod_UI, "pics/win_bg.bmp");
}

/** Window::~Window()
 *
 * Resource cleanup
 */
Window::~Window()
{
}


/*
===============
Window::set_title

Replace the current title with a new one
===============
*/
void Window::set_title(const char *text)
{
	m_title = text;
	update(0, 0, get_w(), WINDOW_BORDER);
}

/** Window::move_to_mouse()
 *
 * Move the window so that it is under the mouse cursor.
 * Ensure that the window doesn't move out of the screen.
 */
void Window::move_to_mouse()
{
	int px, py;

	px = get_mouse_x() - get_w()/2;
	py = get_mouse_y() - get_h()/2;

	Panel *parent = get_parent();
	if (parent) {
		if (px < 0)
			px = 0;
		if (px+(int)get_w() > parent->get_inner_w())
			px = parent->get_inner_w() - get_w();

		if (py < 0)
			py = 0;
		if (py+(int)get_h() > parent->get_inner_h())
			py = parent->get_inner_h() - get_h();
	}

	set_pos(px, py);
}

/*
===============
Window::draw_border

Redraw the window frame and background
===============
*/
void Window::draw_border(RenderTarget* dst)
{
	int px, py;

	// background
	dst->tile(CORNER, CORNER, get_w() - (2*CORNER), get_h() - (2*CORNER), m_pic_background, 0, 0);

	// top left corner
	dst->blitrect(0, 0, m_pic_top, 0, 0, CORNER, CORNER);
	// bottom left corner
	dst->blitrect(0, get_h()-CORNER, m_pic_bottom, 0, 0, CORNER, CORNER);

	// top & bottom bar
	for(px = CORNER; px < get_w()-CORNER-MIDDLE; px += MIDDLE) {
		dst->blitrect(px, 0, m_pic_top, CORNER, 0, MIDDLE, CORNER);
		dst->blitrect(px, get_h()-CORNER, m_pic_bottom, CORNER, 0, MIDDLE, CORNER);
	}
	// odd pixels of top & bottom bar
	dst->blitrect(px, 0, m_pic_top, CORNER, 0, get_w()-px-CORNER, CORNER);
	dst->blitrect(px, get_h()-CORNER, m_pic_bottom, CORNER, 0, get_w()-px-CORNER, CORNER);

	// top right corner
	dst->blitrect(get_w()-CORNER, 0, m_pic_top, MUST_HAVE_NPIX-CORNER, 0, CORNER, CORNER);
	// bottom right corner
	dst->blitrect(get_w()-CORNER, get_h()-CORNER, m_pic_bottom, MUST_HAVE_NPIX-CORNER, 0, CORNER, CORNER);

	// left top thingy
	dst->blitrect(0, CORNER, m_pic_lborder, 0, 0, CORNER, CORNER);
	// right top thingy
	dst->blitrect(get_w()-CORNER, CORNER, m_pic_rborder, 0, 0, CORNER, CORNER);

	// left & right bars
	for(py = 2*CORNER; py < get_h()-2*CORNER-MIDDLE; py += MIDDLE) {
		dst->blitrect(0, py, m_pic_lborder, 0, CORNER, CORNER, MIDDLE);
		dst->blitrect(get_w()-CORNER, py, m_pic_rborder, 0, CORNER, CORNER, MIDDLE);
	}
	// odd pixels of left & right bars
	dst->blitrect(0, py, m_pic_lborder, 0, CORNER, CORNER, get_h()-py-2*CORNER);
	dst->blitrect(get_w()-CORNER, py, m_pic_rborder, 0, CORNER, CORNER, get_h()-py-2*CORNER);

	// left bottom thingy
	dst->blitrect(0, get_h()-2*CORNER, m_pic_lborder, 0, MUST_HAVE_NPIX-CORNER, CORNER, CORNER);
	// right bottom thingy
	dst->blitrect(get_w()-CORNER, get_h()-2*CORNER, m_pic_rborder, 0, MUST_HAVE_NPIX-CORNER, CORNER, CORNER);

	// draw the title if we have one
	if (m_title.length()) {
		px = get_w() >> 1;
		py = CORNER>>1;
		g_font->draw_string(dst, px, py, m_title.c_str(), Align_Center);
	}
}

/** Window::handle_mouseclick(uint btn, bool down, int mx, int my)
 *
 * Left-click: drag the window
 * Right-click: close the window
 */
bool Window::handle_mouseclick(uint btn, bool down, int mx, int my)
{
	if (btn == MOUSE_LEFT)
	{
		if (down) {
			_dragging = true;
			grab_mouse(true);
		} else if (_dragging) {
			grab_mouse(false);
			_dragging = false;
		}
	}
	else if (btn == MOUSE_RIGHT && down)
		delete this; // is this 100% safe?

	return true;
}

/** Window::handle_mousemove(int mx, int my, int xdiff, int ydiff, uint btns)
 *
 * Drag the mouse if the left mouse button is clicked.
 * Ensure that the window isn't dragged out of the screen.
 */
void Window::handle_mousemove(int mx, int my, int xdiff, int ydiff, uint btns)
{
	if (_dragging)
	{
		int nx = get_x() + xdiff;
		int ny = get_y() + ydiff;

		if (get_parent()) {
			if (nx+(int)get_w() > get_parent()->get_inner_w())
				nx = get_parent()->get_inner_w()-get_w();
			if (nx < 0)
				nx = 0;

			if (ny+(int)get_h() > get_parent()->get_inner_h())
				ny = get_parent()->get_inner_h()-get_h();
			if (ny < 0)
				ny = 0;
		}

		set_pos(nx, ny);
	}
}

