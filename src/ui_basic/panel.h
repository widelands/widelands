/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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


#ifndef WL_UI_BASIC_PANEL_H
#define WL_UI_BASIC_PANEL_H

#include <cassert>
#include <cstring>
#include <string>

#include <SDL_keyboard.h>
#include <boost/signals2/trackable.hpp>

#include "base/macros.h"
#include "base/point.h"

class RenderTarget;
class Image;

namespace UI {

/**
 * Panel is a basic rectangular UI element.
 *
 * Every panel has an outer rectangle and an inner rectangle. The inner rectangle
 * is always equal to the outer rectangle minus the border size. Child panel coordinates
 * are always relative to the inner rectangle.
 *
 * Every panel has an actual size and a desired size. In general, a panel should never
 * change its own actual size, but only its desired size. It is up to the parent panel
 * to adjust the actual size based on the desired size.
 *
 * The desired size is a quantity that is computed in \ref update_desired_size based
 * only on the attributes of this panel and of its children. The actual size may differ
 * from this, and when it does, a panel may have to adapt also its children's positions
 * and actual sizes in \ref layout.
 *
 * If a panel is the top-level panel, or if has \ref set_layout_toplevel, then whenever
 * its desired size changes, this automatically changes the actual size (which then invokes
 * \ref layout and \ref move_inside_parent).
 */
class Panel : public boost::signals2::trackable {
public:
	enum {
		pf_handle_mouse = 1, ///< receive mouse events
		pf_thinks = 2, ///< call think() function during run
		pf_top_on_click = 4, ///< bring panel on top when clicked inside it
		pf_die = 8, ///< this panel needs to die
		pf_child_die = 16, ///< a child needs to die
		pf_visible = 32, ///< render the panel
		pf_can_focus = 64, ///< can receive the keyboard focus
		/// children should snap only when overlapping the snap target
		pf_snap_windows_only_when_overlapping = 128,
		/// children should snap to the edges of this panel
		pf_dock_windows_to_edges = 256,
		/// whether any change in the desired size should propagate to the actual size
		pf_layout_toplevel = 512,
		/// whether widget wants to receive unicode textinput messages
		pf_handle_textinput = 1024,
	};

	Panel
		(Panel * const nparent,
		 int32_t  const nx, int32_t  const ny,
		 int const nw, int const nh,
		 const std::string& tooltip_text = std::string());
	virtual ~Panel();

	Panel * get_parent() const {return parent_;}

	void free_children();

	// Modal
	enum class Returncodes {
		kBack,
		kOk
	};

	template<typename Returncode>
	Returncode run() {
		return static_cast<Returncode>(do_run());
	}
	int do_run();

	/**
	 * Cause run() to return as soon as possible, with the given return code
	 */
	template<typename Returncode>
	void end_modal(const Returncode& code) {
		running_ = false;
		return_code_ = static_cast<int>(code);
	}

	bool is_modal();

	virtual void start();
	virtual void end();

	// Geometry
	void set_size(int nw, int nh);
	void set_desired_size(int w, int h);
	void set_pos(Point);
	virtual void move_inside_parent();
	virtual void layout();

	void set_layout_toplevel(bool ltl);
	bool get_layout_toplevel() const;

	void get_desired_size(int* w, int* h) const;

	int32_t get_x() const {return x_;}
	int32_t get_y() const {return y_;}
	Point get_pos() const {return Point(x_, y_);}
	//int unstead of uint because of overflow situations
	int32_t get_w() const {return w_;}
	int32_t get_h() const {return h_;}

	Point to_parent(const Point &) const;

