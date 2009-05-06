/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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


#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "object.h"

#include "point.h"

#include <SDL_keyboard.h>

#include <cassert>
#include <string>
#include <cstring>

struct RenderTarget;

namespace UI {
/**
 * Panel is a basic rectangular UI element.
 * The outer rectangle is defined by (_x, _y, _w, _h) and encloses the entire
 * panel, including both border and inner area/rectangle.  The inner rectangle
 * is the outer rectangle minus the border sizes.  Child panel coordinates are
 * always relative to the inner rectangle.
 */
struct Panel : public Object {
	enum {
		pf_handle_mouse = 1, ///< receive mouse events
		pf_think = 2, ///< call think() function during run
		pf_top_on_click = 4, ///< bring panel on top when clicked inside it
		pf_die = 8, ///< this panel needs to die
		pf_child_die = 16, ///< a child needs to die
		pf_visible = 32, ///< render the panel
		pf_can_focus = 64, ///< can receive the keyboard focus
		/// children should snap only when overlapping the snap target
		pf_snap_windows_only_when_overlapping = 128,
		/// children should snap to the edges of this panel
		pf_dock_windows_to_edges = 256,
	}; ///<\todo Turn this into separate bool flags

	Panel
		(Panel * const nparent,
		 const int32_t nx, const int32_t ny, const uint32_t nw, const uint32_t nh,
		 const std::string & tooltip_text = std::string());
	virtual ~Panel();

	Panel * get_parent() const {return _parent;}

	void free_children();

	// Modal
	static const int32_t dying_code = -1;
	int32_t run();
	void end_modal(int32_t code);
	bool is_modal();

	virtual void start();
	virtual void end();

	// Geometry
	void set_size(uint32_t nw, uint32_t nh);
	void set_pos(Point);
	virtual void move_inside_parent();

	int32_t get_x() const {return _x;}
	int32_t get_y() const {return _y;}
	int32_t get_w() const {return _w;}
	int32_t get_h() const {return _h;}

	virtual bool is_snap_target() const {return false;}
	uint16_t get_border_snap_distance() const {return _border_snap_distance;}
	void set_border_snap_distance(const uint8_t value)
		{_border_snap_distance = value;}
	uint8_t get_panel_snap_distance () const {return _panel_snap_distance;}
	void set_panel_snap_distance(const uint8_t value)
		{_panel_snap_distance = value;}
	bool get_snap_windows_only_when_overlapping() const
		{return _flags & pf_snap_windows_only_when_overlapping;}
	void set_snap_windows_only_when_overlapping(const bool on = true);
	bool get_dock_windows_to_edges() const
		{return _flags & pf_dock_windows_to_edges;}
	void set_dock_windows_to_edges(const bool on = true);
	void set_inner_size(uint32_t nw, uint32_t nh);
	void fit_inner(Panel & inner);
	void set_border(uint32_t l, uint32_t r, uint32_t t, uint32_t b);

	uint32_t get_lborder() const {return _lborder;}
	uint32_t get_rborder() const {return _rborder;}
	uint32_t get_tborder() const {return _tborder;}
	uint32_t get_bborder() const {return _bborder;}

	int32_t get_inner_w() const {return _w - (_lborder + _rborder);}
	int32_t get_inner_h() const {return _h - (_tborder + _bborder);}

	const Panel * get_next_sibling () const {return _next;}
	Panel       * get_next_sibling ()       {return _next;}
	const Panel * get_prev_sibling () const {return _prev;}
	Panel       * get_prev_sibling ()       {return _prev;}
	const Panel * get_first_child  () const {return _fchild;}
	Panel       * get_first_child  ()       {return _fchild;}
	const Panel * get_last_child   () const {return _lchild;}
	Panel       * get_last_child   ()       {return _lchild;}

	void move_to_top();

	// Drawing, visibility
	bool is_visible() const {return _flags & pf_visible;}
	void set_visible(bool on);

	virtual void draw        (RenderTarget &);
	virtual void draw_border (RenderTarget &);
	virtual void draw_overlay(RenderTarget &);
	void update(int32_t x, int32_t y, int32_t w, int32_t h);
	void update();
	void update_inner(int32_t x, int32_t y, int32_t w, int32_t h);
	void set_cache(bool enable);

