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
#include "input.h"
#include "cursor.h"

/*
==============================================================================

Panel

==============================================================================
*/

Panel *Panel::_modal = 0;

/** Panel::Panel(Panel *nparent, const int nx, const int ny, const uint nw, const uint nh)
 *
 * Initialize a panel, link it into the parent's queue.
 */
Panel::Panel(Panel *nparent, const int nx, const int ny, const uint nw, const uint nh)
{
	_parent = nparent;
	_fchild = _lchild = 0;

	if (_parent) {
		_next = _parent->_fchild;
		_prev = 0;
		if (_next)
			_next->_prev = this;
		else
			_parent->_lchild = this;
		_parent->_fchild = this;
	} else
		_prev = _next = 0;

	_mousein = 0;

	_x = nx;
	_y = ny;
	_w = nw;
	_h = nh;

	_flags = pf_handle_mouse|pf_think;
	update(0, 0, _w, _h);
}

/** Panel::~Panel()
 *
 * Unlink the panel from the parent's queue
 */
Panel::~Panel()
{
	while(_fchild)
		delete _fchild;

	if (_parent) {
		if (_prev)
			_prev->_next = _next;
		else
			_parent->_fchild = _next;
		if (_next)
			_next->_prev = _prev;
		else
			_parent->_lchild = _prev;
	}
}

/** Panel::run()
 *
 * Enters the event loop; all events will be handled by this panel.
 *
 * Returns: the return code passed to end_modal
 */
int Panel::run()
{
	// Setup
	g_ip.register_mcf(Panel::ui_mouseclick, Input::BUT1, (void *)0);
	g_ip.register_mcf(Panel::ui_mouseclick, Input::BUT2, (void *)1);
	g_ip.register_mmf(Panel::ui_mousemove);

	Panel *prevmodal = _modal;
	_modal = this;

	Panel *forefather = this;
	while(forefather->_parent)
		forefather = forefather->_parent;

	// Loop
	_running = true;
	start();
	g_gr.needs_fs_update();
	while(_running)
	{
		g_ip.handle_pending_input();
		if (g_ip.should_die())
			end_modal(-1);

		// call logic code here?

		if(g_gr.does_need_update()) {
			forefather->do_draw(g_gr.get_screenbmp(), 0, 0);
			g_cur.draw(g_ip.get_mpx(), g_ip.get_mpy());
			g_gr.update();
		}
	}
	g_gr.needs_fs_update();
	end();

	// Done
	_modal = prevmodal;

	return _retcode;
}

/** Panel::end_modal(int code)
 *
 * Cause run() to return as soon as possible, with the given return code
 *
 * Args: code	run() return code
 */
void Panel::end_modal(int code)
{
	_running = false;
	_retcode = code;
}

/** Panel::start()
 *
 * Called once before the event loop in run is started
 */
void Panel::start()
{
}

/** Panel::end()
 *
 * Called once after the event loop in run() has ended
 */
void Panel::end()
{
}

/** Panel::set_size(const uint nw, const uint nh)
 *
 * Resizes the panel.
 *
 * Args: nw		new width
 *       nh		new height
 */
void Panel::set_size(const uint nw, const uint nh)
{
	uint upw = _w;
	uint uph = _h;
	_w = nw;
	_h = nh;

	if (nw > upw) upw = nw;
	if (nh > uph) uph = nh;
	update(0, 0, upw, uph);
}

/** Panel::set_pos(const int nx, const int ny)
 *
 * Move the panel. Panel's position is relative to the parent.
 *
 * Args: nx		new x coordinate
 *       ny		new y coordinate
 */
void Panel::set_pos(const int nx, const int ny)
{
	update(0, 0, _w, _h);
	_x = nx;
	_y = ny;
	update(0, 0, _w, _h);
}

/** Panel::draw(Bitmap *dst, int ofsx, int ofsy) [virtual]
 *
 * Redraw the panel.
 *
 * Args: dst	the destination bitmap
 *       ofsx	an offset that should be added to all coordinates
 *       ofsy
 */
void Panel::draw(Bitmap *dst, int ofsx, int ofsy)
{
}

/** Panel::update(int x, int y, int w, int h);
 *
 * Mark a part of a panel for updating
 *
 * Args: x	coordinates of the rectangle, relative to the panel
 *       y
 *       w	size of the rectangle
 *       h
 */
