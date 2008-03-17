/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#include "ui_window.h"

#include "constants.h"
#include "font_handler.h"
#include "graphic.h"
#include "rendertarget.h"
#include "wlapplication.h"

#include <SDL_keysym.h>

namespace UI {
/// Width the horizontal border graphics must have.
#define HZ_B_TOTAL_PIXMAP_LEN 100

/// Height the top border must have
#define TP_B_PIXMAP_THICKNESS 20

/// Height the bottom border must have
#define BT_B_PIXMAP_THICKNESS 20

/// Width to use as the corner. This must be >= VT_B_PIXMAP_THICKNESS.
#define HZ_B_CORNER_PIXMAP_LEN 20

#define HZ_B_MIDDLE_PIXMAP_LEN (HZ_B_TOTAL_PIXMAP_LEN - 2 * HZ_B_CORNER_PIXMAP_LEN)

/// Width/height the vertical border graphics must have.
#define VT_B_PIXMAP_THICKNESS 20
#define VT_B_TOTAL_PIXMAP_LEN 100

/// Height to use as the thingy.
#define VT_B_THINGY_PIXMAP_LEN 20

#define VT_B_MIDDLE_PIXMAP_LEN (VT_B_TOTAL_PIXMAP_LEN - 2 * VT_B_THINGY_PIXMAP_LEN)


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
Window::Window(Panel *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *title) :
Panel(parent, x, y, w + VT_B_PIXMAP_THICKNESS * 2, TP_B_PIXMAP_THICKNESS + h + BT_B_PIXMAP_THICKNESS),
_is_minimal(false), _dragging(false),
_docked_left(false), _docked_right(false), _docked_bottom(false),
_drag_start_win_x(0), _drag_start_win_y(0), _drag_start_mouse_x(0), _drag_start_mouse_y(0),
m_pic_lborder   (g_gr->get_picture(PicMod_UI, "pics/win_l_border.png")),
m_pic_rborder   (g_gr->get_picture(PicMod_UI, "pics/win_r_border.png")),
m_pic_top       (g_gr->get_picture(PicMod_UI, "pics/win_top.png")),
m_pic_bottom    (g_gr->get_picture(PicMod_UI, "pics/win_bot.png")),
m_pic_background(g_gr->get_picture(PicMod_UI, "pics/win_bg.png"))
{

	if (title)
		set_title(title);

	set_border(VT_B_PIXMAP_THICKNESS, VT_B_PIXMAP_THICKNESS, TP_B_PIXMAP_THICKNESS, BT_B_PIXMAP_THICKNESS);
	set_cache(true);
	set_top_on_click(true);
}

/**
 * Resource cleanup
 */
Window::~Window() {}


/**
 * Replace the current title with a new one
*/
void Window::set_title(const char *text)
{
	m_title = text;
	update(0, 0, get_w(), TP_B_PIXMAP_THICKNESS);
}

/**
 * Move the window so that it is under the mouse cursor.
*/
void Window::move_to_mouse() {
	set_pos(get_mouse_position() - Point(get_w() / 2, get_h() / 2));
	move_inside_parent();
}


/**
 * Move the window so that it is inside the parent panel.
 * We need this, because move_to_mouse is called before the window is resized.
 * If configured, hang the border off the edge of the panel.
*/
void Window::move_inside_parent() {
	if (Panel * const parent = get_parent()) {
		int32_t px = get_x();
		int32_t py = get_y();
		if (px < 0) {
			px = 0;
			if (parent->get_dock_windows_to_edges() and not _docked_left)
				dock_left();
		} else if (px + static_cast<int32_t>(get_w()) > parent->get_inner_w()) {
			px = parent->get_inner_w() - get_w();
			if (parent->get_dock_windows_to_edges() and not _docked_right)
				dock_right();
		}
		if (py < 0) py = 0;
		else if (py + static_cast<int32_t>(get_h()) > parent->get_inner_h()) {
			py = parent->get_inner_h() - get_h();
			if
				(not _is_minimal
				 and
				 parent->get_dock_windows_to_edges() and not _docked_bottom)
				dock_bottom();
		}
		if      (_docked_left)   px = 0;
		else if (_docked_right)  px = parent->get_inner_w() - get_w();
		if      (_docked_bottom) py = parent->get_inner_h() - get_h();
		set_pos(Point(px, py));
	}
}


/**
 * Move the window so that it is centered wrt the parent.
*/
void Window::center_to_parent()
{
	Panel* parent = get_parent();

	assert(parent);

	set_pos
		(Point
		 ((parent->get_inner_w() - get_w()) / 2,
		  (parent->get_inner_h() - get_h()) / 2));
}


/**
 * Redraw the window frame and background
*/
void Window::draw_border(RenderTarget* dst)
{
	assert(HZ_B_CORNER_PIXMAP_LEN >= VT_B_PIXMAP_THICKNESS);
	assert(HZ_B_MIDDLE_PIXMAP_LEN > 0);

	const uint32_t hidden_width_left  = _docked_left  ? VT_B_PIXMAP_THICKNESS : 0;
	const uint32_t hidden_width_right = _docked_right ? VT_B_PIXMAP_THICKNESS : 0;
	const int32_t hz_bar_end = get_w() - HZ_B_CORNER_PIXMAP_LEN + hidden_width_right;
	const int32_t hz_bar_end_minus_middle = hz_bar_end - HZ_B_MIDDLE_PIXMAP_LEN;

	{//  Top border.
		int32_t pos = HZ_B_CORNER_PIXMAP_LEN - hidden_width_left;

		dst->blitrect //  top left corner
			(Point(0, 0),
			 m_pic_top,
			 Rect(Point(hidden_width_left, 0), pos, TP_B_PIXMAP_THICKNESS));

		//  top bar
		compile_assert(0 <= HZ_B_CORNER_PIXMAP_LEN);
		for (; pos < hz_bar_end_minus_middle; pos += HZ_B_MIDDLE_PIXMAP_LEN)
			dst->blitrect
			(Point(pos, 0),
			 m_pic_top,
			 Rect
			 (Point(HZ_B_CORNER_PIXMAP_LEN, 0),
			  HZ_B_MIDDLE_PIXMAP_LEN, TP_B_PIXMAP_THICKNESS));

		// odd pixels of top bar and top right corner
		const int32_t width = hz_bar_end - pos + HZ_B_CORNER_PIXMAP_LEN;
		assert(0 <= HZ_B_TOTAL_PIXMAP_LEN - width);
		dst->blitrect
			(Point(pos, 0),
			 m_pic_top,
			 Rect
			 (Point(HZ_B_TOTAL_PIXMAP_LEN - width, 0),
			  width - hidden_width_right, TP_B_PIXMAP_THICKNESS));
	}

	// draw the title if we have one
	if (m_title.length())
		g_fh->draw_string
			(*dst,
			 UI_FONT_SMALL, UI_FONT_SMALL_CLR,
			 Point(get_lborder() + get_inner_w() / 2, TP_B_PIXMAP_THICKNESS / 2),
			 m_title.c_str(), Align_Center);

	if (not _is_minimal) {
		const int32_t vt_bar_end = get_h() - (_docked_bottom ? 0 : BT_B_PIXMAP_THICKNESS) - VT_B_THINGY_PIXMAP_LEN;
		const int32_t vt_bar_end_minus_middle = vt_bar_end - VT_B_MIDDLE_PIXMAP_LEN;

		if (not _docked_left) {

			compile_assert(0 <= VT_B_PIXMAP_THICKNESS);
			dst->blitrect // left top thingy
				(Point(0, TP_B_PIXMAP_THICKNESS),
				 m_pic_lborder,
				 Rect(Point(0, 0), VT_B_PIXMAP_THICKNESS, VT_B_THINGY_PIXMAP_LEN));

			int32_t pos = TP_B_PIXMAP_THICKNESS + VT_B_THINGY_PIXMAP_LEN;

			//  left bar
			compile_assert(0 <= VT_B_THINGY_PIXMAP_LEN);
			for (; pos < vt_bar_end_minus_middle; pos += VT_B_MIDDLE_PIXMAP_LEN)
				dst->blitrect
				(Point(0, pos),
				 m_pic_lborder,
				 Rect
				 (Point(0, VT_B_THINGY_PIXMAP_LEN),
				  VT_B_PIXMAP_THICKNESS, VT_B_MIDDLE_PIXMAP_LEN));

			//  odd pixels of left bar and left bottom thingy
			const int32_t height = vt_bar_end - pos + VT_B_THINGY_PIXMAP_LEN;
			assert(0 <= VT_B_TOTAL_PIXMAP_LEN - height);
			dst->blitrect
				(Point(0, pos),
				 m_pic_lborder,
				 Rect
				 (Point(0, VT_B_TOTAL_PIXMAP_LEN - height),
				  VT_B_PIXMAP_THICKNESS, height));
		}


		dst->tile //  background
			(Rect
			 (Point
			  (_docked_left ? 0 : VT_B_PIXMAP_THICKNESS, TP_B_PIXMAP_THICKNESS),
			  get_inner_w(), get_inner_h()),
			 m_pic_background, Point(0, 0));

		if (not _docked_right) {
			const int32_t right_border_x = get_w() - VT_B_PIXMAP_THICKNESS;

			dst->blitrect// right top thingy
				(Point(right_border_x, TP_B_PIXMAP_THICKNESS),
				 m_pic_rborder,
				 Rect(Point(0, 0), VT_B_PIXMAP_THICKNESS, VT_B_THINGY_PIXMAP_LEN));

			int32_t pos = TP_B_PIXMAP_THICKNESS + VT_B_THINGY_PIXMAP_LEN;

			//  right bar
			compile_assert(0 <= VT_B_THINGY_PIXMAP_LEN);
			for (; pos < vt_bar_end_minus_middle; pos += VT_B_MIDDLE_PIXMAP_LEN)
				dst->blitrect
				(Point(right_border_x, pos),
				 m_pic_rborder,
				 Rect
				 (Point(0, VT_B_THINGY_PIXMAP_LEN),
				  VT_B_PIXMAP_THICKNESS, VT_B_MIDDLE_PIXMAP_LEN));

			// odd pixels of right bar and right bottom thingy
			const int32_t height = vt_bar_end - pos + VT_B_THINGY_PIXMAP_LEN;
			dst->blitrect
				(Point(right_border_x, pos),
				 m_pic_rborder,
				 Rect
				 (Point(0, VT_B_TOTAL_PIXMAP_LEN - height),
				  VT_B_PIXMAP_THICKNESS, height));
		}
		if (not _docked_bottom) {
			int32_t pos = HZ_B_CORNER_PIXMAP_LEN - hidden_width_left;

			dst->blitrect //  bottom left corner
				(Point(0, get_h() - BT_B_PIXMAP_THICKNESS),
				 m_pic_bottom,
				 Rect(Point(hidden_width_left, 0), pos, BT_B_PIXMAP_THICKNESS));

			//  bottom bar
			for (; pos < hz_bar_end_minus_middle; pos += HZ_B_MIDDLE_PIXMAP_LEN)
				dst->blitrect
				(Point(pos, get_h() - BT_B_PIXMAP_THICKNESS),
				 m_pic_bottom,
				 Rect
				 (Point(HZ_B_CORNER_PIXMAP_LEN, 0),
				  HZ_B_MIDDLE_PIXMAP_LEN, BT_B_PIXMAP_THICKNESS));

			// odd pixels of bottom bar and bottom right corner
			const int32_t width = hz_bar_end - pos + HZ_B_CORNER_PIXMAP_LEN;
			dst->blitrect
				(Point(pos, get_h() - BT_B_PIXMAP_THICKNESS),
				 m_pic_bottom,
				 Rect
				 (Point(HZ_B_TOTAL_PIXMAP_LEN - width, 0),
				  width - hidden_width_right, BT_B_PIXMAP_THICKNESS));
		}
	}
}


void Window::think() {if (not is_minimal()) Panel::think();}


/**
 * Left-click: drag the window
 * Right-click: close the window
 */
bool Window::handle_mousepress(const Uint8 btn, int32_t mx, int32_t my) {
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
	}
	else if (btn == SDL_BUTTON_RIGHT) {
		play_click();
		delete this; // is this 100% safe?
		//  FIXME No, at least provide a flag for making a window unclosable and
		//  FIXME provide a callback.
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

void Window::restore() {
	assert(_is_minimal);
	set_border
		(get_lborder(), get_rborder(),
		 get_tborder(), _docked_bottom ? 0 : BT_B_PIXMAP_THICKNESS);
	set_inner_size(get_inner_w(), _oldh);
	_is_minimal = false;
	move_inside_parent();
}
void Window::minimize() {
	assert(not _is_minimal);
	if (_docked_bottom) undock_bottom(); //  Minimal can not be bottom-docked.
	_oldh = get_inner_h();
	set_border(get_lborder(), get_rborder(), get_tborder(), 0);
	set_size(get_w(), TP_B_PIXMAP_THICKNESS);
	set_pos(Point(get_x(), get_y())); // If on border, this feels more natural
	_is_minimal = true;
}

inline void Window::dock_left() {
	assert(not _docked_left);
	_docked_left = true;
	set_size(get_inner_w() + get_rborder(), get_h());
	set_border(0, get_rborder(), get_tborder(), get_bborder());
	assert(get_lborder() == 0);
}

inline void Window::undock_left() {
	assert(_docked_left);
	_docked_left = false;
	set_size(VT_B_PIXMAP_THICKNESS + get_inner_w() + get_rborder(), get_h());
	set_border(VT_B_PIXMAP_THICKNESS, get_rborder(), get_tborder(), get_bborder());
}

inline void Window::dock_right() {
	assert(not _docked_right);
	_docked_right = true;
	set_size(get_lborder() + get_inner_w(), get_h());
	set_border(get_lborder(), 0, get_tborder(), get_bborder());
}

inline void Window::undock_right() {
	assert(_docked_right);
	_docked_right = false;
	set_size(get_lborder() + get_inner_w() + VT_B_PIXMAP_THICKNESS, get_h());
	set_border(get_lborder(), VT_B_PIXMAP_THICKNESS, get_tborder(), get_bborder());
}

inline void Window::dock_bottom() {
	assert(not _docked_bottom);
	assert(not _is_minimal); //  Minimal windows can not be bottom-docked.
	_docked_bottom = true;
	set_size(get_w(), get_tborder() + get_inner_h());
	set_border(get_lborder(), get_rborder(), get_tborder(), 0);
}

inline void Window::undock_bottom() {
	assert(_docked_bottom);
	_docked_bottom = false;
	set_size(get_w(), get_tborder() + get_inner_h() + BT_B_PIXMAP_THICKNESS);
	set_border(get_lborder(), get_rborder(), get_tborder(), BT_B_PIXMAP_THICKNESS);
}


/**
 * Drag the mouse if the left mouse button is clicked.
 * Ensure that the window isn't dragged out of the screen.
 */
bool Window::handle_mousemove(const Uint8, int32_t mx, int32_t my, int32_t, int32_t) {
	if (_dragging) {
		const int32_t mouse_x = get_x() + get_lborder() + mx;
		const int32_t mouse_y = get_y() + get_tborder() + my;
		int32_t left = std::max(0, _drag_start_win_x + mouse_x - _drag_start_mouse_x);
		int32_t top  = std::max(0, _drag_start_win_y + mouse_y - _drag_start_mouse_y);
		int32_t new_left = left, new_top = top;

		if (const Panel * const parent = get_parent()) {
			const int32_t w = get_w();
			const int32_t h = get_h();
			const int32_t max_x = parent->get_inner_w();
			const int32_t max_y = parent->get_inner_h();
			assert(w <= max_x); //  These assertions will fail when having too
			assert(h <= max_y); //  low resolution and opening a large window.
			int32_t max_x_minus_w = max_x - w;
			int32_t max_y_minus_h = max_y - h;
			left = std::min(max_x_minus_w, left);
			top  = std::min(max_y_minus_h, top);
			const uint8_t psnap = parent->get_panel_snap_distance ();
			const uint8_t bsnap = parent->get_border_snap_distance();

			//  These are needed to prefer snapping a shorter distance over a
			//  longer distance, when there are several things to snap to.
			uint8_t nearest_snap_distance_x = bsnap;
			uint8_t nearest_snap_distance_y = bsnap;

			//  Snap to parent borders.
			if (left < nearest_snap_distance_x) {
				nearest_snap_distance_x = left;
				assert(nearest_snap_distance_x < bsnap);
				new_left = 0;
			}
			if (left + nearest_snap_distance_x > max_x_minus_w) {
				nearest_snap_distance_x = max_x_minus_w - left;
				assert(nearest_snap_distance_x < bsnap);
				new_left = max_x_minus_w;
			}
			if (top < nearest_snap_distance_y) {
				nearest_snap_distance_y = top;
				assert(nearest_snap_distance_y < bsnap);
				new_top = 0;
			}
			if (top + nearest_snap_distance_y > max_y_minus_h) {
				nearest_snap_distance_y = max_y_minus_h - top;
				assert(nearest_snap_distance_y < bsnap);
				new_top = max_y_minus_h;
			}

			if (nearest_snap_distance_x == bsnap) nearest_snap_distance_x = psnap;
			else {
				assert(nearest_snap_distance_x < bsnap);
				nearest_snap_distance_x = std::min(nearest_snap_distance_x, psnap);
			}
			if (nearest_snap_distance_y == bsnap) nearest_snap_distance_y = psnap;
			else {
				assert(nearest_snap_distance_y < bsnap);
				nearest_snap_distance_y = std::min(nearest_snap_distance_y, psnap);
			}

			{//  Snap to other Panels.
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

			if (new_left < 0)             new_left = 0;
			if (new_top  < 0)             new_top  = 0;
			if (new_left > max_x_minus_w) new_left = max_x_minus_w;
			if (new_top  > max_y_minus_h) new_top  = max_y_minus_h;

			if (parent->get_dock_windows_to_edges()) {
				if (new_left == 0) {
					if (not _docked_left) dock_left();
				} else if (_docked_left) undock_left();
				if (new_left == max_x_minus_w) {
					if (not _docked_right) {
						dock_right();
						new_left += VT_B_PIXMAP_THICKNESS;
						_drag_start_win_x += VT_B_PIXMAP_THICKNESS; //  avoid jumping
					}
				} else if (_docked_right) {
					undock_right();
					new_left -= VT_B_PIXMAP_THICKNESS;
					_drag_start_win_x -= VT_B_PIXMAP_THICKNESS; //  avoid jumping
				}
				if (not _is_minimal) //  Minimal windows can not be bottom-docked.
					if (new_top == max_y_minus_h) {
						if (not _docked_bottom) {
							dock_bottom();
							new_top += BT_B_PIXMAP_THICKNESS;

							//  avoid jumping
							_drag_start_win_y += BT_B_PIXMAP_THICKNESS;
						}
					} else if (_docked_bottom) {
						undock_bottom();
						new_top -= BT_B_PIXMAP_THICKNESS;
						_drag_start_win_y -= BT_B_PIXMAP_THICKNESS; //  avoid jumping
					}
			}
		}
		set_pos(Point(new_left, new_top));
	}
	return true;
}
};
