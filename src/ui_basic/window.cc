/*
 * Copyright (C) 2002, 2006-2011 by the Widelands Development Team
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

#include "ui_basic/window.h"

#include <SDL_keysym.h>

#include "constants.h"
#include "graphic/font.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "log.h"
#include "text_layout.h"
#include "wlapplication.h"


using namespace std;

namespace UI {
/// Width the horizontal border graphics must have.
#define HZ_B_TOTAL_PIXMAP_LEN 100

/// Height the top border must have
#define TP_B_PIXMAP_THICKNESS 20

/// Height the bottom border must have
#define BT_B_PIXMAP_THICKNESS 20

/// Width to use as the corner. This must be >= VT_B_PIXMAP_THICKNESS.
#define HZ_B_CORNER_PIXMAP_LEN 20

#define HZ_B_MIDDLE_PIXMAP_LEN \
   (HZ_B_TOTAL_PIXMAP_LEN - 2 * HZ_B_CORNER_PIXMAP_LEN)

/// Width/height the vertical border graphics must have.
#define VT_B_PIXMAP_THICKNESS 20
#define VT_B_TOTAL_PIXMAP_LEN 100

/// Height to use as the thingy.
#define VT_B_THINGY_PIXMAP_LEN 20

#define VT_B_MIDDLE_PIXMAP_LEN \
   (VT_B_TOTAL_PIXMAP_LEN - 2 * VT_B_THINGY_PIXMAP_LEN)


/**
 * Initialize a framed window.
 *
 * \param parent parent panel
 * \param x x-coordinate of the window relative to the parent (refers to outer
 *          rect!)
 * \param y y-coordinate of the window relative to the parent (refers to outer
 *          rect!)
 * \param w width of the inner rectangle of the window
 * \param h height of the inner rectangle of the window
 * \param title string to display in the window title
 */
Window::Window
	(Panel * const parent,
	 const string & name,
	 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h,
	 const string& title)
	:
		NamedPanel
			(parent, name, x, y, w + VT_B_PIXMAP_THICKNESS * 2,
			 TP_B_PIXMAP_THICKNESS + h + BT_B_PIXMAP_THICKNESS),
		_is_minimal(false), _dragging(false),
		_docked_left(false), _docked_right(false), _docked_bottom(false),
		_drag_start_win_x(0), _drag_start_win_y(0),
		_drag_start_mouse_x(0), _drag_start_mouse_y(0),
		m_pic_lborder
			(g_gr->images().get("pics/win_l_border.png")),
		m_pic_rborder
			(g_gr->images().get("pics/win_r_border.png")),
		m_pic_top
			(g_gr->images().get("pics/win_top.png")),
		m_pic_bottom
			(g_gr->images().get("pics/win_bot.png")),
		m_pic_background
			(g_gr->images().get("pics/win_bg.png")),
		m_center_panel(nullptr),
		m_fastclick_panel(nullptr)
{
	set_title(title);

	set_border
		(VT_B_PIXMAP_THICKNESS, VT_B_PIXMAP_THICKNESS,
		 TP_B_PIXMAP_THICKNESS, BT_B_PIXMAP_THICKNESS);
	set_cache(true);
	set_top_on_click(true);
	set_layout_toplevel(true);
}


/**
 * Replace the current title with a new one
*/
void Window::set_title(const string & text)
{
	m_title = is_richtext(text) ? text : as_window_title(text);
	update(0, 0, get_w(), TP_B_PIXMAP_THICKNESS);
}

/**
 * Set the center panel.
 *
 * The center panel is a child panel that will automatically determine
 * the inner size of the window.
 */
void Window::set_center_panel(Panel * panel)
{
	assert(panel->get_parent() == this);

	m_center_panel = panel;
	update_desired_size();
}

/**
 * Update the window's desired size based on its center panel.
 */
void Window::update_desired_size()
{
	if (m_center_panel) {
		uint32_t innerw, innerh;
		m_center_panel->get_desired_size(innerw, innerh);
		set_desired_size
			(innerw + get_lborder() + get_rborder(),
			 innerh + get_tborder() + get_bborder());
	}
}