void Panel::update(int x, int y, int w, int h)
{
	if (x >= (int)_w || x+w <= 0 ||
	    y >= (int)_h || y+h <= 0)
		return;

	if (_parent) {
		_parent->update(x+_x, y+_y, w, h);
	} else {
		if (x < 0) {
			w += x;
			x = 0;
		}
		if (x+w > (int)g_gr.get_xres())
			w = g_gr.get_xres() - x;
		if (w <= 0)
			return;

		if (y < 0) {
			h += y;
			y = 0;
		}
		if (y+h > (int)g_gr.get_yres())
			h = g_gr.get_yres() - y;
		if (h <= 0)
			return;

		g_gr.register_update_rect(x, y, w, h);
	}
}

/** Panel::think()
 *
 * Called once per event loop pass, unless set_think(false) has
 * been called. It is intended to be used for animations and game logic.
 * The default implementation calls the children's think function.
 */
void Panel::think()
{
	for(Panel *child = _fchild; child; child = child->_next) {
		if (child->get_think())
			child->think();
	}
}

/** Panel::handle_mousein(uint x, uint y, bool inside)
 *
 * Called whenever the mouse enters or leaves the panel.
 *
 * Args: inside	true if the cursor has entered the panel
 */
void Panel::handle_mousein(bool inside)
{
}

/** Panel::handle_mouseclick(uint btn, bool down, uint x, uint y)
 *
 * Called whenever the user clicks into the panel
 *
 * Args: btn	0 = left, 1 = right
 *       down	true if the button was pressed, false if released
 *       x		mouse coordinates relative to the panel
 *       y
 */
void Panel::handle_mouseclick(uint btn, bool down, uint x, uint y)
{
}

/** Panel::handle_mousemove(uint x, uint y, uint btns)
 *
 * Called when the mouse is moved while inside the panel
 *
 * Args: x		mouse coordinates relative to the panel
 *       y
 *       xdiff	relative mouse movement
 *       ydiff
 *       btns	bitmask of currently pressed buttons: (1<<0) left, (1<<1) right
 */
void Panel::handle_mousemove(uint x, uint y, int xdiff, int ydiff, uint btns)
{
}

/** Panel::set_handle_mouse(bool yes)
 *
 * Enable/Disable mouse handling by this panel
 * Default is enabled. Note that when mouse handling is disabled, child panels
 * don't receive mouse events either.
 *
 * Args: yes	true if the panel should receive mouse events
 */
void Panel::set_handle_mouse(bool yes)
{
	if (yes)
		_flags |= pf_handle_mouse;
	else
		_flags &= ~pf_handle_mouse;
}

/** Panel::set_think(bool yes)
 *
 * Enables/Disables calling think() during the event loop.
 * The default is enabled.
 *
 * Args: yes	true if the panel's think function should be called
 */
void Panel::set_think(bool yes)
{
	if (yes)
		_flags |= pf_think;
	else
		_flags &= ~pf_think;
}

/** Panel::do_draw(Bitmap *dst, int ofsx, int ofsy) [private]
 *
 * Adjust the drawing bitmap to clip to our own rectangle.
 * Draw child panels after drawing self.
 */
void Panel::do_draw(Bitmap *dst, int ofsx, int ofsy)
{
	Bitmap mydst;
	int dx, dy;

	dx = _x + ofsx;
	if (dx >= (int)dst->w || dx <= -(int)_w)
		return;
	if (dx < 0) {
		ofsx = dx;
		dx = 0;
	} else
		ofsx = 0;

	dy = _y + ofsy;
	if (dy >= (int)dst->h || dy <= -(int)_h)
		return;
	if (dy < 0) {
		ofsy = dy;
		dy = 0;
	} else
		ofsy = 0;

	mydst.pitch = dst->pitch;
	mydst.pixels = dst->pixels + dy*mydst.pitch + dx;

	mydst.w = dst->w - dx;
	if (mydst.w > _w+ofsx)
		mydst.w = _w+ofsx;
	mydst.h = dst->h - dy;
	if (mydst.h > _h+ofsy)
		mydst.h = _h+ofsy;

	draw(&mydst, ofsx, ofsy);

	// draw back to front
	for(Panel *child = _lchild; child; child = child->_prev)
		child->do_draw(&mydst, ofsx, ofsy);
}

/** Panel::get_mousein(uint x, uint y)
 *
 * Return the panel that receives mouse clicks at the given location
 *
 * Args: x	mouse coordinates relative to panel
 *       y
 *
 * Returns: topmost panel at the given coordinates
 */
Panel *Panel::get_mousein(uint x, uint y)
{
	Panel *child;

	for(child = _fchild; child; child = child->_next) {
		if (!child->get_handle_mouse())
			continue;
		if (x < child->_x+child->_w && (int)x >= child->_x &&
		    y < child->_y+child->_h && (int)y >= child->_y)
			break;
	}

	if (_mousein && _mousein != child)
		_mousein->do_mousein(false);
	_mousein = child;
	if (child)
		child->do_mousein(true);

	return child;
}

