/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#include <cassert>
#include "graphic.h"
#include "rendertarget.h"
#include "system.h"
#include "types.h"
#include "ui_panel.h"

#ifdef DEBUG
void yield_double_game ();
#endif

/*
==============================================================================

Panel

==============================================================================
*/

UIPanel *UIPanel::_modal = 0;
UIPanel *UIPanel::_g_mousegrab = 0;
UIPanel *UIPanel::_g_mousein = 0;
uint UIPanel::s_default_cursor = 0;


/**
 * Initialize a panel, link it into the parent's queue.
 */
UIPanel::UIPanel(UIPanel *nparent, const int nx, const int ny, const uint nw, const uint nh)
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

/**
 * Unlink the panel from the parent's queue
 */
UIPanel::~UIPanel()
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
	free_children();

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


/**
Free all of the panel's children.
*/
void UIPanel::free_children()
{
	while(_fchild)
		delete _fchild;
}


/**
Enters the event loop; all events will be handled by this panel.
Returns the return code passed to end_modal
*/
int UIPanel::run()
{
	UIPanel *prevmodal = _modal;
	_modal = this;
	_g_mousegrab = 0; // good ol' paranoia
	Sys_MouseLock(false); // more paranoia :-)

	UIPanel *forefather = this;
	while(forefather->_parent)
		forefather = forefather->_parent;

	s_default_cursor = g_gr->get_picture(PicMod_UI, "pics/cursor.png", true);

	// Loop
	_running = true;
	start();
	g_gr->update_fullscreen();
	while(_running)
	{
		static InputCallback icb = {
			UIPanel::ui_mouseclick,
			UIPanel::ui_mousemove,
			UIPanel::ui_key
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

#ifdef DEBUG
		yield_double_game ();
#endif
	}
	g_gr->update_fullscreen();
	end();

	// Done
	_modal = prevmodal;

	return _retcode;
}

/**
 * Cause run() to return as soon as possible, with the given return code
 */
void UIPanel::end_modal(int code)
{
	_running = false;
	_retcode = code;
}

/**
 * Called once before the event loop in run is started
 */
void UIPanel::start()
{
}

/**
 * Called once after the event loop in run() has ended
 */
void UIPanel::end()
{
}

/**
 * Resizes the panel.
 */
void UIPanel::set_size(const uint nw, const uint nh)
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

/**
 * Move the panel. UIPanel's position is relative to the parent.
 */
void UIPanel::set_pos(const int nx, const int ny)
{
	bool nd = _needdraw;
	update(0, 0, _w, _h);
	_x = nx;
	_y = ny;
	update(0, 0, _w, _h);
	_needdraw = nd;
}

/**
 * Set the size of the inner area (total area minus border)
 */
void UIPanel::set_inner_size(uint nw, uint nh)
{
	set_size(nw+_lborder+_rborder, nh+_tborder+_bborder);
}


/**
 * Resize so that we match the size of the inner panel.
 */
void UIPanel::fit_inner(UIPanel* inner)
{
	set_inner_size(inner->get_w(), inner->get_h());
	inner->set_pos(0, 0);
}


/**
 * Change the border dimensions.
 * Note that since position and total size aren't changed, so that the size
 * and position of the inner area will change.
 */
void UIPanel::set_border(uint l, uint r, uint t, uint b)
{
	_lborder = l;
	_rborder = r;
	_tborder = t;
	_bborder = b;
	update(0, 0, get_w(), get_h());
}

/**
 * Make this panel the top-most panel in the parent's Z-order.
 */
void UIPanel::move_to_top()
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

/**
 * Makes the panel visible or invisible
 */
void UIPanel::set_visible(bool on)
{

   _flags &= ~pf_visible;
	if (on)
		_flags |= pf_visible;

	update(0, 0, _w, _h);
}

/**
 * [virtual]
 * Redraw the panel. Note that all drawing coordinates are relative to the
 * inner area: you cannot overwrite the panel border in this function.
 */
void UIPanel::draw(RenderTarget* dst)
{
}

/**
 * [virtual]
 * Redraw the panel border.
 */
void UIPanel::draw_border(RenderTarget* dst)
{
}


/*
===============
Panel::draw_overlay [virtual]

Draw overlays that appear over all child panels.
This can be used e.g. for debug information.
===============
*/
void UIPanel::draw_overlay(RenderTarget* dst)
{
}


/**
 * Mark a part of a panel for updating.
 */
void UIPanel::update(int x, int y, int w, int h)
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

/**
 * Mark a part of a panel for updating.
 */
void UIPanel::update_inner(int x, int y, int w, int h)
{
	update(x-_lborder, y-_tborder, w, h);
}

/**
 * Enable/Disable the drawing cache.
 * When the drawing cache is enabled, draw() is only called after an update()
 * has been called explicitly. Otherwise, the contents of the panel are copied
 * from a cached Pic.
 */
void UIPanel::set_cache(bool enable)
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

/**
 * Called once per event loop pass, unless set_think(false) has
 * been called. It is intended to be used for animations and game logic.
 * The default implementation calls the children's think function.
 */
void UIPanel::think()
{
	for(UIPanel *child = _fchild; child; child = child->_next) {
		if (child->get_think())
			child->think();
	}
}


/**
 * Get mouse position relative to this panel
*/
int UIPanel::get_mouse_x()
{
	if (!_parent)
		return Sys_GetMouseX() - get_x()-get_lborder();
	else
		return _parent->get_mouse_x() - get_x()-get_lborder();
}

int UIPanel::get_mouse_y()
{
	if (!_parent)
		return Sys_GetMouseY() - get_y()-get_tborder();
	else
		return _parent->get_mouse_y() - get_y()-get_tborder();
}


/**
 * Set mouse position relative to this panel
*/
void UIPanel::set_mouse_pos(int x, int y)
{
	if (!_parent)
		Sys_SetMousePos(x + get_x()+get_lborder(), y + get_y()+get_tborder());
	else
		_parent->set_mouse_pos(x + get_x()+get_lborder(), y + get_y()+get_tborder());
}


/*
 * Center the mouse on this panel.
*/
void UIPanel::center_mouse()
{
	set_mouse_pos(get_w() / 2, get_h() / 2);
}


/**
 * Called whenever the mouse enters or leaves the panel. The inside state
 * is relative to the outer area of a panel. This means that the mouse
 * position received in handle_mousemove may be negative while the mouse is
 * still inside the panel as far as handle_mousein is concerned.
 */
void UIPanel::handle_mousein(bool inside)
{
}

/**
 * Called whenever the user clicks into the panel.
 * If the panel doesn't process the mouse-click, it is handed to the panel's
 * parent.
 *
 * Returns: true if the mouseclick was processed
 */
bool UIPanel::handle_mouseclick(uint btn, bool down, int x, int y)
{
	return false;
}

/**
 * Called when the mouse is moved while inside the panel
 *
 */
void UIPanel::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
{
}

/**
 * Receive a keypress or keyrelease event.
 * code is one of the KEY_xxx constants, c is the corresponding printable
 * character or 0 for special, unprintable keys.
 *
 * Return true if you processed the key.
*/
bool UIPanel::handle_key(bool down, int code, char c)
{
	return false;
}

/**
 *
 * Enable/Disable mouse handling by this panel
 * Default is enabled. Note that when mouse handling is disabled, child panels
 * don't receive mouse events either.
 *
 * Args: yes	true if the panel should receive mouse events
 */
void UIPanel::set_handle_mouse(bool yes)
{
	if (yes)
		_flags |= pf_handle_mouse;
	else
		_flags &= ~pf_handle_mouse;
}

/**
 *
 * Enable/Disable mouse grabbing. If a panel grabs the mouse, all mouse
 * related events will be sent directly to that panel.
 * You should only grab the mouse as a response to a mouse event (e.g.
 * clicking a mouse button)
 */
void UIPanel::grab_mouse(bool grab)
{
	if (grab) {
		_g_mousegrab = this;
	} else {
		assert(!_g_mousegrab || _g_mousegrab == this);
		_g_mousegrab = 0;
	}
}

/**
 * Set if this panel can receive the keyboard focus
*/
void UIPanel::set_can_focus(bool yes)
{

	if (yes) {
		_flags |= pf_can_focus;
   }	else {
		_flags &= ~pf_can_focus;

		if (_parent && _parent->_focus == this)
			_parent->_focus = 0;
	}
}

/**
 * Grab the keyboard focus, if it can
 */
void UIPanel::focus()
{

   // this assert was deleted, because
   // it happens, that a child can focus, but a parent
   // can't. but focus is called recursivly
   // assert(get_can_focus());

	if (!_parent || this == _modal)
		return;
	if (_parent->_focus == this)
		return;

	_parent->_focus = this;
	_parent->focus();
}

/**
 * Enables/Disables calling think() during the event loop.
 * The default is enabled.
 *
 * Args: yes	true if the panel's think function should be called
 */
void UIPanel::set_think(bool yes)
{
	if (yes)
		_flags |= pf_think;
	else
		_flags &= ~pf_think;
}

/**
 * Cause this panel to be removed on the next frame.
 * Use this for a panel that needs to destroy itself after a button has
 * been pressed (e.g. non-modal dialogs).
 * Do NOT use this to delete a hierarchy of panels that have been modal.
 */
void UIPanel::die()
{
   _flags |= pf_die;

	for(UIPanel *p = _parent; p; p = p->_parent) {
		p->_flags |= pf_child_die;
		if (p == _modal)
			break;
	}
}

/** [private]
 *
 * Recursively walk the panel tree, killing panels that are marked for death
 * using die().
 */
void UIPanel::check_child_death()
{
	UIPanel *next = _fchild;
	while(next) {
		UIPanel *p = next;
		next = p->_next;

		if (p->_flags & pf_die)
			delete p;
		else if (p->_flags & pf_child_die)
			p->check_child_death();
	}

   _flags &= ~pf_child_die;
}

/**
 * [private]
 * dst is the RenderTarget for the parent UIPanel.
 * Subset for the border first and draw the border, then subset for the inner area
 * and draw the inner area.
 * Draw child panels after drawing self.
*/
void UIPanel::do_draw(RenderTarget* dst)
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
				for(UIPanel *child = _lchild; child; child = child->_prev)
					child->do_draw(dst);

				draw_overlay(dst);
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

				for(UIPanel *child = _lchild; child; child = child->_prev)
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


/**
 * Return the panel that receives mouse clicks at the given location
 * Returns: topmost panel at the given coordinates
 */
UIPanel *UIPanel::get_mousein(int x, int y)
{
	UIPanel *child;

	for(child = _fchild; child; child = child->_next) {
		if (!child->get_handle_mouse() || !child->get_visible())
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

/**
 * Propagate mouseleave events (e.g. for buttons that are inside a different
 * window)
 */
void UIPanel::do_mousein(bool inside)
{
	if (!inside && _mousein) {
		_mousein->do_mousein(false);
		_mousein = false;
	}
	handle_mousein(inside);
}

/**
 * Propagate mouse clicks to the appropriate panel.
 *
 * Returns: true, if the click was processed
 */
bool UIPanel::do_mouseclick(uint btn, bool down, int x, int y)
{
	x -= _lborder;
	y -= _tborder;

	if (down && (_flags & pf_top_on_click))
		move_to_top();

	if (_g_mousegrab == this)
		return handle_mouseclick(btn, down, x, y);
	else
	{
		UIPanel *child = get_mousein(x, y);

		if (child) {
			if (child->do_mouseclick(btn, down, x-child->_x, y-child->_y))
				return true;
		}

		return handle_mouseclick(btn, down, x, y);
	}
}

/**
 * Propagate mouse movement to the appropriate panel.
 */
void UIPanel::do_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
{
	x -= _lborder;
	y -= _rborder;

	if (_g_mousegrab == this)
		handle_mousemove(x, y, xdiff, ydiff, btns);
	else
	{
		UIPanel *child = get_mousein(x, y);

		if (child)
			child->do_mousemove(x-child->_x, y-child->_y, xdiff, ydiff, btns);
		else
			handle_mousemove(x, y, xdiff, ydiff, btns);
	}
}

/**
 * Pass the key event to the focussed child.
 * If it doesn't process the key, we'll see if we can use the event.
*/
bool UIPanel::do_key(bool down, int code, char c)
{
	if (_focus) {
		if (_focus->do_key(down, code, c))
			return true;
	}

	return handle_key(down, code, c);
}

/**
 *
 * Determine which panel is to receive a mouse event.
 *
 * Returns: the panel which receives the mouse event
 */
UIPanel *UIPanel::ui_trackmouse(int *x, int *y)
{
	UIPanel *mousein;
	UIPanel *rcv = 0;

	if (_g_mousegrab)
		mousein = rcv = _g_mousegrab;
	else
		mousein = _modal;

	*x -= mousein->_x;
	*y -= mousein->_y;
	for(UIPanel *p = mousein->_parent; p; p = p->_parent) {
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

/**
 * [static]
 * Input callback function. Pass the mouseclick event to the currently modal
 * panel.
*/
void UIPanel::ui_mouseclick(bool down, int button, uint btns, int x, int y)
{
	UIPanel *p;

	p = ui_trackmouse(&x, &y);
	if (!p)
		return;

	p->do_mouseclick(button, down, x, y);
}

/**
 * [static]
 *
 * Input callback function. Pass the mousemove event to the currently modal
 * panel.
*/
void UIPanel::ui_mousemove(uint btns, int x, int y, int xdiff, int ydiff)
{
	if (!xdiff && !ydiff)
		return;

	UIPanel *p;
	int w, h;

	g_gr->get_picture_size(s_default_cursor, &w, &h);

	g_gr->update_rectangle(x-xdiff, y-ydiff, w, h);
	g_gr->update_rectangle(x, y, w, h);

	p = ui_trackmouse(&x, &y);
	if (!p)
		return;

	p->do_mousemove(x, y, xdiff, ydiff, btns);
}

/**
 * [static]
 *
 * Input callback function. Pass the key event to the currently modal panel
 */
void UIPanel::ui_key(bool down, int code, char c)
{
	_modal->do_key(down, code, c);
}