/**
 * Change the center panel's size so that it fills the window entirely, but
 * only if not minimized.
 */
void Window::layout()
{
	if (m_center_panel && not _is_minimal) {
		m_center_panel->set_pos(Point(0, 0));
		m_center_panel->set_size(get_inner_w(), get_inner_h());
	}
}

/**
 * Move the window out of the way so that the field bewlow it is visible
 */
void Window::move_out_of_the_way() {
	center_to_parent();

	const Point mouse = get_mouse_position();
	if
		(0 <= mouse.x and mouse.x < get_w()
		 and
		 0 <= mouse.y and mouse.y < get_h())
		{
			set_pos
				(Point(get_x(), get_y())
				 +
				 Point
				 (0, (mouse.y < get_h() / 2 ? 1 : -1)
				  *
				  get_h()));
			move_inside_parent();
		}
}

/**
 * Moves the mouse to the child panel that is activated as fast click panel
 */
void Window::warp_mouse_to_fastclick_panel() {
	if (m_fastclick_panel) {
		 Point pt(m_fastclick_panel->get_w() / 2, m_fastclick_panel->get_h() / 2);
		 UI::Panel * p = m_fastclick_panel;

		while (p->get_parent() && p != this) {
			 pt = p->to_parent(pt);
			 p = p->get_parent();
		}

		set_mouse_pos(pt);
	}
}

/**
 * Move the window so that it is inside the parent panel.
 * If configured, hang the border off the edge of the panel.
*/
void Window::move_inside_parent() {
	if (Panel * const parent = get_parent()) {
		int32_t px = get_x();
		int32_t py = get_y();
		if
			((parent->get_inner_w() < static_cast<uint32_t>(get_w())) and
			(px + get_w() <= static_cast<int32_t>(parent->get_inner_w()) or px >= 0))
			px = (static_cast<int32_t>(parent->get_inner_w()) - get_w()) / 2;
		if
			((parent->get_inner_h() < static_cast<uint32_t>(get_h())) and
			(py + get_h() < static_cast<int32_t>(parent->get_inner_h()) or py > 0))
				py = 0;

		if (parent->get_inner_w() >= static_cast<uint32_t>(get_w())) {
			if (px < 0) {
				px = 0;
				if (parent->get_dock_windows_to_edges() and not _docked_left)
					_docked_left =  true;
			} else if (px + static_cast<uint32_t>(get_w()) >= parent->get_inner_w()) {
				px = static_cast<int32_t>(parent->get_inner_w()) - get_w();
				if (parent->get_dock_windows_to_edges() and not _docked_right)
					_docked_right = true;
			}
			if (_docked_left)
				px -= VT_B_PIXMAP_THICKNESS;
			else if (_docked_right)
				px += VT_B_PIXMAP_THICKNESS;
		}
		if (parent->get_inner_h() >= static_cast<uint32_t>(get_h())) {
			if (py < 0)
				py = 0;
			else if (py + static_cast<uint32_t>(get_h()) > parent->get_inner_h()) {
				py = static_cast<int32_t>(parent->get_inner_h()) - get_h();
				if
					(not _is_minimal
					and
					parent->get_dock_windows_to_edges() and not _docked_bottom)
					_docked_bottom = true;
			}
			if (_docked_bottom)
				py += BT_B_PIXMAP_THICKNESS;
		}
		set_pos(Point(px, py));
	}
}


/**
 * Move the window so that it is centered wrt the parent.
*/
void Window::center_to_parent()
{
	Panel & parent = *get_parent();

	set_pos
		(Point
		((static_cast<int32_t>(parent.get_inner_w()) - get_w()) / 2,
		 (static_cast<int32_t>(parent.get_inner_h()) - get_h()) / 2));
}


/**
 * Redraw the window background.
 */
void Window::draw(RenderTarget & dst)
{
	if (!is_minimal()) {
		dst.tile
			(Rect(Point(0, 0), get_inner_w(), get_inner_h()),
			 m_pic_background, Point(0, 0));
	}
}


