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
#include "input.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/** class Window
 *
 * The graphics (see static Pics) are used in the following manner: (Example)
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
 *
 * DEPENDS: Graph::Pic
 * 			Graph::draw_pic
 * 			Initalized g_gr object
 */

AutoPic Window::l_border("win_l_border.bmp", CORNER, MUST_HAVE_NPIX);
AutoPic Window::r_border("win_r_border.bmp", CORNER, MUST_HAVE_NPIX);
AutoPic Window::top("win_top.bmp", MUST_HAVE_NPIX, CORNER);
AutoPic Window::bot("win_bot.bmp", MUST_HAVE_NPIX, CORNER);
AutoPic Window::bg("win_bg.bmp");


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
	_title = 0;
	_custom_bg = 0;
	_dragging = false;

	if (title)
		set_title(title);

	set_border(WINDOW_BORDER, WINDOW_BORDER, WINDOW_BORDER, WINDOW_BORDER);
	set_cache(true);
}

/** Window::~Window()
 *
 * Resource cleanup
 */
Window::~Window()
{
	if (_title)
		delete _title;
	if (_custom_bg)
		delete _custom_bg;
}

/** Window::set_title(const char *text)
 *
 * Replace the current title with a new one
 *
 * Args: text	title string
 */
void Window::set_title(const char *text)
{
	if (_title) {
		delete _title;
		_title = 0;
	}
	if (text)
		_title = g_fh.get_string(text, 0);
	update(0, 0, get_w(), WINDOW_BORDER);
}

/** Window::set_new_bg(Pic* p)
 *
 * Set a custom background picture.
 * Window will ensure that the picture is freed.
 *
 * Args: p	custom background picture
 */
void Window::set_new_bg(Pic* p)
{
	if (_custom_bg)
		delete _custom_bg;
	_custom_bg = p;

	update(0, 0, get_w(), get_h());
}

/** Window::move_to_mouse()
 *
 * Move the window so that it is under the mouse cursor.
 * Ensure that the window doesn't move out of the screen.
 */
void Window::move_to_mouse()
{
	int px, py;

	px = g_ip.get_mpx() - get_w()/2;
	py = g_ip.get_mpy() - get_h()/2;

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

/** Window::draw_border(Bitmap *dst, int ofsx, int ofsy)
 *
 * Redraw the window frame and background
 */
void Window::draw_border(Bitmap *dst, int ofsx, int ofsy)
{
	Pic *usebg = _custom_bg ? _custom_bg : &bg;
	int px, py;

	// fill background
	for(py = CORNER; py < get_h()-CORNER; py += usebg->get_h()) {
		for(px = CORNER; px < get_w()-CORNER; px += usebg->get_w())
			copy_pic(dst, usebg, ofsx+px, ofsy+py, 0, 0, usebg->get_w(), usebg->get_h());
	}

	// top left corner
	copy_pic(dst, &top, ofsx, ofsy, 0, 0, CORNER, CORNER);
	// bottom left corner
	copy_pic(dst, &bot, ofsx, ofsy+get_h()-CORNER, 0, 0, CORNER, CORNER);

	// top & bottom bar
	for(px = CORNER; px < get_w()-CORNER-MIDDLE; px += MIDDLE) {
		copy_pic(dst, &top, ofsx+px, ofsy, CORNER, 0, MIDDLE, CORNER);
		copy_pic(dst, &bot, ofsx+px, ofsy+get_h()-CORNER, CORNER, 0, MIDDLE, CORNER);
	}
	// odd pixels of top & bottom bar
	copy_pic(dst, &top, ofsx+px, ofsy, CORNER, 0, get_w()-px-CORNER, CORNER);
	copy_pic(dst, &bot, ofsx+px, ofsy+get_h()-CORNER, CORNER, 0, get_w()-px-CORNER, CORNER);

	// top right corner
	copy_pic(dst, &top, ofsx+get_w()-CORNER, ofsy, MUST_HAVE_NPIX-CORNER, 0, CORNER, CORNER);
	// bottom right corner
	copy_pic(dst, &bot, ofsx+get_w()-CORNER, ofsy+get_h()-CORNER, MUST_HAVE_NPIX-CORNER, 0, CORNER, CORNER);

	// left top thingy
	copy_pic(dst, &l_border, ofsx, ofsy+CORNER, 0, 0, CORNER, CORNER);
	// right top thingy
	copy_pic(dst, &r_border, ofsx+get_w()-CORNER, ofsy+CORNER, 0, 0, CORNER, CORNER);

	// left & right bars
	for(py = 2*CORNER; py < get_h()-2*CORNER-MIDDLE; py += MIDDLE) {
		copy_pic(dst, &l_border, ofsx, ofsy+py, 0, CORNER, CORNER, MIDDLE);
		copy_pic(dst, &r_border, ofsx+get_w()-CORNER, ofsy+py, 0, CORNER, CORNER, MIDDLE);
	}
	// odd pixels of left & right bars
	copy_pic(dst, &l_border, ofsx, ofsy+py, 0, CORNER, CORNER, get_h()-py-2*CORNER);
	copy_pic(dst, &r_border, ofsx+get_w()-CORNER, ofsy+py, 0, CORNER, CORNER, get_h()-py-2*CORNER);

	// left bottom thingy
	copy_pic(dst, &l_border, ofsx, ofsy+get_h()-2*CORNER, 0, MUST_HAVE_NPIX-CORNER, CORNER, CORNER);
	// right bottom thingy
	copy_pic(dst, &r_border, ofsx+get_w()-CORNER, ofsy+get_h()-2*CORNER, 0, MUST_HAVE_NPIX-CORNER, CORNER, CORNER);

	// draw the title if we have one
	if (_title) {
		px = (get_w()-_title->get_w()) >> 1;
		py = (CORNER-_title->get_h()) >> 1;
		copy_pic(dst, _title, ofsx+px, ofsy+py, 0, 0, _title->get_w(), _title->get_h());
	}
}

/** Window::handle_mouseclick(uint btn, bool down, int mx, int my)
 *
 * Left-click: drag the window
 * Right-click: close the window
 */
bool Window::handle_mouseclick(uint btn, bool down, int mx, int my)
{
	if (btn == 0)
	{
		if (down) {
			_dragging = true;
			grab_mouse(true);
		} else if (_dragging) {
			grab_mouse(false);
			_dragging = false;
		}
	}
	else if (btn == 1 && down)
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