/** Panel::do_mousein(bool inside)
 *
 * Propagate mouseleave events (e.g. for buttons that are inside a different
 * window)
 *
 * Args: inside		true if the mouse enters the panel
 */
void Panel::do_mousein(bool inside)
{
	if (!inside && _mousein) {
		_mousein->do_mousein(false);
		_mousein = false;
	}
	handle_mousein(inside);
}

/** Panel::do_mouseclick(uint btn, bool down, uint x, uint y)
 *
 * Propagate mouse clicks to the appropriate panel.
 *
 * Args: btn	button number (0 = left, 1 = right)
 *       down	true if button was pressed
 *       x		mouse coordinates relative to panel
 *       y
 */
void Panel::do_mouseclick(uint btn, bool down, uint x, uint y)
{
	Panel *child = get_mousein(x, y);

	if (child)
		child->do_mouseclick(btn, down, x-child->_x, y-child->_y);
	else
		handle_mouseclick(btn, down, x, y);
}

/** Panel::do_mousemove(uint x, uint y, uint btns)
 *
 * Propagate mouse movement to the appropriate panel.
 *
 * Args: x		mouse coordinates relative to panel
 *       y
 *       xdiff	relative mouse movement
 *       ydiff
 *       btns	bitmask of pressed buttons
 */
void Panel::do_mousemove(uint x, uint y, int xdiff, int ydiff, uint btns)
{
	Panel *child = get_mousein(x, y);

	if (child)
		child->do_mousemove(x-child->_x, y-child->_y, xdiff, ydiff, btns);
	else
		handle_mousemove(x, y, xdiff, ydiff, btns);
}

/** Panel::ui_mouseclick(const bool down, const uint mx, const uint my, void *a) [static]
 *
 * Input callback function. Pass the mouseclick event to the currently modal
 * panel.
 *
 * Args: down	true if the button was pressed
 *       mx		screen coordinates
 *       my
 *       a		parameter (will be the button number)
 */
int Panel::ui_mouseclick(const bool down, const uint mx, const uint my, void *a)
{
	int x = mx - _modal->_x;
	int y = my - _modal->_y;

//	printf("mouse click %i: %i %i (%s)\n", (int)a, x, y, down ? "down" : "up");

	if (x < 0 || x >= (int)_modal->_w ||
	    y < 0 || y >= (int)_modal->_h)
		return 0;

	_modal->do_mouseclick((int)a, down, x, y);
	return 1;
}

/** Panel::ui_mousemove(const uint mx, const uint my, const int xdiff, const int ydiff,
 *                      const bool lbtn, const bool rbtn, void *a)
 *
 * Input callback function. Pass the mousemove event to the currently modal
 * panel.
 *
 * Args: mx		screen coordinates
 *       my
 *       xdiff	relative mouse movement
 *       ydiff
 *       lbtn	true if the left button is pressed
 *       rbtn	true if the right button is pressed
 *       a		parameter (unused)
 */
int Panel::ui_mousemove(const uint mx, const uint my, const int xdiff, const int ydiff,
	                    const bool lbtn, const bool rbtn, void *a)
{
	if (!xdiff && !ydiff)
		return 0;

	int x = mx - _modal->_x;
	int y = my - _modal->_y;

//	printf("mouse move: %i %i\n", x, y);
	g_gr.register_update_rect(g_ip.get_mplx(), g_ip.get_mply(), g_cur.get_w(), g_cur.get_h());
	g_gr.register_update_rect(mx, my, g_cur.get_w(), g_cur.get_h());

	if (x < 0 || x >= (int)_modal->_w ||
	    y < 0 || y >= (int)_modal->_h)
		return 0;

	uint btns = 0;
	if (lbtn)
		btns |= 1;
	if (rbtn)
		btns |= 2;

	_modal->do_mousemove(x, y, xdiff, ydiff, btns);
	return 1;
}

/*
==============================================================================

User_Interface

==============================================================================
*/

/** class User_Interface
 *
 * This class used to offer a user interface. This means windows, buttons and
 * so on. I'll leave it in for a while, maybe it'll be useful for something.
 *
 * It's a Singleton.
 *
 * DEPENDS: on all the sub interface classes (Buttons, Textareas, windows a.s.on)
 * 			class Graph::Pic
 * 			func	Graph::draw_pic
 * 			Initalized g_gr object
 */

/** User_Interface::User_Interface(void)
 *
 * Def Constructor
 *
 * Args: none
 * Returns: Nothing
 */
User_Interface::User_Interface(void)
{
}

/** User_Interface::~User_Interface(void)
 *
 * Def Destructor
 *
 * Args: none
 * Returns: Nothing
 */
User_Interface::~User_Interface(void)
{
}