	virtual bool is_snap_target() const {return false;}
	uint16_t get_border_snap_distance() const {return border_snap_distance_;}
	void set_border_snap_distance(uint8_t const value) {
		border_snap_distance_ = value;
	}
	uint8_t get_panel_snap_distance () const {return panel_snap_distance_;}
	void set_panel_snap_distance(uint8_t const value) {
		panel_snap_distance_ = value;
	}
	bool get_snap_windows_only_when_overlapping() const {
		return flags_ & pf_snap_windows_only_when_overlapping;
	}
	void set_snap_windows_only_when_overlapping(const bool on = true);
	bool get_dock_windows_to_edges() const {
		return flags_ & pf_dock_windows_to_edges;
	}
	void set_dock_windows_to_edges(const bool on = true);
	void set_inner_size(int nw, int nh);
	void set_border(int l, int r, int t, int b);

	int get_lborder() const {return lborder_;}
	int get_rborder() const {return rborder_;}
	int get_tborder() const {return tborder_;}
	int get_bborder() const {return bborder_;}

	int get_inner_w() const {
		assert(lborder_ + rborder_ <= w_);
		return w_ - (lborder_ + rborder_);
	}
	int get_inner_h() const {
		assert(tborder_ + bborder_ <= h_);
		return h_ - (tborder_ + bborder_);
	}

	const Panel * get_next_sibling () const {return next_;}
	Panel       * get_next_sibling ()       {return next_;}
	const Panel * get_prev_sibling () const {return prev_;}
	Panel       * get_prev_sibling ()       {return prev_;}
	const Panel * get_first_child  () const {return first_child_;}
	Panel       * get_first_child  ()       {return first_child_;}
	const Panel * get_last_child   () const {return last_child_;}
	Panel       * get_last_child   ()       {return last_child_;}

	void move_to_top();

	// Drawing, visibility
	bool is_visible() const {return flags_ & pf_visible;}
	void set_visible(bool on);

	virtual void draw        (RenderTarget &);
	virtual void draw_border (RenderTarget &);
	virtual void draw_overlay(RenderTarget &);

	// Events
	virtual void think();

	Point get_mouse_position() const;
	void set_mouse_pos(Point);
	void center_mouse();

