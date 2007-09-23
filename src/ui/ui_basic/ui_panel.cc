/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "ui_panel.h"

#include "constants.h"
#include "font_handler.h"
#include "graphic.h"
#include "rendertarget.h"
#include "sound/sound_handler.h"
#include "wlapplication.h"

namespace UI {

Panel *Panel::_modal = 0;
Panel *Panel::_g_mousegrab = 0;
Panel *Panel::_g_mousein = 0;
uint32_t Panel::s_default_cursor = 0;


/**
 * Initialize a panel, link it into the parent's queue.
 */
Panel::Panel
	(Panel * const nparent,
	 const int nx, const int ny, const uint32_t nw, const uint32_t nh,
	 const std::string & tooltip_text)
	:
_parent(nparent), _mousein(0), _focus(0),
_flags(pf_handle_mouse|pf_think|pf_visible), _cache(0), _needdraw(false),
_x(nx), _y(ny), _w(nw), _h(nh),
_lborder(0), _rborder(0), _tborder(0), _bborder(0),
_border_snap_distance(0), _panel_snap_distance(0),
	_tooltip(tooltip_text.size() ? strdup(tooltip_text.c_str()) : 0)
{
	assert(nparent != this);

	if (_parent)
		_parent->add_child(this);

	update(0, 0, _w, _h);
}

/**
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
	free_children();

	// Unlink
	if (_parent) {
		if (_parent->_mousein == this)
			_parent->_mousein = 0;
		if (_parent->_focus == this)
			_parent->_focus = 0;

		_parent->remove_child(this);
	}

	free(_tooltip);
}

void Panel::add_child(Panel * child)
{
	m_children.push_front(child);
}

void Panel::remove_child(Panel * child)
{
	m_children.remove(child);
}

/**
 * Free all of the panel's children.
 */
void Panel::free_children() {
 	while (!m_children.empty())
 		delete *(m_children.begin());
}


/**
 * Enters the event loop; all events will be handled by this panel.
 * \return the return code passed to end_modal. This return code
 * will be negative when the event loop was quit in an abnormal way
 * (e.g. the user clicked the window's close button or similar).
 */
int Panel::run()
{
	WLApplication *app=WLApplication::get();
	Panel *prevmodal = _modal;
	_modal = this;
	_g_mousegrab = 0; // good ol' paranoia
	app->set_mouse_lock(false); // more paranoia :-)

	Panel *forefather = this;
	while (forefather->_parent)
		forefather = forefather->_parent;

	s_default_cursor = g_gr->get_picture(PicMod_UI,  "pics/cursor.png");

	// Loop
	_running = true;

	// Panel-specific startup code. Note that this might call end_modal()!
	start();
	g_gr->update_fullscreen();
	while (_running)
	{
		static InputCallback icb = {
			Panel::ui_mousepress,
			Panel::ui_mouserelease,
			Panel::ui_mousemove,
			Panel::ui_key
		};

		app->handle_input(&icb);
		if (app->should_die()) end_modal(dying_code);

		if (_flags & pf_think)
			think();

		if (g_gr->need_update()) {
			RenderTarget* rt = g_gr->get_render_target();

			forefather->do_draw(rt);

			rt->blit(app->get_mouse_position() - Point(3, 7), s_default_cursor);

			if (Panel *lowest = _mousein)
			{
				while (lowest->_mousein)
					lowest = lowest->_mousein;
				if (lowest->tooltip()) draw_tooltip(rt, lowest);
			}

			g_gr->refresh();
		}

#ifdef DEBUG
#ifndef __WIN32__
		WLApplication::yield_double_game ();
#endif
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
void Panel::end_modal(int code)
{
	_running = false;
	_retcode = code;
}

/**
 * Called once before the event loop in run is started
 */
void Panel::start()
{
}

/**
 * Called once after the event loop in run() has ended
 */
void Panel::end()
{
}

/**
 * Resizes the panel.
 */
void Panel::set_size(const uint32_t nw, const uint32_t nh)
{
	uint32_t upw = _w;
	uint32_t uph = _h;
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
 * Move the panel. Panel's position is relative to the parent.
 */
void Panel::set_pos(const Point n) {
	bool nd = _needdraw;
	update(0, 0, _w, _h);
	_x = n.x;
	_y = n.y;
	update(0, 0, _w, _h);
	_needdraw = nd;
}

/**
Do nothing
*/
void Panel::move_inside_parent() {}

/**
 * Set the size of the inner area (total area minus border)
 */
void Panel::set_inner_size(uint32_t nw, uint32_t nh)
{
	set_size(nw+_lborder+_rborder, nh+_tborder+_bborder);
}


/**
 * Resize so that we match the size of the inner panel.
 */
void Panel::fit_inner(Panel* inner)
{
	set_inner_size(inner->get_w(), inner->get_h());
	inner->set_pos(Point(0, 0));
}


/**
 * Change the border dimensions.
 * Note that since position and total size aren't changed, so that the size
 * and position of the inner area will change.
 */
void Panel::set_border(uint32_t l, uint32_t r, uint32_t t, uint32_t b)
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
void Panel::move_to_top()
{
	if (!_parent)
		return;

	//readd ourselves at the top of stack
	_parent->remove_child(this);
	_parent->add_child(this);
}

/**
 * Makes the panel visible or invisible
 */
void Panel::set_visible(bool on)
{

   _flags &= ~pf_visible;
	if (on)
		_flags |= pf_visible;

	update(0, 0, _w, _h);
}

/**
 * Redraw the panel. Note that all drawing coordinates are relative to the
 * inner area: you cannot overwrite the panel border in this function.
 */
void Panel::draw(RenderTarget *) {}

/**
 * Redraw the panel border.
 */
void Panel::draw_border(RenderTarget *) {}


/**
 * Draw overlays that appear over all child panels.
 * This can be used e.g. for debug information.
*/
void Panel::draw_overlay(RenderTarget &) {}


/**
 * Mark a part of a panel for updating.
 */
void Panel::update(int x, int y, int w, int h)
{
	if
		(x >= static_cast<int>(_w) or x + w <= 0
		 or
		 y >= static_cast<int>(_h) or y + h <= 0)
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
void Panel::update_inner(int x, int y, int w, int h)
{
	update(x-_lborder, y-_tborder, w, h);
}

/**
 * Enable/Disable the drawing cache.
 * When the drawing cache is enabled, draw() is only called after an update()
 * has been called explicitly. Otherwise, the contents of the panel are copied
 * from a cached Pic.
 */
void Panel::set_cache(bool)
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
			delete _cache;
			_cache = 0;
	}
*/
}

/**
 * Called once per event loop pass, unless set_think(false) has
 * been called. It is intended to be used for animations and game logic.
 * The default implementation calls the children's think function.
 */
void Panel::think()
{
	panellist_it child;

	for (child=m_children.begin(); child!=m_children.end(); ++child) {
		if ((*child)->get_think())
			(*child)->think();
	}
}


/**
 * Get mouse position relative to this panel
*/
Point Panel::get_mouse_position() const throw () {
	return
		(_parent ?
		 _parent             ->get_mouse_position()
		 :
		 WLApplication::get()->get_mouse_position())
		-
		Point(get_x() + get_lborder(), get_y() + get_tborder());
}


/**
 * Set mouse position relative to this panel
*/
void Panel::set_mouse_pos(const Point p) {
	const Point relative_p =
		p + Point(get_x() + get_lborder(), get_y() + get_tborder());
	if (_parent) _parent     ->set_mouse_pos(relative_p);
	else WLApplication::get()->set_mouse_pos(relative_p);
}


/*
 * Center the mouse on this panel.
*/
void Panel::center_mouse() {set_mouse_pos(Point(get_w() / 2, get_h() / 2));}


/**
 * Called whenever the mouse enters or leaves the panel. The inside state
 * is relative to the outer area of a panel. This means that the mouse
 * position received in handle_mousemove may be negative while the mouse is
 * still inside the panel as far as handle_mousein is concerned.
 */
void Panel::handle_mousein(bool) {}

/**
 * Called whenever the user presses or releases a mouse button in the panel.
 * If the panel doesn't process the mouse-click, it is handed to the panel's
 * parent.
 *
 * Returns: true if the mouseclick was processed
 */
bool Panel::handle_mousepress  (const Uint8, int, int) {return false;}
bool Panel::handle_mouserelease(const Uint8, int, int) {return false;}

/**
 * Called when the mouse is moved while inside the panel
 *
 */
bool Panel::handle_mousemove(const Uint8, int, int, int, int) {return _tooltip;}

/**
 * Receive a keypress or keyrelease event.
 * code is one of the KEY_xxx constants, c is the corresponding printable
 * character or 0 for special, unprintable keys.
 *
 * Return true if you processed the key.
*/
bool Panel::handle_key(bool, int, char) {return false;}

/**
 *
 * Enable/Disable mouse handling by this panel
 * Default is enabled. Note that when mouse handling is disabled, child panels
 * don't receive mouse events either.
 *
 * Args: yes  true if the panel should receive mouse events
 */
void Panel::set_handle_mouse(bool yes)
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
void Panel::grab_mouse(bool grab)
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
void Panel::set_can_focus(bool yes)
{

	if (yes) _flags |= pf_can_focus;
	else {
		_flags &= ~pf_can_focus;

		if (_parent && _parent->_focus == this)
			_parent->_focus = 0;
	}
}

/**
 * Grab the keyboard focus, if it can
 */
void Panel::focus()
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
 * Args: yes  true if the panel's think function should be called
 */
void Panel::set_think(bool yes)
{
	if (yes)
		_flags |= pf_think;
	else
		_flags &= ~pf_think;
}

/**
 * Wrapper around Sound_Handler::play_fx() to prevent having to include
 * sound_handler.h in every UI subclass just for playing a 'click'
 */
void Panel::play_click()
{
	g_sound_handler.play_fx("click", 128, PRIO_ALWAYS_PLAY);
}

/**
 * dst is the RenderTarget for the parent Panel.
 * Subset for the border first and draw the border, then subset for the inner area
 * and draw the inner area.
 * Draw child panels after drawing self.
 * Draw tooltip if required.
*/
void Panel::do_draw(RenderTarget* dst)
{
	if (!get_visible())
		return;

	if (!_cache)
	{
		Rect outerrc;
		Point outerofs;

		if (dst->enter_window(Rect(Point(_x, _y), _w, _h), &outerrc, &outerofs)) {
			draw_border(dst);

			Rect innerwindow
				(Point(_lborder, _tborder),
				 _w - (_lborder + _rborder), _h - (_tborder + _bborder));

			if (dst->enter_window(innerwindow, 0, 0)) {
				draw(dst);

				// draw back to front
				for (panellist_crit i=m_children.rbegin(); i!=m_children.rend(); ++i)
					(*i)->do_draw(dst);

				draw_overlay(*dst);
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

				for (panellist_crit i=m_children.rbegin(); i!=m_children.rend(); ++i)
					(*i)->do_draw(inner);

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
 * \return The child panel that receives mouse events at the given location.
 */
inline Panel * Panel::child_at_mouse_cursor(int x, int y)
{
	for (panellist_it i = m_children.begin(); i!=m_children.end(); ++i) {
		if (!(*i)->get_handle_mouse() || !(*i)->get_visible())
			continue;

		if (x < (*i)->_x + static_cast<int>((*i)->_w) and x >= (*i)->_x
		    and
		    y < (*i)->_y + static_cast<int>((*i)->_h) and y >= (*i)->_y)
			return (*i);
	}

	return 0;
}

/**
 * Propagate mouseleave events (e.g. for buttons that are inside a different
 * window)
 */
void Panel::do_mousein(bool inside)
{
	if (!inside && _mousein) {
		_mousein->do_mousein(false);
		_mousein = false;
	}
	handle_mousein(inside);
}

/**
 * Propagate mousepresses to the appropriate panel.
 *
 * \return Whether the event was processed.
 */
bool Panel::do_mousepress(const Uint8 btn, int x, int y) {
	x -= _lborder;
	y -= _tborder;
	if (_flags & pf_top_on_click) move_to_top();
	if (_g_mousegrab != this) {
		Panel * child = child_at_mouse_cursor(x, y);
		if (child)
			if (child->do_mousepress(btn, x - child->_x, y - child->_y))
				return true;
	}
	return handle_mousepress(btn, x, y);
}

/**
 * Propagate mousereleases to the appropriate panel.
 *
 * \return Whether the event was processed.
 */
bool Panel::do_mouserelease(const Uint8 btn, int x, int y) {
	x -= _lborder;
	y -= _tborder;
	if (_g_mousegrab != this) {
		Panel * child = child_at_mouse_cursor(x, y);
		if (child)
			if (child->do_mouserelease(btn, x - child->_x, y - child->_y))
				return true;
	}
	return handle_mouserelease(btn, x, y);
}

/**
 * Propagate mousemoves to the appropriate panel.
 *
 * \return Whether the event was processed.
 */
bool Panel::do_mousemove(const Uint8 state, int x, int y, int xdiff, int ydiff)
{
	x -= _lborder;
	y -= _tborder;
	if (_g_mousegrab != this) {
		Panel * child = child_at_mouse_cursor(x, y);

		if (_mousein != child) { // is the mousecursor now over a different panel than before?
			if (_mousein) //if we were over a subpanel before, tell it the mouse has left
				_mousein->do_mousein(false);

			if (child) //if we're over a subpanel now, tell it the mouse has entered
				child->do_mousein(true);

			_mousein = child; //remember the new panel under the mouse
		}

		if (child) {
			if (child->do_mousemove(state, x - child->_x, y - child->_y, xdiff, ydiff))
 				return true;
		}
	}
	return handle_mousemove(state, x, y, xdiff, ydiff);
}

/**
 * Pass the key event to the focussed child.
 * If it doesn't process the key, we'll see if we can use the event.
*/
bool Panel::do_key(bool down, int code, char c)
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
	for (Panel *p = mousein->_parent; p; p = p->_parent) {
		*x -= p->_lborder + p->_x;
		*y -= p->_tborder + p->_y;
	}

	if
		(*x >= 0 and *x < static_cast<int>(mousein->_w)
		 and
		 *y >= 0 and *y < static_cast<int>(mousein->_h))
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
void Panel::ui_mousepress(const Uint8 button, int x, int y) {
	if (Panel * const p = ui_trackmouse(&x, &y))
		p->do_mousepress(button, x, y);
}
void Panel::ui_mouserelease(const Uint8 button, int x, int y) {
	if (Panel * const p = ui_trackmouse(&x, &y))
		p->do_mouserelease(button, x, y);
}

/**
 * [static]
 *
 * Input callback function. Pass the mousemove event to the currently modal
 * panel.
*/
void Panel::ui_mousemove(const Uint8 state, int x, int y, int xdiff, int ydiff) {
	if (!xdiff && !ydiff)
		return;

	Panel *p;
	uint32_t w, h;
	g_gr->get_picture_size(s_default_cursor, w, h);

	g_gr->update_rectangle(x-xdiff, y-ydiff, w, h);
	g_gr->update_rectangle(x, y, w, h);

	p = ui_trackmouse(&x, &y);
	if (!p)
		return;

	p->do_mousemove(state, x, y, xdiff, ydiff);
}

/**
 * [static]
 *
 * Input callback function. Pass the key event to the currently modal panel
 */
void Panel::ui_key(bool down, int code, char c)
{
	_modal->do_key(down, code, c);
}

/**
 * Set the tooltip for the panel.
 */
void Panel::set_tooltip(const char * const text) {
	if (_tooltip != text) {
		free(_tooltip);
		_tooltip = text ? strdup(text) : 0;
	}
}

/** [private]
 * Draw the tooltip.
 */
void Panel::draw_tooltip(RenderTarget* dst, Panel *lowest)
{
	int tip_width, tip_height;
	g_fh->get_size(UI_FONT_TOOLTIP, lowest->tooltip(), &tip_width, &tip_height, 0);
	tip_width += 4;
	tip_height += 4;
	assert(0 <= tip_width);
	assert(0 <= tip_height);
	const WLApplication & wlapplication = *WLApplication::get();
	Rect r
		(wlapplication.get_mouse_position() + Point(2, 32),
		 tip_width, tip_height);
	const Point tooltip_bottom_left = r.bottom_left();
	const Point screen_botton_left(g_gr->get_xres(), g_gr->get_yres());
	if (screen_botton_left.x < tooltip_bottom_left.x) r.x -=  4 + r.w;
	if (screen_botton_left.y < tooltip_bottom_left.y) r.y -= 35 + r.h;

	dst->fill_rect(r, RGBColor(230, 200, 50));
	dst->draw_rect(r, RGBColor(0, 0, 0));
	g_fh->draw_string
		(*dst,
		 UI_FONT_TOOLTIP,
		 UI_FONT_TOOLTIP_CLR,
		 r + Point(2, 2),
		 lowest->tooltip(),
		 Align_Left, -1,
		 Widget_Cache_None,
		 0, -1, false);
}
};