/**
 * Redraw the window frame
 */
void Window::draw_border(RenderTarget & dst)
{
	assert(HZ_B_CORNER_PIXMAP_LEN >= VT_B_PIXMAP_THICKNESS);
	assert(HZ_B_MIDDLE_PIXMAP_LEN > 0);

	const int32_t hz_bar_end = get_w() -
		HZ_B_CORNER_PIXMAP_LEN;
	const int32_t hz_bar_end_minus_middle = hz_bar_end -
		HZ_B_MIDDLE_PIXMAP_LEN;

	{ //  Top border.
		int32_t pos = HZ_B_CORNER_PIXMAP_LEN;

		dst.blitrect //  top left corner
			(Point(0, 0),
			 m_pic_top,
			 Rect(Point(0, 0), pos, TP_B_PIXMAP_THICKNESS));

		//  top bar
		static_assert(0 <= HZ_B_CORNER_PIXMAP_LEN, "assert(0 <= HZ_B_CORNER_PIXMAP_LEN) failed.");
		for (; pos < hz_bar_end_minus_middle; pos += HZ_B_MIDDLE_PIXMAP_LEN)
			dst.blitrect
				(Point(pos, 0),
				 m_pic_top,
				 Rect
				 	(Point(HZ_B_CORNER_PIXMAP_LEN, 0),
				 	 HZ_B_MIDDLE_PIXMAP_LEN, TP_B_PIXMAP_THICKNESS));

		// odd pixels of top bar and top right corner
		const int32_t width = hz_bar_end - pos + HZ_B_CORNER_PIXMAP_LEN;
		assert(0 <= HZ_B_TOTAL_PIXMAP_LEN - width);
		dst.blitrect
			(Point(pos, 0),
			 m_pic_top,
			 Rect
			 	(Point(HZ_B_TOTAL_PIXMAP_LEN - width, 0),
			 	 width, TP_B_PIXMAP_THICKNESS));
	}

	// draw the title if we have one
	if (!m_title.empty()) {
		dst.blit
			(Point(get_lborder() + get_inner_w() / 2, TP_B_PIXMAP_THICKNESS / 2),
				UI::g_fh1->render(m_title),
				CM_Normal,
				Align_Center);
	}

	if (not _is_minimal) {
		const int32_t vt_bar_end = get_h() -
			(_docked_bottom ? 0 : BT_B_PIXMAP_THICKNESS) - VT_B_THINGY_PIXMAP_LEN;
		const int32_t vt_bar_end_minus_middle =
			vt_bar_end - VT_B_MIDDLE_PIXMAP_LEN;

		{ // Left border

			static_assert(0 <= VT_B_PIXMAP_THICKNESS, "assert(0 <= VT_B_PIXMAP_THICKNESS) failed.");
			dst.blitrect // left top thingy
				(Point(0, TP_B_PIXMAP_THICKNESS),
				 m_pic_lborder,
				 Rect(Point(0, 0), VT_B_PIXMAP_THICKNESS, VT_B_THINGY_PIXMAP_LEN));

			int32_t pos = TP_B_PIXMAP_THICKNESS + VT_B_THINGY_PIXMAP_LEN;

			//  left bar
			static_assert(0 <= VT_B_THINGY_PIXMAP_LEN, "assert(0 <= VT_B_THINGY_PIXMAP_LEN) failed.");
			for (; pos < vt_bar_end_minus_middle; pos += VT_B_MIDDLE_PIXMAP_LEN)
				dst.blitrect
					(Point(0, pos),
					 m_pic_lborder,
					 Rect
					 	(Point(0, VT_B_THINGY_PIXMAP_LEN),
					 	 VT_B_PIXMAP_THICKNESS, VT_B_MIDDLE_PIXMAP_LEN));

			//  odd pixels of left bar and left bottom thingy
			const int32_t height = vt_bar_end - pos + VT_B_THINGY_PIXMAP_LEN;
			assert(0 <= VT_B_TOTAL_PIXMAP_LEN - height);
			dst.blitrect
				(Point(0, pos),
				 m_pic_lborder,
				 Rect
				 	(Point(0, VT_B_TOTAL_PIXMAP_LEN - height),
				 	 VT_B_PIXMAP_THICKNESS, height));
		}


		{ // Right border
			const int32_t right_border_x = get_w() - VT_B_PIXMAP_THICKNESS;

			dst.blitrect // right top thingy
				(Point(right_border_x, TP_B_PIXMAP_THICKNESS),
				 m_pic_rborder,
				 Rect(Point(0, 0), VT_B_PIXMAP_THICKNESS, VT_B_THINGY_PIXMAP_LEN));

			int32_t pos = TP_B_PIXMAP_THICKNESS + VT_B_THINGY_PIXMAP_LEN;

			//  right bar
			static_assert(0 <= VT_B_THINGY_PIXMAP_LEN, "assert(0 <= VT_B_THINGY_PIXMAP_LEN) failed.");
			for (; pos < vt_bar_end_minus_middle; pos += VT_B_MIDDLE_PIXMAP_LEN)
				dst.blitrect
					(Point(right_border_x, pos),
					 m_pic_rborder,
					 Rect
					 	(Point(0, VT_B_THINGY_PIXMAP_LEN),
					 	 VT_B_PIXMAP_THICKNESS, VT_B_MIDDLE_PIXMAP_LEN));

			// odd pixels of right bar and right bottom thingy
			const int32_t height = vt_bar_end - pos + VT_B_THINGY_PIXMAP_LEN;
			dst.blitrect
				(Point(right_border_x, pos),
				 m_pic_rborder,
				 Rect
				 	(Point(0, VT_B_TOTAL_PIXMAP_LEN - height),
				 	 VT_B_PIXMAP_THICKNESS, height));
		}

		{ // Bottom border
			int32_t pos = HZ_B_CORNER_PIXMAP_LEN;

			dst.blitrect //  bottom left corner
				(Point(0, get_h() - BT_B_PIXMAP_THICKNESS),
				 m_pic_bottom,
				 Rect(Point(0, 0), pos, BT_B_PIXMAP_THICKNESS));

			//  bottom bar
			for (; pos < hz_bar_end_minus_middle; pos += HZ_B_MIDDLE_PIXMAP_LEN)
				dst.blitrect
					(Point(pos, get_h() - BT_B_PIXMAP_THICKNESS),
					 m_pic_bottom,
					 Rect
					 	(Point(HZ_B_CORNER_PIXMAP_LEN, 0),
					 	 HZ_B_MIDDLE_PIXMAP_LEN, BT_B_PIXMAP_THICKNESS));

			// odd pixels of bottom bar and bottom right corner
			const int32_t width = hz_bar_end - pos + HZ_B_CORNER_PIXMAP_LEN;
			dst.blitrect
				(Point(pos, get_h() - BT_B_PIXMAP_THICKNESS),
				 m_pic_bottom,
				 Rect
				 	(Point(HZ_B_TOTAL_PIXMAP_LEN - width, 0),
				 	 width, BT_B_PIXMAP_THICKNESS));
		}
	}
}


