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
#include "font.h"

/*
==============================================================================

Panel

==============================================================================
*/

Panel *Panel::_modal = 0;
Panel *Panel::_g_mousegrab = 0;
Panel *Panel::_g_mousein = 0;
uint Panel::s_default_cursor = 0;


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
	_focus = 0;

	_x = nx;
	_y = ny;
	_w = nw;
	_h = nh;

	_lborder = _rborder = _tborder = _bborder = 0;

	_cache = 0;
	_needdraw = false;

	_flags = pf_handle_mouse|pf_think|pf_visible;
	update(0, 0, _w, _h);
}

/** Panel::~Panel()
 *
 * Unlink the panel from the parent's queue
 */
Panel::~Panel()
{
	update(0, 0, get_w(), get_h());

	if (_cache)
		g_gr->free_surface(_cache);

	// Release pointers to this object
	if (_g_mousegrab == this)
		_g_mousegrab = 0;
	if (_g_mousein == this)
		_g_mousein = 0;

	// Free children
	while(_fchild)
		delete _fchild;

	// Unlink
	if (_parent) {
		if (_parent->_mousein == this)
			_parent->_mousein = 0;
		if (_parent->_focus == this)
			_parent->_focus = 0;

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

/*
===============
Panel::run

Enters the event loop; all events will be handled by this panel.

Returns the return code passed to end_modal
===============
*/
int Panel::run()
{
	Panel *prevmodal = _modal;
	_modal = this;
	_g_mousegrab = 0; // good ol' paranoia
	Sys_MouseLock(false); // more paranoia :-)

	Panel *forefather = this;
	while(forefather->_parent)
		forefather = forefather->_parent;

	s_default_cursor = g_gr->get_picture(PicMod_UI, "pics/cursor.bmp", RGBColor(0,0,255));
	
	// Loop
	_running = true;
	start();
	g_gr->update_fullscreen();
	while(_running)
	{
		static InputCallback icb = {
			Panel::ui_mouseclick,
			Panel::ui_mousemove,
			Panel::ui_key
		};
	
		Sys_HandleInput(&icb);
		if (Sys_ShouldDie())
			end_modal(-1);

		if (_flags & pf_think)
			think();

		if (g_gr->need_update()) {
			RenderTarget* rt = g_gr->get_render_target();
			
			forefather->do_draw(rt);
			
			rt->blit(Sys_GetMouseX(), Sys_GetMouseY(), s_default_cursor);
			
			g_gr->refresh();
		}

		if (_flags & pf_child_die)
			check_child_death();
	}
	g_gr->update_fullscreen();
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

	if (_cache) {
		g_gr->free_surface(_cache);
		_cache = g_gr->create_surface(_w, _h);
	}

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
	bool nd = _needdraw;
	update(0, 0, _w, _h);
	_x = nx;
	_y = ny;
	update(0, 0, _w, _h);
	_needdraw = nd;
}

/** Panel::set_inner_size(uint nw, uint nh)
 *
 * Set the size of the inner area (total area minus border)
 *
 * Args: nw	new dimensions of the inner area
 */
void Panel::set_inner_size(uint nw, uint nh)
{
	set_size(nw+_lborder+_rborder, nh+_tborder+_bborder);
}

/** Panel::set_border(uint l, uint r, uint t, uint b)
 *
 * Change the border dimensions.
 * Note that since position and total size aren't changed, so that the size
 * and position of the inner area will change.
 *
 * Args: l	size of left border, in pixels
 *       r	size of right border, in pixels
 *       t	size of top border, in pixels
 *       b	size of bottom border, in pixels
 */
void Panel::set_border(uint l, uint r, uint t, uint b)
{
	_lborder = l;
	_rborder = r;
	_tborder = t;
	_bborder = b;
	update(0, 0, get_w(), get_h());
}

/** Panel::move_to_top()
 *
 * Make this panel the top-most panel in the parent's Z-order.
 */
void Panel::move_to_top()
{
	if (!_parent)
		return;

	// unlink
	if (_prev)
		_prev->_next = _next;
	else
		_parent->_fchild = _next;
	if (_next)
		_next->_prev = _prev;
	else
		_parent->_lchild = _prev;

	// relink
	_prev = 0;
	_next = _parent->_fchild;
	_parent->_fchild = this;
	if (_next)
		_next->_prev = this;
	else
		_parent->_lchild = this;
}

/** Panel::set_visible(bool on)
 *
 * Makes the panel visible or invisible
 */
void Panel::set_visible(bool on)
{
	_flags &= ~pf_visible;
	if (on)
		_flags |= pf_visible;
		
	update(0, 0, _w, _h);
}

/*
===============
Panel::draw [virtual]

Redraw the panel. Note that all drawing coordinates are relative to the
inner area: you cannot overwrite the panel border in this function.
===============
*/
void Panel::draw(RenderTarget* dst)
{
}

/*
===============
Panel::draw_border [virtual]

Redraw the panel border.
===============
*/
void Panel::draw_border(RenderTarget* dst)
{
}

/** Panel::update(int x, int y, int w, int h);
 *
 * Mark a part of a panel for updating.
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

	_needdraw = true;

	if (_parent) {
		_parent->update_inner(x+_x, y+_y, w, h);
	} else {
		if (x < 0) {
			w += x;
			x = 0;
		}
		if (x+w > g_gr->get_xres())
			w = g_gr->get_xres() - x;
		if (w <= 0)
			return;

		if (y < 0) {
			h += y;
			y = 0;
		}
		if (y+h > g_gr->get_yres())
			h = g_gr->get_yres() - y;
		if (h <= 0)
			return;

		g_gr->update_rectangle(x, y, w, h);
	}
}

/** Panel::update_inner(int x, int y, int w, int h)
 *
 * Mark a part of a panel for updating.
 *
 * Args: x	coordinates of the rectangle, relative to the inner rectangle
 *       y
 *       w	size of the rectangle
 *       h
 */
void Panel::update_inner(int x, int y, int w, int h)
{
	update(x-_lborder, y-_tborder, w, h);
}

/** Panel::set_cache(bool enable)
 *
 * Enable/Disable the drawing cache.
 * When the drawing cache is enabled, draw() is only called after an update()
 * has been called explicitly. Otherwise, the contents of the panel are copied
 * from a cached Pic.
 */
void Panel::set_cache(bool enable)
{
/* Deactivated for now
	if (enable)
	{
		if (_cache)
			return;
		_cache = new Pic;
		_cache->set_size(_w, _h);
		_needdraw = true;
	}
	else
	{
		if (_cache) {
			delete _cache;
			_cache = 0;
		}
	}
*/
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


/*
===============
Panel::get_mouse_x
Panel::get_mouse_y

Get mouse position relative to this panel
===============
*/
int Panel::get_mouse_x()
{
	if (!_parent)
		return Sys_GetMouseX() - get_x()-get_lborder();
	else
		return _parent->get_mouse_x() - get_x()-get_lborder();
}

int Panel::get_mouse_y()
{
	if (!_parent)
		return Sys_GetMouseY() - get_y()-get_tborder();
	else
		return _parent->get_mouse_y() - get_y()-get_tborder();
}


/*
===============
Panel::set_mouse_pos

Set mouse position relative to this panel
===============
*/
void Panel::set_mouse_pos(int x, int y)
{
	if (!_parent)
		Sys_SetMousePos(x + get_x()+get_lborder(), y + get_y()+get_tborder());
	else
		_parent->set_mouse_pos(x + get_x()+get_lborder(), y + get_y()+get_tborder());
}


	
/** Panel::handle_mousein(uint x, uint y, bool inside)
 *
 * Called whenever the mouse enters or leaves the panel. The inside state
 * is relative to the outer area of a panel. This means that the mouse
 * position received in handle_mousemove may be negative while the mouse is
 * still inside the panel as far as handle_mousein is concerned.
 *
 * Args: inside	true if the cursor has entered the panel
 */
void Panel::handle_mousein(bool inside)
{
}

/** Panel::handle_mouseclick(uint btn, bool down, int x, int y)
 *
 * Called whenever the user clicks into the panel.
 * If the panel doesn't process the mouse-click, it is handed to the panel's
 * parent.
 *
 * Args: btn	0 = left, 1 = right
 *       down	true if the button was pressed, false if released
 *       x		mouse coordinates relative to the inner rectangle
 *       y
 *
 * Returns: true if the mouseclick was processed
 */
bool Panel::handle_mouseclick(uint btn, bool down, int x, int y)
{
	return false;
}

/** Panel::handle_mousemove(int x, int y, uint btns)
 *
 * Called when the mouse is moved while inside the panel
 *
 * Args: x		mouse coordinates relative to the inner rectangle
 *       y
 *       xdiff	relative mouse movement
 *       ydiff
 *       btns	bitmask of currently pressed buttons: (1<<0) left, (1<<1) right
 */
void Panel::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
{
}

/*
===============
Panel::handle_key

Receive a keypress or keyrelease event.
code is one of the KEY_xxx constants, c is the corresponding printable
character or 0 for special, unprintable keys.

Return true if you processed the key.
===============
*/
bool Panel::handle_key(bool down, int code, char c)
{
	return false;
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

/** Panel::grab_mouse(bool grab)
 *
 * Enable/Disable mouse grabbing. If a panel grabs the mouse, all mouse
 * related events will be sent directly to that panel.
 * You should only grab the mouse as a response to a mouse event (e.g.
 * clicking a mouse button)
 *
 * Args: grab	true if grabbing is to be enabled
 */
void Panel::grab_mouse(bool grab)
{
	if (grab) {
		_g_mousegrab = this;
	} else {
		assert(!_g_mousegrab || _g_mousegrab == this);
		_g_mousegrab = 0;
	}
}

/*
===============
Panel::set_can_focus
===============
*/
void Panel::set_can_focus(bool yes)
{
	if (yes)
		_flags |= pf_can_focus;
	else {
		_flags &= ~pf_can_focus;
		
		if (_parent && _parent->_focus == this)
			_parent->_focus = 0;
	}
}

/*
===============
Panel::focus

Grab the keyboard focus
===============
*/
void Panel::focus()
{
	assert(get_can_focus());

	if (!_parent || this == _modal)
		return;
	if (_parent->_focus == this)
		return;

	_parent->_focus = this;
	_parent->focus();
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

/** Panel::die()
 *
 * Cause this panel to be removed on the next frame.
 * Use this for a panel that needs to destroy itself after a button has
 * been pressed (e.g. non-modal dialogs).
 * Do NOT use this to delete a hierarchy of panels that have been modal.
 */
void Panel::die()
{
	_flags |= pf_die;

	for(Panel *p = _parent; p; p = p->_parent) {
		p->_flags |= pf_child_die;
		if (p == _modal)
			break;
	}
}

/** Panel::check_child_death() [private]
 *
 * Recursively walk the panel tree, killing panels that are marked for death
 * using die().
 */
void Panel::check_child_death()
{
	Panel *next = _fchild;
	while(next) {
		Panel *p = next;
		next = p->_next;

		if (p->_flags & pf_die)
			delete p;
		else if (p->_flags & pf_child_die)
			p->check_child_death();
	}

	_flags &= ~pf_child_die;
}

/*
===============
Panel::do_draw [private]

dst is the RenderTarget for the parent Panel.
Subset for the border first and draw the border, then subset for the inner area
and draw the inner area.
Draw child panels after drawing self.
===============
*/
void Panel::do_draw(RenderTarget* dst)
{
	if (!get_visible())
		return;
	
	if (!_cache)
	{
		Rect outerrc;
		Point outerofs;
		
		if (dst->enter_window(Rect(_x, _y, _w, _h), &outerrc, &outerofs)) {
			draw_border(dst);
			
			Rect innerwindow(_lborder, _tborder, _w-(_lborder+_rborder), _h-(_tborder+_bborder));
			
			if (dst->enter_window(innerwindow, 0, 0)) {
				draw(dst);

				// draw back to front
				for(Panel *child = _lchild; child; child = child->_prev)
					child->do_draw(dst);
			}
			
			dst->set_window(outerrc, outerofs);
		}
	}
	/*
	else
	{
		// redraw only if explicitly requested
		if (_needdraw) {
			draw_border(_cache);

			RenderTarget* inner = _cache->enter_window(_lborder, _tborder, 
			       _w-(_lborder+_rborder), _h-(_tborder+_bborder));
			
			if (inner) {
				draw(inner);

				for(Panel *child = _lchild; child; child = child->_prev)
					child->do_draw(inner);
				
				inner->leave_window();
			}

			_needdraw = false;
		}

		// now just blit from the cache
		dst->blit(_x, _y, _cache);
	}
	*/
}


/** Panel::get_mousein(int x, int y)
 *
 * Return the panel that receives mouse clicks at the given location
 *
 * Args: x	mouse coordinates relative to panel inner rectangle
 *       y
 *
 * Returns: topmost panel at the given coordinates
 */
Panel *Panel::get_mousein(int x, int y)
{
	Panel *child;

	for(child = _fchild; child; child = child->_next) {
		if (!child->get_handle_mouse())
			continue;
		if (x < child->_x+(int)child->_w && x >= child->_x &&
		    y < child->_y+(int)child->_h && y >= child->_y)
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

/** Panel::do_mouseclick(uint btn, bool down, int x, int y)
 *
 * Propagate mouse clicks to the appropriate panel.
 *
 * Args: btn	button number (0 = left, 1 = right)
 *       down	true if button was pressed
 *       x		mouse coordinates relative to panel
 *       y
 *
 * Returns: true, if the click was processed
 */
bool Panel::do_mouseclick(uint btn, bool down, int x, int y)
{
	x -= _lborder;
	y -= _tborder;

	if (down && (_flags & pf_top_on_click))
		move_to_top();

	if (_g_mousegrab == this)
		return handle_mouseclick(btn, down, x, y);
	else
	{
		Panel *child = get_mousein(x, y);

		if (child) {
			if (child->do_mouseclick(btn, down, x-child->_x, y-child->_y))
				return true;
		}

		return handle_mouseclick(btn, down, x, y);
	}
}

/** Panel::do_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
 *
 * Propagate mouse movement to the appropriate panel.
 *
 * Args: x		mouse coordinates relative to panel
 *       y
 *       xdiff	relative mouse movement
 *       ydiff
 *       btns	bitmask of pressed buttons
 */
void Panel::do_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
{
	x -= _lborder;
	y -= _rborder;

	if (_g_mousegrab == this)
		handle_mousemove(x, y, xdiff, ydiff, btns);
	else
	{
		Panel *child = get_mousein(x, y);

		if (child)
			child->do_mousemove(x-child->_x, y-child->_y, xdiff, ydiff, btns);
		else
			handle_mousemove(x, y, xdiff, ydiff, btns);
	}
}

/*
===============
Panel::do_key

Pass the key event to the focussed child.
If it doesn't process the key, we'll see if we can use the event.
===============
*/
bool Panel::do_key(bool down, int code, char c)
{
	if (_focus) {
		if (_focus->do_key(down, code, c))
			return true;
	}
	
	return handle_key(down, code, c);
}

/** Panel::ui_trackmouse(int *x, int *y) [static]
 *
 * Determine which panel is to receive a mouse event.
 *
 * Args: x	mouse coordinates, relative to the screen
 *       y	converted to coordinates local to the panel
 *
 * Returns: the panel which receives the mouse event
 */
Panel *Panel::ui_trackmouse(int *x, int *y)
{
	Panel *mousein;
	Panel *rcv = 0;

	if (_g_mousegrab)
		mousein = rcv = _g_mousegrab;
	else
		mousein = _modal;

	*x -= mousein->_x;
	*y -= mousein->_y;
	for(Panel *p = mousein->_parent; p; p = p->_parent) {
		*x -= p->_lborder + p->_x;
		*y -= p->_tborder + p->_y;
	}

	if (*x >= 0 && *x < (int)mousein->_w &&
	    *y >= 0 && *y < (int)mousein->_h)
		rcv = mousein;
	else
		mousein = 0;

	if (mousein != _g_mousein) {
		if (_g_mousein)
			_g_mousein->do_mousein(false);
		_g_mousein = mousein;
		if (_g_mousein)
			_g_mousein->do_mousein(true);
	}

	return rcv;
}

/*
===============
Panel::ui_mouseclick [static]
 
Input callback function. Pass the mouseclick event to the currently modal
panel.
===============
*/
void Panel::ui_mouseclick(bool down, int button, uint btns, int x, int y)
{
	Panel *p;

	p = ui_trackmouse(&x, &y);
	if (!p)
		return;

	p->do_mouseclick(button, down, x, y);
}

/*
===============
Panel::ui_mousemove [static]

Input callback function. Pass the mousemove event to the currently modal
panel.
===============
*/
void Panel::ui_mousemove(uint btns, int x, int y, int xdiff, int ydiff)
{
	if (!xdiff && !ydiff)
		return;

	Panel *p;
	int w, h;
	
	g_gr->get_picture_size(s_default_cursor, &w, &h);
	
	g_gr->update_rectangle(x-xdiff, y-ydiff, w, h);
	g_gr->update_rectangle(x, y, w, h);

	p = ui_trackmouse(&x, &y);
	if (!p)
		return;

	p->do_mousemove(x, y, xdiff, ydiff, btns);
}

/*
===============
Panel::ui_key [static]

Input callback function. Pass the key event to the currently modal panel
===============
*/
void Panel::ui_key(bool down, int code, char c)
{
	_modal->do_key(down, code, c);
}
