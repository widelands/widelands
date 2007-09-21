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


#ifndef __S__PANEL_H
#define __S__PANEL_H

#include "geometry.h"
#include "ui_object.h"

#include <SDL_types.h>
#include <stdint.h>

#include <cassert>
#include <string>

class RenderTarget;

#define BUTTON_EDGE_BRIGHT_FACTOR 60
#define MOUSE_OVER_BRIGHT_FACTOR 15

namespace UI {
/**
 * Panel is a basic rectangular UI element.
 * The outer rectangle is defined by (_x, _y, _w, _h) and encloses the entire panel,
 * including both border and inner area/rectangle.
 * The inner rectangle is the outer rectangle minus the border sizes.
 * Child panel coordinates are always relative to the inner rectangle.
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
		pf_snap_windows_only_when_overlapping = 128, ///< children should snap only when overlapping the snap target
		pf_dock_windows_to_edges = 256, ///< children should snap to the edges of this panel
	};

	Panel
		(Panel * const nparent,
		 const int nx, const int ny, const uint32_t nw, const uint32_t nh,
		 const std::string & tooltip_text = std::string());
	virtual ~Panel();

	inline Panel *get_parent() const {return _parent;}

	void free_children();

	// Modal
	static const int dying_code = -1;
	int run();
	void end_modal(int code);

	virtual void start();
	virtual void end();

	// Geometry
	void set_size(const uint32_t nw, const uint32_t nh);
	void set_pos(const Point);
	virtual void move_inside_parent();

	inline int get_x() const {return _x;}
	inline int get_y() const {return _y;}
	inline int get_w() const {return _w;}
	inline int get_h() const {return _h;}

	virtual bool is_snap_target() const {return false;}
	uint16_t get_border_snap_distance() const {return _border_snap_distance;}
	void set_border_snap_distance(const uint8_t value) {_border_snap_distance = value;}
	uint8_t get_panel_snap_distance () const {return _panel_snap_distance;}
	void set_panel_snap_distance(const uint8_t value) {_panel_snap_distance = value;}
	bool get_snap_windows_only_when_overlapping() const {return _flags & pf_snap_windows_only_when_overlapping;}
	void set_snap_windows_only_when_overlapping(const bool on = true);
	bool get_dock_windows_to_edges() const {return _flags & pf_dock_windows_to_edges;}
	void set_dock_windows_to_edges(const bool on = true);
	void set_inner_size(uint32_t nw, uint32_t nh);
	void fit_inner(Panel* inner);
	void set_border(uint32_t l, uint32_t r, uint32_t t, uint32_t b);

	inline uint32_t get_lborder() const {return _lborder;}
	inline uint32_t get_rborder() const {return _rborder;}
	inline uint32_t get_tborder() const {return _tborder;}
	inline uint32_t get_bborder() const {return _bborder;}

	inline int get_inner_w() const {return _w-(_lborder+_rborder);}
	inline int get_inner_h() const {return _h-(_tborder+_bborder);}

	const Panel * get_next_sibling () const {return _next;}
	const Panel * get_prev_sibling () const {return _prev;}
	const Panel * get_first_child  () const {return _fchild;}
	const Panel * get_last_child   () const {return _lchild;}

	void move_to_top();

	// Drawing, visibility
	inline bool get_visible() const {return (_flags & pf_visible) ? true : false;}
	void set_visible(bool on);

	virtual void draw(RenderTarget* dst);
	virtual void draw_border(RenderTarget* dst);
	virtual void draw_overlay(RenderTarget & dst);
	void update(int x, int y, int w, int h);
	void update_inner(int x, int y, int w, int h);
	void set_cache(bool enable);

	// Events
	virtual void think();

	Point get_mouse_position() const throw();
	void set_mouse_pos(const Point);
	void center_mouse();

	virtual void handle_mousein(bool inside);
	virtual bool handle_mousepress  (const Uint8 btn, int x, int y);
	virtual bool handle_mouserelease(const Uint8 btn, int x, int y);
	virtual bool handle_mousemove
		(const Uint8 state, int x, int y, int xdiff, int ydiff);
	virtual bool handle_key(bool down, int code, char c);

	void set_handle_mouse(bool yes);
	inline bool get_handle_mouse() const {return (_flags & pf_handle_mouse) ? true : false;}
	void grab_mouse(bool grab);

	void set_can_focus(bool yes);
	inline bool get_can_focus() const {return (_flags & pf_can_focus) ? true : false;}
	inline bool has_focus() const {assert(get_can_focus()); return (_parent->_focus == this);}
   void focus();

	void set_think(bool yes);
	inline bool get_think() const {return (_flags & pf_think) ? true : false;}

	inline void set_top_on_click(bool on) {
		if (on) _flags |= pf_top_on_click;
		else _flags &= ~pf_top_on_click;
	}
	inline bool get_top_on_click() const {return (_flags & pf_top_on_click) ? true : false;}

protected:
	void die();

   bool keyboard_free() {return !(_focus);}

	void play_click();

private:
	void check_child_death();

	void do_draw(RenderTarget* dst);

	/**
	 * Returns the child panel that receives mouse events at the given location.
	 * Starts the search with child (which should usually be set to _fchild) and
	 * returns the first match.
	 */
	Panel * child_at_mouse_cursor(int mouse_x, int mouse_y, Panel * child);
	void do_mousein(bool inside);
	bool do_mousepress  (const Uint8 btn, int x, int y);
	bool do_mouserelease(const Uint8 btn, int x, int y);
	bool do_mousemove(const Uint8 state, int x, int y, int xdiff, int ydiff);
	bool do_key(bool down, int code, char c);

	Panel *_parent;
	Panel *_next, *_prev;
	Panel *_fchild, *_lchild; // first, last child
	Panel *_mousein; // child panel the mouse is in
	Panel *_focus; // keyboard focus

	uint32_t _flags;
	uint32_t _cache;
	bool _needdraw;

	int _x, _y;
	uint32_t _w, _h;
	uint32_t _lborder, _rborder, _tborder, _bborder;
	uint8_t _border_snap_distance, _panel_snap_distance;

	bool _running;
	int _retcode;

	char * _tooltip;

	void draw_tooltip(RenderTarget* dst, Panel *lowest);

public:
	void set_tooltip(const char * const);
	const char * tooltip() const throw () {return _tooltip;}


private:
	static Panel *ui_trackmouse(int *x, int *y);
	static void ui_mousepress  (const Uint8 button, int x, int y);
	static void ui_mouserelease(const Uint8 button, int x, int y);
	static void ui_mousemove
		(const Uint8 state, int x, int y, int xdiff, int ydiff);
	static void ui_key(bool down, int code, char c);

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