void Window::think() {if (not is_minimal()) Panel::think();}


/**
 * Left-click: drag the window
 * Right-click: close the window
 */
bool Window::handle_mousepress(const Uint8 btn, int32_t mx, int32_t my)
{
	//  FIXME This code is erroneous. It checks the current key state. What it
	//  FIXME needs is the key state at the time the mouse was clicked. See the
	//  FIXME usage comment for get_key_state.
	if
		(((get_key_state(SDLK_LCTRL) | get_key_state(SDLK_RCTRL))
		  and
		  btn == SDL_BUTTON_LEFT)
		 or
		 btn == SDL_BUTTON_MIDDLE)
		is_minimal() ? restore() : minimize();
	else if (btn == SDL_BUTTON_LEFT) {
		_dragging = true;
		_drag_start_win_x = get_x();
		_drag_start_win_y = get_y();
		_drag_start_mouse_x = get_x() + get_lborder() + mx;
		_drag_start_mouse_y = get_y() + get_tborder() + my;
		grab_mouse(true);
	} else if (btn == SDL_BUTTON_RIGHT) {
		play_click();
		die();
	}

	return true;
}
bool Window::handle_mouserelease(const Uint8 btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT) {
		grab_mouse(false);
		_dragging = false;
	}
	return true;
}