	virtual void handle_mousein(bool inside);
	virtual bool handle_mousepress  (uint8_t btn, int32_t x, int32_t y);
	virtual bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y);
	virtual bool handle_mousemove
		(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	virtual bool handle_mousewheel(uint32_t which, int32_t x, int32_t y);
	virtual bool handle_key(bool down, SDL_Keysym);
	virtual bool handle_textinput(const std::string& text);
	virtual bool handle_tooltip();

	/// \returns whether a certain given is currently down.
	///
	/// \note Never call this function from a keyboard event handler (a function
	/// that overrides bool handle_key(bool, SDL_Keysym code)) to get the state
	/// of a modifier key. In that case code.mod must be used. It contains the
	/// state of the modifier keys at the time of the event. Unfortunately there
	/// is no information about modifier key states in mouse events (tracker
	/// item #1916453). That is "a huge oversight" in SDL 1.2 and a fix is
	/// promised in SDL 1.3:
	/// http://lists.libsdl.org/pipermail/sdl-libsdl.org/2008-March/064560.html
	bool get_key_state(SDL_Scancode) const;

	void set_handle_mouse(bool yes);
	void grab_mouse(bool grab);

	void set_can_focus(bool yes);
	bool get_can_focus() const {return flags_ & pf_can_focus;}
	bool has_focus() const {
		assert(get_can_focus());
		return (parent_->focus_ == this);
	}
	virtual void focus(bool topcaller = true);

	void set_top_on_click(bool const on) {
		if (on)
			flags_ |= pf_top_on_click;
		else
			flags_ &= ~pf_top_on_click;
	}
	bool get_top_on_click() const {return flags_ & pf_top_on_click;}

	static void set_allow_user_input(bool const t) {allow_user_input_ = t;}
	static bool allow_user_input() {return allow_user_input_;}

	void set_tooltip(const std::string& text) {tooltip_ = text;}
	const std::string& tooltip() const {return tooltip_;}

	virtual void die();

protected:
	// This panel will never receive keypresses (do_key), instead
	// textinput will be passed on (do_textinput).
	void set_handle_textinput() {
		flags_ |= pf_handle_textinput;
	}

	// Defines if think() should be called repeatedly. This is true on construction.
	void set_thinks(bool yes);


	bool keyboard_free() {return !(focus_);}

	virtual void update_desired_size();

	static void play_click();
	static void play_new_chat_member();
	static void play_new_chat_message();

	static bool draw_tooltip(RenderTarget &, const std::string & text);

private:
	bool handles_mouse() const {
		return (flags_ & pf_handle_mouse) != 0;
	}
	bool handles_textinput() const {
		return (flags_ & pf_handle_textinput) != 0;
	}
	bool thinks() const {
		return (flags_ & pf_thinks) != 0;
	}

	void check_child_death();

	void do_draw(RenderTarget &);
	void do_draw_inner(RenderTarget &);
	void do_think();

	Panel * child_at_mouse_cursor
		(int32_t mouse_x, int32_t mouse_y, Panel * child);
	void do_mousein(bool inside);
	bool do_mousepress  (const uint8_t btn, int32_t x, int32_t y);
	bool do_mouserelease(const uint8_t btn, int32_t x, int32_t y);
	bool do_mousemove
		(const uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	bool do_mousewheel(uint32_t which, int32_t x, int32_t y);
	bool do_key(bool down, SDL_Keysym code);
	bool do_textinput(const std::string& text);
	bool do_tooltip();

	static Panel * ui_trackmouse(int32_t & x, int32_t & y);
	static bool ui_mousepress  (const uint8_t button, int32_t x, int32_t y);
	static bool ui_mouserelease(const uint8_t button, int32_t x, int32_t y);
	static bool ui_mousemove
		(const uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	static bool ui_mousewheel(uint32_t which, int32_t x, int32_t y);
	static bool ui_key(bool down, SDL_Keysym code);
	static bool ui_textinput(const std::string& text);

	Panel * parent_;
	Panel * next_, * prev_;
	Panel * first_child_, * last_child_;
	Panel * mousein_child_; //  child panel that the mouse is in
	Panel * focus_; //  keyboard focus

	uint32_t flags_;

	/**
	 * The outer rectangle is defined by (x_, y_, w_, h_)
	 */
	/*@{*/
	int32_t x_, y_;
	int w_, h_;
	/*@}*/
	int lborder_, rborder_, tborder_, bborder_;
	uint8_t border_snap_distance_, panel_snap_distance_;
	int desired_w_, desired_h_;

	bool running_;
	int return_code_;

	std::string tooltip_;
	static Panel * modal_;
	static Panel * mousegrab_;
	static Panel * mousein_;
	static bool allow_user_input_;
	static const Image* default_cursor_;
	static const Image* default_cursor_click_;

	DISALLOW_COPY_AND_ASSIGN(Panel);
};

inline void Panel::set_snap_windows_only_when_overlapping(const bool on) {
	flags_ &= ~pf_snap_windows_only_when_overlapping;
	if (on)
		flags_ |= pf_snap_windows_only_when_overlapping;
}
inline void Panel::set_dock_windows_to_edges(const bool on) {
	flags_ &= ~pf_dock_windows_to_edges;
	if (on)
		flags_ |= pf_dock_windows_to_edges;
}

/**
 * A Panel with a name. Important for scripting
 */
struct NamedPanel : public Panel {
	NamedPanel
		(Panel * const nparent, const std::string & name,
		 int32_t  const nx, int32_t  const ny,
		 int const nw, int const nh,
		 const std::string & tooltip_text = std::string())
		: Panel(nparent, nx, ny, nw, nh, tooltip_text), name_(name)
	{
	}

	const std::string & get_name() const {return name_;}

private:
	std::string name_;
};

}

#endif  // end of include guard: WL_UI_BASIC_PANEL_H