	// Events
	virtual void think();

	Point get_mouse_position() const throw ();
	void set_mouse_pos(Point);
	void center_mouse();

	virtual void handle_mousein(bool inside);
	virtual bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	virtual bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);
	virtual bool handle_mousemove
		(Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	virtual bool handle_key(bool down, SDL_keysym code);

	/// \Returns whether a certain key is currently down.
	///
	/// \note Never call this function from a keyboard event handler (a function
	/// that overrides bool handle_key(bool, SDL_keysym code)) to get the state
	/// of a modifier key. In that case code.mod must be used. It contains the
	/// state of the modifier keys at the time of the event. Unfortunately there
	/// is no information about modifier key states in mouse events (tracker
	/// item #1916453). That is "a huge oversight" in SDL 1.2 and a fix is
	/// promised in SDL 1.3:
	/// http://lists.libsdl.org/pipermail/sdl-libsdl.org/2008-March/064560.html
	bool get_key_state(SDLKey) const;

	void set_handle_mouse(bool yes);
	bool get_handle_mouse() const
		{return (_flags & pf_handle_mouse) ? true : false;}
	void grab_mouse(bool grab);

	void set_can_focus(bool yes);
	bool get_can_focus() const {return (_flags & pf_can_focus) ? true : false;}
	bool has_focus() const
		{assert(get_can_focus()); return (_parent->_focus == this);}
	void focus();

	void set_think(bool yes);
	bool get_think() const {return (_flags & pf_think) ? true : false;}

	void set_top_on_click(bool on) {
		if (on) _flags |= pf_top_on_click;
		else _flags &= ~pf_top_on_click;
	}
	bool get_top_on_click()
		const {return (_flags & pf_top_on_click) ? true : false;}

protected:
	void die();
	bool keyboard_free() {return !(_focus);}

	void play_click();

private:
	void check_child_death();

	void do_draw(RenderTarget &);
	void do_think();

	Panel * child_at_mouse_cursor
		(int32_t mouse_x, int32_t mouse_y, Panel * child);
	void do_mousein(bool inside);
	bool do_mousepress  (const Uint8 btn, int32_t x, int32_t y);
	bool do_mouserelease(const Uint8 btn, int32_t x, int32_t y);
	bool do_mousemove
		(const Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	bool do_key(bool down, SDL_keysym code);

	Panel *_parent;
	Panel *_next, *_prev;
	Panel *_fchild, *_lchild; // first, last child
	Panel *_mousein; // child panel the mouse is in
	Panel *_focus; // keyboard focus

	uint32_t _flags;
	uint32_t _cache;
	bool _needdraw;

	int32_t _x, _y;
	uint32_t _w, _h;
	uint32_t _lborder, _rborder, _tborder, _bborder;
	uint8_t _border_snap_distance, _panel_snap_distance;

	bool _running;
	int32_t _retcode;

	char * _tooltip;

	void draw_tooltip(RenderTarget &, Panel & lowest);

public:
	void set_tooltip(const char * const);
	const char * tooltip() const throw () {return _tooltip;}


private:
	static Panel *ui_trackmouse(int32_t *x, int32_t *y);
	static void ui_mousepress  (const Uint8 button, int32_t x, int32_t y);
	static void ui_mouserelease(const Uint8 button, int32_t x, int32_t y);
	static void ui_mousemove
		(const Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	static void ui_key(bool down, SDL_keysym code);

	static Panel *_modal;
	static Panel *_g_mousegrab;
	static Panel *_g_mousein;
	static uint32_t s_default_cursor;
};

inline void Panel::set_snap_windows_only_when_overlapping(const bool on) {
	_flags &= ~pf_snap_windows_only_when_overlapping;
	if (on) _flags |= pf_snap_windows_only_when_overlapping;
}
inline void Panel::set_dock_windows_to_edges(const bool on) {
	_flags &= ~pf_dock_windows_to_edges;
	if (on) _flags |= pf_dock_windows_to_edges;
}
};

#endif