bool Window::handle_alt_drag(int32_t mx, int32_t my)
{
	_dragging = true;
	_drag_start_win_x = get_x();
	_drag_start_win_y = get_y();
	_drag_start_mouse_x = get_x() + get_lborder() + mx;
	_drag_start_mouse_y = get_y() + get_tborder() + my;
	grab_mouse(true);
	return true;
}

// Always consume the tooltip event to prevent tooltips from
// our parent to be rendered
bool Window::handle_tooltip()
{
	UI::Panel::handle_tooltip();
	return true;
}

/**
 * Close the window. Overwrite this virtual method if you want
 * to take some action before the window is destroyed, or to
 * prevent it
 */
void Window::die()
{
	if (is_modal()) {
		end_modal(0);
	} else {
		Panel::die();
	}
}


void Window::restore() {
	assert(_is_minimal);
	_is_minimal = false;
	set_border
		(get_lborder(), get_rborder(),
		 get_tborder(), BT_B_PIXMAP_THICKNESS);
	set_inner_size(get_inner_w(), _oldh);
	move_inside_parent();
}
void Window::minimize() {
	assert(not _is_minimal);
	int32_t y = get_y(), x = get_x();
	if (_docked_bottom) {
		y -= BT_B_PIXMAP_THICKNESS; //  Minimal can not be bottom-docked.
		_docked_bottom = false;
	}
	if (y < 0) y = 0; //  Move into the screen
	_oldh = get_inner_h();
	_is_minimal = true;
	set_border(get_lborder(), get_rborder(), get_tborder(), 0);
	set_size(get_w(), TP_B_PIXMAP_THICKNESS);
	set_pos(Point(x, y)); // If on border, this feels more natural
}

/**
 * Drag the mouse if the left mouse button is clicked.
 * Ensure that the window isn't fully dragged out of the screen.
 */
