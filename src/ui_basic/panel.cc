/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "ui_basic/panel.h"

#include "base/log.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "profile/profile.h"
#include "sound/sound_handler.h"
#include "wlapplication.h"

using namespace std;

namespace UI {

Panel * Panel::_modal       = nullptr;
Panel * Panel::_g_mousegrab = nullptr;
Panel * Panel::_g_mousein   = nullptr;

// The following variable can be set to false. If so, all mouse and keyboard
// events are ignored and not passed on to any widget. This is only useful
// for scripts that want to show off functionality without the user interfering.
bool Panel::_g_allow_user_input = true;
const Image* Panel::s_default_cursor = nullptr;
const Image* Panel::s_default_cursor_click = nullptr;

/**
 * Initialize a panel, link it into the parent's queue.
 */
Panel::Panel
	(Panel * const nparent,
	 const int32_t nx, const int32_t ny, const uint32_t nw, const uint32_t nh,
	 const std::string & tooltip_text)
	:
	_parent(nparent), _fchild(nullptr), _lchild(nullptr), _mousein(nullptr), _focus(nullptr),
	_flags(pf_handle_mouse|pf_thinks|pf_visible),
	_x(nx), _y(ny), _w(nw), _h(nh),
	_lborder(0), _rborder(0), _tborder(0), _bborder(0),
	_border_snap_distance(0), _panel_snap_distance(0),
	_desired_w(nw), _desired_h(nh),
	_running(false),
	_tooltip(tooltip_text)
{
	assert(nparent != this);
	if (_parent) {
		_next = _parent->_fchild;
		_prev = nullptr;
		if (_next)
			_next->_prev = this;
		else
			_parent->_lchild = this;
		_parent->_fchild = this;
	} else
		_prev = _next = nullptr;
	update(0, 0, _w, _h);
}

/**
 * Unlink the panel from the parent's queue
 */
Panel::~Panel()
{
	update();

	// Release pointers to this object
	if (_g_mousegrab == this)
		_g_mousegrab = nullptr;
	if (_g_mousein == this)
		_g_mousein = nullptr;

	// Free children
	free_children();

	// Unlink
	if (_parent) {
		if (_parent->_mousein == this)
			_parent->_mousein = nullptr;
		if (_parent->_focus == this)
			_parent->_focus = nullptr;

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
 * Free all of the panel's children.
 */
void Panel::free_children() {
	//Scan-build claims this results in double free.
	//This is a false positive.
	//See https://bugs.launchpad.net/widelands/+bug/1198928
	while (_fchild) delete _fchild;
}


/**
 * Enters the event loop; all events will be handled by this panel.
 *
 * \return the return code passed to end_modal. This return code will be
 * negative when the event loop was quit in an abnormal way (e.g. the user
 * clicked the window's close button or similar).
 */
int Panel::do_run()
{
	// TODO(sirver): the main loop should not be in UI, but in WLApplication.
	WLApplication * const app = WLApplication::get();
	Panel * const prevmodal = _modal;
	_modal = this;
	_g_mousegrab = nullptr; // good ol' paranoia
	app->set_mouse_lock(false); // more paranoia :-)

	Panel * forefather = this;
	while (Panel * const p = forefather->_parent)
		forefather = p;

	s_default_cursor = g_gr->images().get("pics/cursor.png");
	s_default_cursor_click = g_gr->images().get("pics/cursor_click.png");

	// Loop
	_running = true;

	// Panel-specific startup code. This might call end_modal()!
	start();
	g_gr->update();

	uint32_t minTime;
	{
		int32_t maxfps = g_options.pull_section("global").get_int("maxfps", 25);
		if (maxfps < 5)
			maxfps = 5;
		minTime = 1000 / maxfps;
	}

	while (_running) {
		const uint32_t startTime = SDL_GetTicks();

		static InputCallback icb = {
			Panel::ui_mousepress,
			Panel::ui_mouserelease,
			Panel::ui_mousemove,
			Panel::ui_key,
			Panel::ui_textinput,
			Panel::ui_mousewheel
		};

		app->handle_input(&icb);
		if (app->should_die())
			end_modal<Returncodes>(Returncodes::kBack);

		do_think();

		if (g_gr->need_update()) {
			RenderTarget & rt = *g_gr->get_render_target();

			forefather->do_draw(rt);

			rt.blit
				(app->get_mouse_position() - Point(3, 7),
				 WLApplication::get()->is_mouse_pressed() ?
					s_default_cursor_click :
					s_default_cursor);

			forefather->do_tooltip();

			g_gr->refresh();
		}

		if (_flags & pf_child_die)
			check_child_death();

		//  Wait until 1second/maxfps are over.
		const uint32_t diffTime = SDL_GetTicks() - startTime;
		if (diffTime < minTime) {
			SDL_Delay(minTime - diffTime);
		}
	}
	g_gr->update();
	end();

	// Done
	_modal = prevmodal;

	return _retcode;
}

/**
 * \return \c true if this is the currently modal panel
 */
bool Panel::is_modal()
{
	return _running;
}


/**
 * Called once before the event loop in run is started
 */
void Panel::start() {}

/**
 * Called once after the event loop in run() has ended
 */
void Panel::end() {}

/**
 * Resizes the panel.
 *
 * \note NEVER override this function. If you feel the urge to override this
 * function, you probably want to override \ref layout.
 */
void Panel::set_size(const uint32_t nw, const uint32_t nh)
{
	if (nw == _w && nh == _h)
		return;

	uint32_t const upw = std::min(nw, _w);
	uint32_t const uph = std::min(nh, _h);
	_w = nw;
	_h = nh;

	if (_parent)
		move_inside_parent();

	layout();

	update(0, 0, upw, uph);
}

/**
 * Move the panel. Panel's position is relative to the parent.
 */
void Panel::set_pos(const Point n) {
	update(0, 0, _w, _h);
	_x = n.x;
	_y = n.y;
	update(0, 0, _w, _h);
}

/**
 * Set \p w and \p h to the desired
 * width and height of this panel, respectively.
 */
void Panel::get_desired_size(uint32_t & w, uint32_t & h) const
{
	w = _desired_w;
	h = _desired_h;
}

/**
 * Set this panel's desired size and invoke the recursive update of the parent.
 *
 * \note The desired size of a panel must only depend on the attributes of this
 * panel and its children that are not derived from layout routines.
 * In particular, it must be independent of the panel's position on the screen
 * or of its actual size.
 *
 * \note NEVER override this function
 */
void Panel::set_desired_size(uint32_t w, uint32_t h)
{
	if (_desired_w == w && _desired_h == h)
		return;

	assert(w < 3000);
	assert(h < 3000);

	_desired_w = w;
	_desired_h = h;
	if (!get_layout_toplevel() && _parent) {
		_parent->update_desired_size();
	} else {
		set_size(_desired_w, _desired_h);
	}
}

/**
 * Recompute this panel's desired size.
 *
 * This is automatically called whenever a child panel's desired size changes.
 */
void Panel::update_desired_size()
{
}

/**
 * Set whether this panel acts as a layouting toplevel.
 *
 * Typically, only true for \ref Window.
 */
void Panel::set_layout_toplevel(bool ltl)
{
	_flags &= ~pf_layout_toplevel;
	if (ltl)
		_flags |= pf_layout_toplevel;
}

bool Panel::get_layout_toplevel() const
{
	return _flags & pf_layout_toplevel;
}

/**
 * Interpret \p pt as a point in the interior of this panel,
 * and translate it into the interior coordinate system of the parent
 * and return the result.
 */
Point Panel::to_parent(const Point & pt) const
{
	if (!_parent)
		return pt;

	return pt + Point(_lborder + _x, _tborder + _y);
}


/**
 * Ensure the panel is inside the parent's visibile area.
 *
 * The default implementation does nothing, this is overridden
 * by \ref Window
 */
void Panel::move_inside_parent()
{
}

/**
 * Automatically layout the children of this panel and adjust their size.
 *
 * \note This is always called when this panel's size is changed, so do not
 * call \ref set_size from this function!
 *
 * The default implementation does nothing.
 */
void Panel::layout()
{
}

/**
 * Set the size of the inner area (total area minus border)
 */
void Panel::set_inner_size(uint32_t const nw, uint32_t const nh)
{
	set_size(nw + _lborder + _rborder, nh + _tborder + _bborder);
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
	update();
}

/**
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
	_prev = nullptr;
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
void Panel::set_visible(bool const on)
{
	if (((_flags & pf_visible) > 1) == on)
		return;

	_flags &= ~pf_visible;
	if (on)
		_flags |= pf_visible;

	update(0, 0, _w, _h);
}

/**
 * Redraw the panel. Note that all drawing coordinates are relative to the
 * inner area: you cannot overwrite the panel border in this function.
 */
void Panel::draw(RenderTarget &) {}

/**
 * Redraw the panel border.
 */
void Panel::draw_border(RenderTarget &) {}


/**
 * Draw overlays that appear over all child panels.
 * This can be used e.g. for debug information.
*/
void Panel::draw_overlay(RenderTarget &) {}


/**
 * Mark a part of a panel for updating.
 */
void Panel::update(int32_t x, int32_t y, int32_t w, int32_t h)
{
	if
		(x >= static_cast<int32_t>(_w) || x + w <= 0
		 ||
		 y >= static_cast<int32_t>(_h) || y + h <= 0)
		return;

	if (_parent) {
		_parent->update_inner(x + _x, y + _y, w, h);
	} else {
		if (x < 0) {
			w += x;
			x = 0;
		}
		if (x + w > g_gr->get_xres())
			w = g_gr->get_xres() - x;
		if (w <= 0)
			return;

		if (y < 0) {
			h += y;
			y = 0;
		}
		if (y + h > g_gr->get_yres())
			h = g_gr->get_yres() - y;
		if (h <= 0)
			return;

		g_gr->update();
	}
}


/**
 * Overload for convenience.
 *
 * Equivalent to update(0, 0, get_w(), get_h());
 */
void Panel::update()
{
	update(0, 0, get_w(), get_h());
}


/**
 * Mark a part of a panel for updating.
 */
void Panel::update_inner(int32_t x, int32_t y, int32_t w, int32_t h)
{
	update(x - _lborder, y - _tborder, w, h);
}

/**
 * Called once per event loop pass, unless set_think(false) has
 * been called. It is intended to be used for animations and game logic.
 */
void Panel::think()
{
}


/**
 * Descend the panel hierarchy and call the \ref think() function of all
 * (grand-)children for which set_thinks(false) has not been called.
 */
void Panel::do_think()
{
	if (thinks())
		think();

	for (Panel * child = _fchild; child; child = child->_next)
		child->do_think();
}


/**
 * Get mouse position relative to this panel
*/
Point Panel::get_mouse_position() const {
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
	if (_parent)
		_parent             ->set_mouse_pos(relative_p);
	else
		WLApplication::get()->warp_mouse   (relative_p);
}


/**
 * Center the mouse on this panel.
*/
void Panel::center_mouse() {
	set_mouse_pos(Point(get_w() / 2, get_h() / 2));
}


/**
 * Called whenever the mouse enters or leaves the panel. The inside state
 * is relative to the outer area of a panel. This means that the mouse
 * position received in handle_mousemove may be negative while the mouse is
 * still inside the panel as far as handle_mousein is concerned.
 */
void Panel::handle_mousein(bool)
{}

/**
 * Called whenever the user presses a mouse button in the panel.
 * If the panel doesn't process the mouse-click, it is handed to the panel's
 * parent.
 *
 * \return true if the mouseclick was processed, flase otherwise
 */
bool Panel::handle_mousepress  (const uint8_t, int32_t, int32_t)
{
	return false;
}

/**
 * Called whenever the user releases a mouse button in the panel.
 * If the panel doesn't process the mouse-click, it is handed to the panel's
 * parent.
 *
 * \return true if the mouseclick was processed, false otherwise
 */
bool Panel::handle_mouserelease(const uint8_t, int32_t, int32_t)
{
	return false;
}

/**
 * Called whenever the user moves the mouse wheel.
 * If the panel doesn't process the mouse-wheel, it is handed to the panel's
 * parent.
 *
 * \return true if the mouseclick was processed, false otherwise
 */
bool Panel::handle_mousewheel(uint32_t, int32_t, int32_t) {
	return false;
}


/**
 * Called when the mouse is moved while inside the panel
 */
bool Panel::handle_mousemove(const uint8_t, int32_t, int32_t, int32_t, int32_t)
{
	return !_tooltip.empty();
}


bool Panel::handle_key(bool down, SDL_Keysym code)
{
	if (down) {
		if (_focus) {
				Panel * p = _focus->_next;
				switch (code.sym) {

				case SDLK_TAB:
					while (p != _focus) {
						if (p->get_can_focus()) {
							p->focus();
							p->update();
							break;
						}
						if (p == _lchild) {
								p = _fchild;
						}
						else {
								p = p->_next;
						}
					}
					return true;

				default:
					return false;
			}
		}
	}
	return false;
}


bool Panel::handle_textinput(const std::string& /* text */) {
	return false;
}


/**
 * Called whenever a tooltip could be drawn.
 * Return true if the tooltip has been drawn,
 * false otherwise.
 */
bool Panel::handle_tooltip()
{
	RenderTarget & rt = *g_gr->get_render_target();
	return draw_tooltip(rt, tooltip());
}

/**
 * Enable/Disable mouse handling by this panel
 * Default is enabled. Note that when mouse handling is disabled, child panels
 * don't receive mouse events either.
 *
 * \param yes true if the panel should receive mouse events
 */
void Panel::set_handle_mouse(bool const yes)
{
	if (yes)
		_flags |= pf_handle_mouse;
	else
		_flags &= ~pf_handle_mouse;
}

/**
 * Enable/Disable mouse grabbing. If a panel grabs the mouse, all mouse
 * related events will be sent directly to that panel.
 * You should only grab the mouse as a response to a mouse event (e.g.
 * clicking a mouse button)
 *
 * \param grab true if the mouse should be grabbed
 */
void Panel::grab_mouse(bool const grab)
{
	if (grab) {
		_g_mousegrab = this;
	} else {
		assert(!_g_mousegrab || _g_mousegrab == this);
		_g_mousegrab = nullptr;
	}
}

/**
 * Set if this panel can receive the keyboard focus
*/
void Panel::set_can_focus(bool const yes)
{

	if (yes)
		_flags |= pf_can_focus;
	else {
		_flags &= ~pf_can_focus;

		if (_parent && _parent->_focus == this)
			_parent->_focus = nullptr;
	}
}

/**
 * Grabs the keyboard focus, if it can,
 * topcaller identifies widget at the beginning of the recursion
 */
void Panel::focus(const bool topcaller)
{
	if (topcaller) {
		if (handles_textinput()) {
			if (!SDL_IsTextInputActive()) {
				SDL_StartTextInput();
			}
		} else {
			if (SDL_IsTextInputActive()) {
				SDL_StopTextInput();
			}
		}
	}

	if (!_parent || this == _modal) {
		return;
	}
	if (_parent->_focus == this)
		return;

	_parent->_focus = this;
	_parent->focus(false);
}

/**
 * Enables/Disables calling think() during the event loop.
 * The default is enabled.
 *
 * \param yes true if the panel's think function should be called
 */
void Panel::set_thinks(bool const yes)
{
	if (yes)
		_flags |= pf_thinks;
	else
		_flags &= ~pf_thinks;
}

/**
 * Cause this panel to be removed on the next frame.
 * Use this for a panel that needs to destroy itself after a button has
 * been pressed (e.g. non-modal dialogs).
 * Do NOT use this to delete a hierarchy of panels that have been modal.
 */
void Panel::die()
{
	_flags |= pf_die;

	for (Panel * p = _parent; p; p = p->_parent) {
		p->_flags |= pf_child_die;
		if (p == _modal)
			break;
	}
}

/**
 * Wrapper around SoundHandler::play_fx() to prevent having to include
 * sound_handler.h in every UI subclass just for playing a 'click'
 */
void Panel::play_click()
{
	g_sound_handler.play_fx("sound/click", 128, PRIO_ALWAYS_PLAY);
}
void Panel::play_new_chat_message()
{
	g_sound_handler.play_fx("sound/lobby_chat", 128, PRIO_ALWAYS_PLAY);
}
void Panel::play_new_chat_member()
{
	g_sound_handler.play_fx("sound/lobby_freshmen", 128, PRIO_ALWAYS_PLAY);
}


/**
 * Recursively walk the panel tree, killing panels that are marked for death
 * using die().
 */
void Panel::check_child_death()
{
	Panel * next = _fchild;
	while (next) {
		Panel * p = next;
		next = p->_next;

		if (p->_flags & pf_die)
			delete p;
		else if (p->_flags & pf_child_die)
			p->check_child_death();
	}

	_flags &= ~pf_child_die;
}


/**
 * Draw the inner region of the panel into the given target.
 *
 * \param dst target to render into, assumed to be prepared for the panel's
 * inner coordinate system.
 */
void Panel::do_draw_inner(RenderTarget & dst)
{
	draw(dst);

	// draw back to front
	for (Panel * child = _lchild; child; child = child->_prev)
		child->do_draw(dst);

	draw_overlay(dst);
}


/**
 * Subset for the border first and draw the border, then subset for the inner
 * area and draw the inner area.
 * Draw child panels after drawing self.
 * Draw tooltip if required.
 *
 * \param dst RenderTarget for the parent Panel
*/
void Panel::do_draw(RenderTarget & dst)
{
	if (!is_visible())
		return;

	Rect outerrc;
	Point outerofs;

	if (!dst.enter_window(Rect(Point(_x, _y), _w, _h), &outerrc, &outerofs))
		return;

	draw_border(dst);

	Rect innerwindow
		(Point(_lborder, _tborder),
			_w - (_lborder + _rborder), _h - (_tborder + _bborder));

	if (dst.enter_window(innerwindow, nullptr, nullptr))
		do_draw_inner(dst);

	dst.set_window(outerrc, outerofs);
}


/**
 * Returns the child panel that receives mouse events at the given location.
 * Starts the search with child (which should usually be set to _fchild) and
 * returns the first match.
 */
inline Panel * Panel::child_at_mouse_cursor
	(int32_t const x, int32_t const y, Panel * child)
{

	for (; child; child = child->_next) {
		if (!child->handles_mouse() || !child->is_visible())
			continue;
		if
			(x < child->_x + static_cast<int32_t>(child->_w) && x >= child->_x
			 &&
			 y < child->_y + static_cast<int32_t>(child->_h) && y >= child->_y)
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
void Panel::do_mousein(bool const inside)
{
	if (!inside && _mousein) {
		_mousein->do_mousein(false);
		_mousein = nullptr;
	}
	handle_mousein(inside);
}

/**
 * Propagate mousepresses/-releases/-moves to the appropriate panel.
 *
 * Returns whether the event was processed.
 */
bool Panel::do_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	if (get_can_focus()) {
		focus();
	}
	x -= _lborder;
	y -= _tborder;
	if (_flags & pf_top_on_click)
		move_to_top();

	if (_g_mousegrab != this)
		for
			(Panel * child = _fchild;
			 (child = child_at_mouse_cursor(x, y, child));
			 child = child->_next)
			{
				if (child->do_mousepress(btn, x - child->_x, y - child->_y))
					return true;
			}
	return handle_mousepress(btn, x, y);
}


bool Panel::do_mousewheel(uint32_t which, int32_t x, int32_t y) {
	// TODO(GunChleoc): This is just a hack for focussed panels
	// We need to find the actualy scrollable panel beneaththe mouse cursor,
	// so we can have multiple scrollable elements on the same screen
	// e.g. load map with a long desctiprion has 2 of them.
	if (_focus) {
		if (_focus->do_mousewheel(which, x, y))
			return true;
	}

	return handle_mousewheel(which, x, y);
}


bool Panel::do_mouserelease(const uint8_t btn, int32_t x, int32_t y) {
	x -= _lborder;
	y -= _tborder;
	if (_g_mousegrab != this)
		for
			(Panel * child = _fchild;
			 (child = child_at_mouse_cursor(x, y, child));
			 child = child->_next)
			if (child->do_mouserelease(btn, x - child->_x, y - child->_y))
				return true;
	return handle_mouserelease(btn, x, y);
}

bool Panel::do_mousemove
	(uint8_t const state,
	 int32_t x, int32_t y, int32_t const xdiff, int32_t const ydiff)
{
	x -= _lborder;
	y -= _tborder;
	if (_g_mousegrab != this) {
		for
			(Panel * child = _fchild;
			 (child = child_at_mouse_cursor(x, y, child));
			 child = child->_next)
		{
			if
				(child->do_mousemove
				 	(state, x - child->_x, y - child->_y, xdiff, ydiff))
			{
				return true;
			}
		}
	}
	return handle_mousemove(state, x, y, xdiff, ydiff);
}

/**
 * Pass the key event to the focused child.
 * If it doesn't process the key, we'll see if we can use the event.
 */
bool Panel::do_key(bool const down, SDL_Keysym const code)
{
	if (_focus && _focus->do_key(down, code)) {
		return true;
	}

	// If we handle text, it does not matter if we handled this key
	// or not, it should not propagate.
	if (handle_key(down, code) || handles_textinput()) {
		return true;
	}
	return false;
}

bool Panel::do_textinput(const std::string& text) {
	if (_focus && _focus->do_textinput(text)) {
		return true;
	}

	if (!handles_textinput()) {
		return false;
	}

	return handle_textinput(text);
}

bool Panel::do_tooltip()
{
	if (_mousein && _mousein->do_tooltip()) {
		return true;
	}
	return handle_tooltip();
}

/**
 * \return \c true if the given key is currently pressed, or \c false otherwise
 */
bool Panel::get_key_state(const SDL_Scancode key) const
{
	return WLApplication::get()->get_key_state(key);
}

/**
 * Determine which panel is to receive a mouse event.
 *
 * \return The panel which receives the mouse event
 */
Panel * Panel::ui_trackmouse(int32_t & x, int32_t & y)
{
	Panel * mousein;
	Panel * rcv = nullptr;

	if (_g_mousegrab)
		mousein = rcv = _g_mousegrab;
	else
		mousein = _modal;

	x -= mousein->_x;
	y -= mousein->_y;
	for (Panel * p = mousein->_parent; p; p = p->_parent) {
		x -= p->_lborder + p->_x;
		y -= p->_tborder + p->_y;
	}

	if
		(0 <= x && x < static_cast<int32_t>(mousein->_w)
		 &&
		 0 <= y && y < static_cast<int32_t>(mousein->_h))
		rcv = mousein;
	else
		mousein = nullptr;

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
 * Input callback function. Pass the mouseclick event to the currently modal
 * panel.
*/
bool Panel::ui_mousepress(const uint8_t button, int32_t x, int32_t y) {
	if (!_g_allow_user_input) {
		return true;
	}

	Panel * const p = ui_trackmouse(x, y);
	if (p == nullptr) {
		return false;
	}
	return p->do_mousepress(button, x, y);
}

bool Panel::ui_mouserelease(const uint8_t button, int32_t x, int32_t y) {
	if (!_g_allow_user_input) {
		return true;
	}

	Panel * const p = ui_trackmouse(x, y);
	if (p == nullptr) {
		return false;
	}
	return p->do_mouserelease(button, x, y);
}

/**
 * Input callback function. Pass the mousemove event to the currently modal
 * panel.
*/
bool Panel::ui_mousemove
	(uint8_t const state,
	 int32_t x, int32_t y, int32_t const xdiff, int32_t const ydiff)
{
	if (!_g_allow_user_input) {
		return true;
	}

	if (!xdiff && !ydiff) {
		return true;
	}

	Panel * p;
	g_gr->update();

	p = ui_trackmouse(x, y);
	if (!p)
		return false;

	return p->do_mousemove(state, x, y, xdiff, ydiff);
}

/**
 * Input callback function. Pass the mousewheel event to the currently modal
 * panel.
*/
bool Panel::ui_mousewheel(uint32_t which, int32_t x, int32_t y) {
	if (!_g_allow_user_input) {
		return true;
	}
	if (!x && !y) {
		return true;
	}
	Panel* p = nullptr;
	if (_g_mousein) {
		p = _g_mousein;
	} else {
		p = _g_mousegrab ? _g_mousegrab : _modal;
	}
	if (!p) {
		return false;
	}
	return p->do_mousewheel(which, x, y);
}


/**
 * Input callback function. Pass the key event to the currently modal panel
 */
bool Panel::ui_key(bool const down, SDL_Keysym const code)
{
	if (!_g_allow_user_input) {
		return true;
	}

	return _modal->do_key(down, code);
}


/**
 * Input callback function. Pass the textinput event to the currently modal panel
 */
bool Panel::ui_textinput(const std::string& text) {
	if (!_g_allow_user_input) {
		return true;
	}
	return _modal->do_textinput(text);
}

/**
 * Draw the tooltip. Return true on success
 */
bool Panel::draw_tooltip(RenderTarget & dst, const std::string & text)
{
	if (text.empty()) {
		return false;
	}
	std::string text_to_render = text;
	if (!is_richtext(text_to_render)) {
		text_to_render = as_tooltip(text);
	}

	static const uint32_t TIP_WIDTH_MAX = 360;
	const Image* rendered_text = g_fh1->render(text_to_render, TIP_WIDTH_MAX);
	if (!rendered_text) {
		return false;
	}
	uint16_t tip_width = rendered_text->width() + 4;
	uint16_t tip_height = rendered_text->height() + 4;

	Rect r
		(WLApplication::get()->get_mouse_position() + Point(2, 32),
		 tip_width, tip_height);
	const Point tooltip_bottom_right = r.bottom_right();
	const Point screen_bottom_right(g_gr->get_xres(), g_gr->get_yres());
	if (screen_bottom_right.x < tooltip_bottom_right.x)
		r.x -=  4 + r.w;
	if (screen_bottom_right.y < tooltip_bottom_right.y)
		r.y -= 35 + r.h;

	dst.fill_rect(r, RGBColor(63, 52, 34));
	dst.draw_rect(r, RGBColor(0, 0, 0));
	dst.blit(r.top_left() + Point(2, 2), rendered_text);
	return true;
}

std::string Panel::ui_fn() {
	std::string style(UI::g_fh1->fontset().serif());
	if (g_fs->file_exists("i18n/fonts/" + style)) {
		return style;
	}
	log
		("Could not find font file \"%s\"\n"
		 "Make sure the path is given relative to Widelands font directory. "
		 "Widelands will use standard font.\n",
		 style.c_str());
	return UI::FontSet::kFallbackFont;
}

}
