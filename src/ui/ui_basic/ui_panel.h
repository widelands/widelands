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

/** 
 * Panel is a basic rectangular UI element.
 * The outer rectangle is defined by (_x,_y,_w,_h) and encloses the entire panel,
 * including both border and inner area/rectangle.
 * The inner rectangle is the outer rectangle minus the border sizes.
 * Child panel coordinates are always relative to the inner rectangle.
 */
class Panel : public UIObject {
public:
	enum {
		pf_handle_mouse = 1, ///< receive mouse events
		pf_think = 2, ///< call think() function during run
		pf_top_on_click = 4, ///< bring panel on top when clicked inside it
		pf_die = 8, ///< this panel needs to die
		pf_child_die = 16, ///< a child needs to die
		pf_visible = 32, ///< render the panel
		pf_can_focus = 64, ///< can receive the keyboard focus
	};

	Panel(Panel *nparent, const int nx, const int ny, const uint nw, const uint nh);
	virtual ~Panel();

	inline Panel *get_parent() const { return _parent; }

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

	void set_inner_size(uint nw, uint nh);
	void fit_inner(Panel* inner);
	void set_border(uint l, uint r, uint t, uint b);

	inline uint get_lborder() const { return _lborder; }
	inline uint get_rborder() const { return _rborder; }
	inline uint get_tborder() const { return _tborder; }
	inline uint get_bborder() const { return _bborder; }

	inline int get_inner_w() const { return _w-(_lborder+_rborder); }
	inline int get_inner_h() const { return _h-(_tborder+_bborder); }

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

private:
	void check_child_death();

	void do_draw(RenderTarget* dst);

	Panel *get_mousein(int x, int y);
	void do_mousein(bool inside);
	bool do_mouseclick(uint btn, bool down, int x, int y);
	void do_mousemove(int x, int y, int xdiff, int ydiff, uint btns);
	bool do_key(bool down, int code, char c);

	Panel *_parent;
	Panel *_next, *_prev;
	Panel *_fchild, *_lchild; // first, last child
	Panel *_mousein; // child panel the mouse is in
	Panel *_focus; // keyboard focus

	uint _flags;
	uint _cache;
	bool _needdraw;

	int _x, _y;
	uint _w, _h;
	uint _lborder, _rborder, _tborder, _bborder;

	bool _running;
	int _retcode;

private:
	static Panel *ui_trackmouse(int *x, int *y);
	static void ui_mouseclick(bool down, int button, uint btns, int x, int y);
	static void ui_mousemove(uint btns, int x, int y, int xdiff, int ydiff);
	static void ui_key(bool down, int code, char c);

	static Panel *_modal;
	static Panel *_g_mousegrab;
	static Panel *_g_mousein;
	static uint s_default_cursor;
};

#endif