bool Window::handle_mousemove
		(const Uint8, int32_t mx, int32_t my, int32_t, int32_t)
{
	if (_dragging) {
		const int32_t mouse_x = get_x() + get_lborder() + mx;
		const int32_t mouse_y = get_y() + get_tborder() + my;
		int32_t left = _drag_start_win_x + mouse_x - _drag_start_mouse_x;
		int32_t top  = _drag_start_win_y + mouse_y - _drag_start_mouse_y;
		int32_t new_left = left, new_top = top;

		if (const Panel * const parent = get_parent()) {
			const int32_t w = get_w();
			const int32_t h = get_h();
			const int32_t max_x = parent->get_inner_w();
			const int32_t max_y = parent->get_inner_h();

			left = min<int32_t>(max_x - get_lborder(), left);
			top  = min<int32_t>(max_y - get_tborder(), top);
			left = max<int32_t>(get_rborder() - w, left);
			top  = max
				(-static_cast<int32_t>(h - ((_is_minimal) ? get_tborder() : get_bborder())), top);
			new_left = left; new_top = top;

			const uint8_t psnap = parent->get_panel_snap_distance ();
			const uint8_t bsnap = parent->get_border_snap_distance();

			const uint32_t pborder_distance_l = abs(left);
			const uint32_t pborder_distance_t = abs(top);
			const uint32_t pborder_distance_r = abs(max_x - (left + w));
			const uint32_t pborder_distance_b = abs(max_y - (top  + h));

			//  These are needed to prefer snapping a shorter distance over a
			//  longer distance, when there are several things to snap to.
			uint8_t nearest_snap_distance_x = bsnap;
			uint8_t nearest_snap_distance_y = bsnap;

			//  Snap to parent borders.
			if (pborder_distance_l < nearest_snap_distance_x) {
				nearest_snap_distance_x = pborder_distance_l;
				assert(nearest_snap_distance_x < bsnap);
				new_left = 0;
			}
			if (pborder_distance_r < nearest_snap_distance_x) {
				nearest_snap_distance_x = pborder_distance_r;
				assert(nearest_snap_distance_x < bsnap);
				new_left = max_x - w;
			}
			if (pborder_distance_t < nearest_snap_distance_y) {
				nearest_snap_distance_y = pborder_distance_t;
				assert(nearest_snap_distance_y < bsnap);
				new_top = 0;
			}
			if (pborder_distance_b < nearest_snap_distance_y) {
				nearest_snap_distance_y = pborder_distance_b;
				assert(nearest_snap_distance_y < bsnap);
				new_top = max_y - h;
			}

			if (nearest_snap_distance_x == bsnap)
				nearest_snap_distance_x = psnap;
			else {
				assert(nearest_snap_distance_x < bsnap);
				nearest_snap_distance_x = min(nearest_snap_distance_x, psnap);
			}
			if (nearest_snap_distance_y == bsnap)
				nearest_snap_distance_y = psnap;
			else {
				assert(nearest_snap_distance_y < bsnap);
				nearest_snap_distance_y = min(nearest_snap_distance_y, psnap);
			}

			{ //  Snap to other Panels.
				const bool SOWO = parent->get_snap_windows_only_when_overlapping();
				const int32_t right = left + w, bot = top + h;

				for
					(const Panel * snap_target = parent->get_first_child();
					 snap_target;
					 snap_target = snap_target->get_next_sibling())
				{
					if (snap_target != this and snap_target->is_snap_target()) {
						int32_t const other_left  = snap_target->get_x();
						int32_t const other_top   = snap_target->get_y();
						int32_t const other_right = other_left + snap_target->get_w();
						int32_t const other_bot   = other_top  + snap_target->get_h();

						if (other_top <= bot && other_bot >= top) {
							if (not SOWO || left <= other_right) {
								const int32_t distance = abs(left - other_right);
								if (distance < nearest_snap_distance_x) {
									nearest_snap_distance_x = distance;
									new_left = other_right;
								}
							}
							if (not SOWO || right >= other_left) {
								const int32_t distance = abs(right - other_left);
								if (distance < nearest_snap_distance_x) {
									nearest_snap_distance_x = distance;
									new_left = other_left - w;
								}
							}
						}
						if (other_left <= right && other_right >= left) {
							if (not SOWO || top <= other_bot) {
								const int32_t distance = abs(top - other_bot);
								if (distance < nearest_snap_distance_y) {
									nearest_snap_distance_y = distance;
									new_top = other_bot;
								}
							}
							if (not SOWO || bot >= other_top) {
								const int32_t distance = abs(bot - other_top);
								if (distance < nearest_snap_distance_y) {
									nearest_snap_distance_y = distance;
									new_top = other_top - h;
								}
							}
						}
					}
				}
			}

			if (parent->get_dock_windows_to_edges()) {
				if (new_left <= 0 and new_left >= -VT_B_PIXMAP_THICKNESS) {
						new_left = -VT_B_PIXMAP_THICKNESS;
						_docked_left = true;
				} else if (_docked_left) {
					_docked_left = false;
				}
				if (new_left >= (max_x - w) and new_left <= (max_x - w) + VT_B_PIXMAP_THICKNESS) {
					new_left = (max_x - w) + VT_B_PIXMAP_THICKNESS;
						_docked_right = true;
				} else if (_docked_right) {
					_docked_right = false;
				}
				if (not _is_minimal) { //  minimal windows can not be bottom-docked
					if (new_top >= (max_y - h) and new_top <= (max_y - h) + BT_B_PIXMAP_THICKNESS) {
						new_top = (max_y - h) + BT_B_PIXMAP_THICKNESS;
							_docked_bottom = true;
					} else if (_docked_bottom) {
						_docked_bottom = false;
					}
				}
			}
		}
		set_pos(Point(new_left, new_top));
	}
	return true;
}

}
