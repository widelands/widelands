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


#ifndef __S__PANEL_H
#define __S__PANEL_H

#include "constants.h"
#include "error.h"
#include "font_handler.h"
#include "rendertarget.h"
#include "sound_handler.h"
#include "types.h"
#include "ui_object.h"

class RenderTarget;

#define BUTTON_EDGE_BRIGHT_FACTOR 60
#define MOUSE_OVER_BRIGHT_FACTOR 15

/**
 * UIPanel is a basic rectangular UI element.
 * The outer rectangle is defined by (_x,_y,_w,_h) and encloses the entire panel,
 * including both border and inner area/rectangle.
 * The inner rectangle is the outer rectangle minus the border sizes.
 * Child panel coordinates are always relative to the inner rectangle.
 */
class UIPanel : public UIObject {
public:
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

	UIPanel(UIPanel *nparent, const int nx, const int ny, const uint nw, const uint nh);
	virtual ~UIPanel();

	inline UIPanel *get_parent() const { return _parent; }

	void free_children();

	// Modal
	int run();
	void end_modal(int code);

	virtual void start();
	virtual void end();

	// Geometry
	void set_size(const uint nw, const uint nh);
	void set_pos(const int nx, const int ny);

	inline int get_x() const { return _x; }
	inline int get_y() const { return _y; }
	inline int get_w() const { return _w; }
	inline int get_h() const { return _h; }

	virtual bool is_snap_target() const {return false;}
	ushort get_border_snap_distance() const {return _border_snap_distance;}
	void set_border_snap_distance(const uchar value) {_border_snap_distance = value;}
	uchar get_panel_snap_distance () const {return _panel_snap_distance;}
	void set_panel_snap_distance(const uchar value) {_panel_snap_distance = value;}
	bool get_snap_windows_only_when_overlapping() const {return _flags & pf_snap_windows_only_when_overlapping;}
	void set_snap_windows_only_when_overlapping(const bool on = true);
	bool get_dock_windows_to_edges() const {return _flags & pf_dock_windows_to_edges;}
	void set_dock_windows_to_edges(const bool on = true);
	void set_inner_size(uint nw, uint nh);
	void fit_inner(UIPanel* inner);
	void set_border(uint l, uint r, uint t, uint b);

	inline uint get_lborder() const { return _lborder; }
	inline uint get_rborder() const { return _rborder; }
	inline uint get_tborder() const { return _tborder; }
	inline uint get_bborder() const { return _bborder; }

	inline int get_inner_w() const { return _w-(_lborder+_rborder); }
	inline int get_inner_h() const { return _h-(_tborder+_bborder); }

	const UIPanel * get_next_sibling () const {return _next;}
	const UIPanel * get_prev_sibling () const {return _prev;}
	const UIPanel * get_first_child  () const {return _fchild;}
	const UIPanel * get_last_child   () const {return _lchild;}

	void move_to_top();

	// Drawing, visibility
	inline bool get_visible() const { return (_flags & pf_visible) ? true : false; }
	void set_visible(bool on);

	virtual void draw(RenderTarget* dst);
	virtual void draw_border(RenderTarget* dst);
	virtual void draw_overlay(RenderTarget* dst);
	void update(int x, int y, int w, int h);
	void update_inner(int x, int y, int w, int h);
	void set_cache(bool enable);

	// Events
	virtual void think();

	int get_mouse_x();
	int get_mouse_y();
	void set_mouse_pos(int x, int y);
	void center_mouse();

	virtual void handle_mousein(bool inside);
	virtual bool handle_mouseclick(uint btn, bool down, int x, int y);
	virtual void handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns);
	virtual bool handle_key(bool down, int code, char c);

	void set_handle_mouse(bool yes);
	inline bool get_handle_mouse() const { return (_flags & pf_handle_mouse) ? true : false; }
	void grab_mouse(bool grab);

	void set_can_focus(bool yes);
	inline bool get_can_focus() const { return (_flags & pf_can_focus) ? true : false; }
	inline bool has_focus() const { assert(get_can_focus()); return (_parent->_focus == this); }
   void focus();

	void set_think(bool yes);
	inline bool get_think() const { return (_flags & pf_think) ? true : false; }

	inline void set_top_on_click(bool on) {
		if (on) _flags |= pf_top_on_click;
		else _flags &= ~pf_top_on_click;
	}
	inline bool get_top_on_click() const { return (_flags & pf_top_on_click) ? true : false; }

protected:
	void die();

   bool keyboard_free(void) { return !(_focus); }

	void play_click();

private:
	void check_child_death();

	void do_draw(RenderTarget* dst);

	UIPanel *get_mousein(int x, int y);
	void do_mousein(bool inside);
	bool do_mouseclick(uint btn, bool down, int x, int y);
	void do_mousemove(int x, int y, int xdiff, int ydiff, uint btns);
	bool do_key(bool down, int code, char c);

	UIPanel *_parent;
	UIPanel *_next, *_prev;
	UIPanel *_fchild, *_lchild; // first, last child
	UIPanel *_mousein; // child panel the mouse is in
	UIPanel *_focus; // keyboard focus

	uint _flags;
	uint _cache;
	bool _needdraw;

	int _x, _y;
	uint _w, _h;
	uint _lborder, _rborder, _tborder, _bborder;
	uchar _border_snap_distance, _panel_snap_distance;

	bool _running;
	int _retcode;

	bool _use_tooltip;
	char _tooltip[255];

	void draw_tooltip(RenderTarget* dst, UIPanel *lowest);

public:
	void set_tooltip(const char tooltip[255]);
	void unset_tooltip();
	inline char* get_tooltip(){return _tooltip;}
	bool use_tooltip(){return _use_tooltip;}


private:
	static UIPanel *ui_trackmouse(int *x, int *y);
	static void ui_mouseclick(bool down, int button, uint btns, int x, int y);
	static void ui_mousemove(uint btns, int x, int y, int xdiff, int ydiff);
	static void ui_key(bool down, int code, char c);

	static UIPanel *_modal;
	static UIPanel *_g_mousegrab;
	static UIPanel *_g_mousein;
	static uint s_default_cursor;
};

inline void UIPanel::set_snap_windows_only_when_overlapping(const bool on) {
	_flags &= ~pf_snap_windows_only_when_overlapping;
	if (on) _flags |= pf_snap_windows_only_when_overlapping;
}
inline void UIPanel::set_dock_windows_to_edges(const bool on) {
	_flags &= ~pf_dock_windows_to_edges;
	if (on) _flags |= pf_dock_windows_to_edges;
}

#endif
