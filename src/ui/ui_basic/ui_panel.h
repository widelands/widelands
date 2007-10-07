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

#include <cassert>
#include "geometry.h"
#include <list>
#include <SDL_keyboard.h>
#include <SDL_types.h>
#include <stdint.h>
#include <string>
#include "ui_object.h"

class RenderTarget;

#define BUTTON_EDGE_BRIGHT_FACTOR 60
#define MOUSE_OVER_BRIGHT_FACTOR 15

namespace UI {

struct Panel; //forward declaration for next three lines
typedef std::list<Panel *>::iterator panellist_it;
typedef std::list<Panel *>::const_iterator panellist_cit;
typedef std::list<Panel *>::const_reverse_iterator panellist_crit;

/**
 * Panel is a basic rectangular UI element.
 * The outer rectangle is defined by (_x, _y, _w, _h) and encloses the entire panel,
 * including both border and inner area/rectangle.
 * The inner rectangle is the outer rectangle minus the border sizes.
 * Child panel coordinates are always relative to the inner rectangle.
 */
struct Panel : public Object {
	Panel
		(Panel * const nparent,
		 const int32_t nx, const int32_t ny, const uint32_t nw, const uint32_t nh,
		 const std::string & tooltip_text = std::string());
	virtual ~Panel();

	Panel *get_parent() const {return _parent;}

	void add_child(Panel * child);
	void remove_child(Panel * child);
	void free_children();

	// Modal
	static const int32_t dying_code = -1;
	int32_t run();
	void end_modal(int32_t code);

	virtual void start();
	virtual void end();

	// Geometry
	void set_size(const uint32_t nw, const uint32_t nh);
	void set_pos(const Point);
	virtual void move_inside_parent();

	int32_t get_x() const {return _x;}
	int32_t get_y() const {return _y;}
	int32_t get_w() const {return _w;}
	int32_t get_h() const {return _h;}

	virtual bool is_snap_target() const {return false;}
	uint16_t get_border_snap_distance() const {return _border_snap_distance;}
	void set_border_snap_distance(const uint8_t value) {_border_snap_distance = value;}
	uint8_t get_panel_snap_distance () const {return _panel_snap_distance;}
	void set_panel_snap_distance(const uint8_t value) {_panel_snap_distance = value;}
	bool get_snap_windows_only_when_overlapping() const {return m_snap_windows_only_when_overlapping;}
	void set_snap_windows_only_when_overlapping(const bool snap = true);
	bool get_dock_windows_to_edges() const {return m_dock_windows_to_edges;}
	void set_dock_windows_to_edges(const bool dock = true);
	void set_inner_size(uint32_t nw, uint32_t nh);
	void fit_inner(Panel* inner);
	void set_border(uint32_t l, uint32_t r, uint32_t t, uint32_t b);

	uint32_t get_lborder() const {return _lborder;}
	uint32_t get_rborder() const {return _rborder;}
	uint32_t get_tborder() const {return _tborder;}
	uint32_t get_bborder() const {return _bborder;}

	int32_t get_inner_w() const {return _w-(_lborder+_rborder);}
	int32_t get_inner_h() const {return _h-(_tborder+_bborder);}

	panellist_cit get_first_child  () const {return m_children.begin();}
	panellist_cit get_last_child  () const {return m_children.end();}

	void move_to_top();

	// Drawing, visibility
	bool get_visible() const {return m_visible;}
	void set_visible(bool visible);

	virtual void draw(RenderTarget* dst);
	virtual void draw_border(RenderTarget* dst);
	virtual void draw_overlay(RenderTarget & dst);
	void update(int32_t x, int32_t y, int32_t w, int32_t h);
	void update_inner(int32_t x, int32_t y, int32_t w, int32_t h);
	void set_cache(bool enable);

	// Events
	virtual void think();

	Point get_mouse_position() const throw();
	void set_mouse_pos(const Point);
	void center_mouse();

	virtual void handle_mousein(bool inside);
	virtual bool handle_mousepress  (const Uint8 btn, int32_t x, int32_t y);
	virtual bool handle_mouserelease(const Uint8 btn, int32_t x, int32_t y);
	virtual bool handle_mousemove
		(const Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	virtual bool handle_key(bool down, SDL_keysym code);

	void grab_mouse(bool grab);

	void set_can_focus(bool can_focus);
	bool get_can_focus() const {return m_can_focus;}
	bool has_focus() const {assert(get_can_focus()); return (_parent->_focus == this);}
	void focus();

	void set_handle_mouse(bool handle_mouse);
	bool get_handle_mouse() const {return m_handle_mouse;}

	void set_think(bool can_think);
	bool get_think() const {return m_think;}

	void set_top_on_click(bool top_on_click) {m_top_on_click=top_on_click;}
	bool get_top_on_click() const {return m_top_on_click;}

protected:
	bool keyboard_free() {return !(_focus);}

	void play_click();

private:
	void do_draw(RenderTarget* dst);

	Panel * child_at_mouse_cursor(int32_t mouse_x, int32_t mouse_y);
	void do_mousein(bool inside);
	bool do_mousepress  (const Uint8 btn, int32_t x, int32_t y);
	bool do_mouserelease(const Uint8 btn, int32_t x, int32_t y);
	bool do_mousemove(const Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	bool do_key(bool down, SDL_keysym code);

	Panel *_parent;
	std::list<Panel *> m_children;
	Panel *_mousein; ///< cache child panel the mouse is in
	Panel *_focus; ///< keyboard focus

	bool m_handle_mouse; ///< Receive mouse events
	bool m_think; ///< call think() function during run
	bool m_top_on_click; ///< bring panel on top when clicked inside it
	bool m_visible; ///< render the panel
	bool m_can_focus; ///< can receive the keyboard focus
	bool m_snap_windows_only_when_overlapping; ///< children should snap only when overlapping the snap target
	bool m_dock_windows_to_edges; ///< children should snap to the edges of this panel

	uint32_t _cache;
	bool _needdraw;

	int32_t _x, _y;
	uint32_t _w, _h;
	uint32_t _lborder, _rborder, _tborder, _bborder;
	uint8_t _border_snap_distance, _panel_snap_distance;

	bool _running;
	int32_t _retcode;

	char * _tooltip;

	void draw_tooltip(RenderTarget* dst, Panel *lowest);

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

};

#endif
