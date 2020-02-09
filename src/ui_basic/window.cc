/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include <SDL_keycode.h>

#include "base/log.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"

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
Window::Window(Panel* const parent,
               const std::string& name,
               int32_t const x,
               int32_t const y,
               uint32_t const w,
               uint32_t const h,
               const std::string& title)
   : NamedPanel(parent,
                name,
                x,
                y,
                w + VT_B_PIXMAP_THICKNESS * 2,
                TP_B_PIXMAP_THICKNESS + h + BT_B_PIXMAP_THICKNESS),
     is_minimal_(false),
     oldh_(TP_B_PIXMAP_THICKNESS + h + BT_B_PIXMAP_THICKNESS),
     dragging_(false),
     docked_left_(false),
     docked_right_(false),
     docked_bottom_(false),
     drag_start_win_x_(0),
     drag_start_win_y_(0),
     drag_start_mouse_x_(0),
     drag_start_mouse_y_(0),
     pic_lborder_(g_gr->images().get(kTemplateDir + "wui/left.png")),
     pic_rborder_(g_gr->images().get(kTemplateDir + "wui/right.png")),
     pic_top_(g_gr->images().get(kTemplateDir + "wui/top.png")),
     pic_bottom_(g_gr->images().get(kTemplateDir + "wui/bottom.png")),
     pic_background_(g_gr->images().get(kTemplateDir + "wui/background.png")),
     center_panel_(nullptr),
     fastclick_panel_(nullptr) {
	set_title(title);

	set_border(
	   VT_B_PIXMAP_THICKNESS, VT_B_PIXMAP_THICKNESS, TP_B_PIXMAP_THICKNESS, BT_B_PIXMAP_THICKNESS);
	set_top_on_click(true);
	set_layout_toplevel(true);
	layout();
	focus();

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& note) { on_resolution_changed_note(note); });
}

/**
 * Replace the current title with a new one
 */
void Window::set_title(const std::string& text) {
	assert(!is_richtext(text));
	title_ = text;
}

/**
 * Set the center panel.
 *
 * The center panel is a child panel that will automatically determine
 * the inner size of the window.
 */
void Window::set_center_panel(Panel* panel) {
	assert(panel->get_parent() == this);

	center_panel_ = panel;
	update_desired_size();
}

/**
 * Update the window's desired size based on its center panel.
 */
void Window::update_desired_size() {
	if (center_panel_ && !is_minimal_) {
		int innerw, innerh = 0;
		center_panel_->get_desired_size(&innerw, &innerh);
		set_desired_size(
		   innerw + get_lborder() + get_rborder(), innerh + get_tborder() + get_bborder());
	}
}

/**
 * Change the center panel's size so that it fills the window entirely, but
 * only if not minimized.
 */
void Window::layout() {
	if (center_panel_ && !is_minimal_) {
		center_panel_->set_pos(Vector2i::zero());
		center_panel_->set_size(get_inner_w(), get_inner_h());
	}
}

/**
 * Move the window out of the way so that the field bewlow it is visible
 */
void Window::move_out_of_the_way() {
	center_to_parent();

	const Vector2i mouse = get_mouse_position();
	if (0 <= mouse.x && mouse.x < get_w() && 0 <= mouse.y && mouse.y < get_h()) {
		set_pos(Vector2i(get_x(), get_y()) + Vector2i(0, (mouse.y < get_h() / 2 ? 1 : -1) * get_h()));
		move_inside_parent();
	}
}

/**
 * Moves the mouse to the child panel that is activated as fast click panel
 */
void Window::warp_mouse_to_fastclick_panel() {
	if (fastclick_panel_) {
		Vector2i pt(fastclick_panel_->get_w() / 2, fastclick_panel_->get_h() / 2);
		UI::Panel* p = fastclick_panel_;

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
	if (Panel* const parent = get_parent()) {
		int32_t px = get_x();
		int32_t py = get_y();
		if ((parent->get_inner_w() < get_w()) && (px + get_w() <= parent->get_inner_w() || px >= 0))
			px = (parent->get_inner_w() - get_w()) / 2;
		if ((parent->get_inner_h() < get_h()) && (py + get_h() < parent->get_inner_h() || py > 0))
			py = 0;

		if (parent->get_inner_w() >= get_w()) {
			if (px < 0) {
				px = 0;
				if (parent->get_dock_windows_to_edges() && !docked_left_)
					docked_left_ = true;
			} else if (px + get_w() >= parent->get_inner_w()) {
				px = parent->get_inner_w() - get_w();
				if (parent->get_dock_windows_to_edges() && !docked_right_)
					docked_right_ = true;
			}
			if (docked_left_)
				px -= VT_B_PIXMAP_THICKNESS;
			else if (docked_right_)
				px += VT_B_PIXMAP_THICKNESS;
		}
		if (parent->get_inner_h() >= get_h()) {
			if (py < 0)
				py = 0;
			else if (py + get_h() > parent->get_inner_h()) {
				py = parent->get_inner_h() - get_h();
				if (!is_minimal_ && parent->get_dock_windows_to_edges() && !docked_bottom_)
					docked_bottom_ = true;
			}
			if (docked_bottom_)
				py += BT_B_PIXMAP_THICKNESS;
		}
		set_pos(Vector2i(px, py));
	}
}

/**
 * Move the window so that it is centered inside the parent.
 *
 * Do nothing if window has no parent.
 */
void Window::center_to_parent() {
	Panel* parent = get_parent();

	assert(parent);
	set_pos(Vector2i((static_cast<int32_t>(parent->get_inner_w()) - get_w()) / 2,
	                 (static_cast<int32_t>(parent->get_inner_h()) - get_h()) / 2));
}

/**
 * Redraw the window background.
 */
void Window::draw(RenderTarget& dst) {
	if (!is_minimal()) {
		dst.tile(
		   Recti(Vector2i::zero(), get_inner_w(), get_inner_h()), pic_background_, Vector2i::zero());
	}
}

/**
 * Redraw the window frame
 */
void Window::draw_border(RenderTarget& dst) {
	static_assert(HZ_B_CORNER_PIXMAP_LEN >= VT_B_PIXMAP_THICKNESS,
	              "HZ_B_CORNER_PIXMAP_LEN < VT_B_PIXMAP_THICKNESS");
	static_assert(HZ_B_MIDDLE_PIXMAP_LEN > 0, "HZ_B_MIDDLE_PIXMAP_LEN <= 0");

	const int32_t hz_bar_end = get_w() - HZ_B_CORNER_PIXMAP_LEN;
	const int32_t hz_bar_end_minus_middle = hz_bar_end - HZ_B_MIDDLE_PIXMAP_LEN;

	{  //  Top border.
		int32_t pos = HZ_B_CORNER_PIXMAP_LEN;

		dst.blitrect  //  top left corner
		   (Vector2i::zero(), pic_top_, Recti(Vector2i::zero(), pos, TP_B_PIXMAP_THICKNESS));

		//  top bar
		static_assert(0 <= HZ_B_CORNER_PIXMAP_LEN, "assert(0 <= HZ_B_CORNER_PIXMAP_LEN) failed.");
		for (; pos < hz_bar_end_minus_middle; pos += HZ_B_MIDDLE_PIXMAP_LEN)
			dst.blitrect(
			   Vector2i(pos, 0), pic_top_, Recti(Vector2i(HZ_B_CORNER_PIXMAP_LEN, 0),
			                                     HZ_B_MIDDLE_PIXMAP_LEN, TP_B_PIXMAP_THICKNESS));

		// odd pixels of top bar and top right corner
		const int32_t width = hz_bar_end - pos + HZ_B_CORNER_PIXMAP_LEN;
		assert(0 <= HZ_B_TOTAL_PIXMAP_LEN - width);
		dst.blitrect(Vector2i(pos, 0), pic_top_,
		             Recti(Vector2i(HZ_B_TOTAL_PIXMAP_LEN - width, 0), width, TP_B_PIXMAP_THICKNESS));
	}

	// draw the title if we have one
	if (!title_.empty()) {
		// The title shouldn't be richtext, but we escape it just to make sure.
		std::shared_ptr<const UI::RenderedText> text =
		   autofit_text(richtext_escape(title_),
		                g_gr->styles().font_style(UI::FontStyle::kWuiWindowTitle), get_inner_w());

		// Blit on pixel boundary (not float), so that the text is blitted pixel perfect.
		Vector2i pos(get_lborder() + get_inner_w() / 2, TP_B_PIXMAP_THICKNESS / 2);
		UI::center_vertically(text->height(), &pos);
		text->draw(dst, pos, UI::Align::kCenter);
	}

	if (!is_minimal_) {
		const int32_t vt_bar_end =
		   get_h() - (docked_bottom_ ? 0 : BT_B_PIXMAP_THICKNESS) - VT_B_THINGY_PIXMAP_LEN;
		const int32_t vt_bar_end_minus_middle = vt_bar_end - VT_B_MIDDLE_PIXMAP_LEN;

		{  // Left border

			static_assert(0 <= VT_B_PIXMAP_THICKNESS, "assert(0 <= VT_B_PIXMAP_THICKNESS) failed.");
			dst.blitrect  // left top thingy
			   (Vector2i(0, TP_B_PIXMAP_THICKNESS), pic_lborder_,
			    Recti(Vector2i::zero(), VT_B_PIXMAP_THICKNESS, VT_B_THINGY_PIXMAP_LEN));

			int32_t pos = TP_B_PIXMAP_THICKNESS + VT_B_THINGY_PIXMAP_LEN;

			//  left bar
			static_assert(0 <= VT_B_THINGY_PIXMAP_LEN, "assert(0 <= VT_B_THINGY_PIXMAP_LEN) failed.");
			for (; pos < vt_bar_end_minus_middle; pos += VT_B_MIDDLE_PIXMAP_LEN)
				dst.blitrect(Vector2i(0, pos), pic_lborder_,
				             Recti(Vector2i(0, VT_B_THINGY_PIXMAP_LEN), VT_B_PIXMAP_THICKNESS,
				                   VT_B_MIDDLE_PIXMAP_LEN));

			//  odd pixels of left bar and left bottom thingy
			const int32_t height = vt_bar_end - pos + VT_B_THINGY_PIXMAP_LEN;
			assert(0 <= VT_B_TOTAL_PIXMAP_LEN - height);
			dst.blitrect(
			   Vector2i(0, pos), pic_lborder_,
			   Recti(Vector2i(0, VT_B_TOTAL_PIXMAP_LEN - height), VT_B_PIXMAP_THICKNESS, height));
		}

		{  // Right border
			const int32_t right_border_x = get_w() - VT_B_PIXMAP_THICKNESS;

			dst.blitrect  // right top thingy
			   (Vector2i(right_border_x, TP_B_PIXMAP_THICKNESS), pic_rborder_,
			    Recti(Vector2i::zero(), VT_B_PIXMAP_THICKNESS, VT_B_THINGY_PIXMAP_LEN));

			int32_t pos = TP_B_PIXMAP_THICKNESS + VT_B_THINGY_PIXMAP_LEN;

			//  right bar
			static_assert(0 <= VT_B_THINGY_PIXMAP_LEN, "assert(0 <= VT_B_THINGY_PIXMAP_LEN) failed.");
			for (; pos < vt_bar_end_minus_middle; pos += VT_B_MIDDLE_PIXMAP_LEN)
				dst.blitrect(Vector2i(right_border_x, pos), pic_rborder_,
				             Recti(Vector2i(0, VT_B_THINGY_PIXMAP_LEN), VT_B_PIXMAP_THICKNESS,
				                   VT_B_MIDDLE_PIXMAP_LEN));

			// odd pixels of right bar and right bottom thingy
			const int32_t height = vt_bar_end - pos + VT_B_THINGY_PIXMAP_LEN;
			dst.blitrect(
			   Vector2i(right_border_x, pos), pic_rborder_,
			   Recti(Vector2i(0, VT_B_TOTAL_PIXMAP_LEN - height), VT_B_PIXMAP_THICKNESS, height));
		}

		{  // Bottom border
			int32_t pos = HZ_B_CORNER_PIXMAP_LEN;

			dst.blitrect  //  bottom left corner
			   (Vector2i(0, get_h() - BT_B_PIXMAP_THICKNESS), pic_bottom_,
			    Recti(Vector2i::zero(), pos, BT_B_PIXMAP_THICKNESS));

			//  bottom bar
			for (; pos < hz_bar_end_minus_middle; pos += HZ_B_MIDDLE_PIXMAP_LEN)
				dst.blitrect(Vector2i(pos, get_h() - BT_B_PIXMAP_THICKNESS), pic_bottom_,
				             Recti(Vector2i(HZ_B_CORNER_PIXMAP_LEN, 0), HZ_B_MIDDLE_PIXMAP_LEN,
				                   BT_B_PIXMAP_THICKNESS));

			// odd pixels of bottom bar and bottom right corner
			const int32_t width = hz_bar_end - pos + HZ_B_CORNER_PIXMAP_LEN;
			dst.blitrect(
			   Vector2i(pos, get_h() - BT_B_PIXMAP_THICKNESS), pic_bottom_,
			   Recti(Vector2i(HZ_B_TOTAL_PIXMAP_LEN - width, 0), width, BT_B_PIXMAP_THICKNESS));
		}
	}
}

void Window::think() {
	if (!is_minimal())
		Panel::think();
}

/**
 * Left-click: drag the window
 * Right-click: close the window
 */
bool Window::handle_mousepress(const uint8_t btn, int32_t mx, int32_t my) {
	//  TODO(unknown): This code is erroneous. It checks the current key state. What it
	//  needs is the key state at the time the mouse was clicked. See the
	//  usage comment for get_key_state.
	if ((SDL_GetModState() & KMOD_CTRL && btn == SDL_BUTTON_LEFT && my < VT_B_PIXMAP_THICKNESS) ||
	    btn == SDL_BUTTON_MIDDLE)
		is_minimal() ? restore() : minimize();
	else if (btn == SDL_BUTTON_LEFT) {
		dragging_ = true;
		drag_start_win_x_ = get_x();
		drag_start_win_y_ = get_y();
		drag_start_mouse_x_ = get_x() + get_lborder() + mx;
		drag_start_mouse_y_ = get_y() + get_tborder() + my;
		grab_mouse(true);
	} else if (btn == SDL_BUTTON_RIGHT) {
		play_click();
		die();
	}

	return true;
}
bool Window::handle_mouserelease(const uint8_t btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT) {
		grab_mouse(false);
		dragging_ = false;
	}
	return false;
}

// Always consume the tooltip event to prevent tooltips from
// our parent to be rendered
bool Window::handle_tooltip() {
	UI::Panel::handle_tooltip();
	return true;
}

bool Window::handle_mousewheel(uint32_t, int32_t, int32_t) {
	// Mouse wheel events should not propagate to objects below us, so we claim
	// that they have been handled.
	return true;
}

bool Window::handle_key(bool down, SDL_Keysym code) {
	// Handles a key input and event and will close when pressing ESC

	if (down) {
		switch (code.sym) {
		case SDLK_ESCAPE: {
			die();
			Panel* ch = get_next_sibling();
			if (ch != nullptr)
				ch->focus();
			return true;
		}
		default:
			break;
		}
	}
	return UI::Panel::handle_key(down, code);
}

/**
 * Close the window. Overwrite this virtual method if you want
 * to take some action before the window is destroyed, or to
 * prevent it
 */
void Window::die() {
	if (is_modal()) {
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	} else {
		Panel::die();
	}
}

void Window::restore() {
	assert(is_minimal_);
	is_minimal_ = false;
	set_border(get_lborder(), get_rborder(), get_tborder(), BT_B_PIXMAP_THICKNESS);
	set_inner_size(get_inner_w(), oldh_);
	update_desired_size();
	move_inside_parent();
	set_handle_keypresses(true);
}
void Window::minimize() {
	assert(!is_minimal_);
	int32_t y = get_y(), x = get_x();
	if (docked_bottom_) {
		y -= BT_B_PIXMAP_THICKNESS;  //  Minimal can not be bottom-docked.
		docked_bottom_ = false;
	}
	if (y < 0)
		y = 0;  //  Move into the screen
	oldh_ = get_inner_h();
	is_minimal_ = true;
	set_border(get_lborder(), get_rborder(), get_tborder(), 0);
	set_size(get_w(), TP_B_PIXMAP_THICKNESS);
	set_pos(Vector2i(x, y));  // If on border, this feels more natural
	set_handle_keypresses(false);
}

/**
 * Drag the mouse if the left mouse button is clicked.
 * Ensure that the window isn't fully dragged out of the screen.
 */
bool Window::handle_mousemove(const uint8_t, int32_t mx, int32_t my, int32_t, int32_t) {
	if (dragging_) {
		const int32_t mouse_x = get_x() + get_lborder() + mx;
		const int32_t mouse_y = get_y() + get_tborder() + my;
		int32_t left = drag_start_win_x_ + mouse_x - drag_start_mouse_x_;
		int32_t top = drag_start_win_y_ + mouse_y - drag_start_mouse_y_;
		int32_t new_left = left, new_top = top;

		if (const Panel* const parent = get_parent()) {
			const int32_t w = get_w();
			const int32_t h = get_h();
			const int32_t max_x = parent->get_inner_w();
			const int32_t max_y = parent->get_inner_h();

			left = std::min<int32_t>(max_x - get_lborder(), left);
			top = std::min<int32_t>(max_y - get_tborder(), top);
			left = std::max<int32_t>(get_rborder() - w, left);
			top = std::max(
			   -static_cast<int32_t>(h - ((is_minimal_) ? get_tborder() : get_bborder())), top);
			new_left = left;
			new_top = top;

			const uint8_t psnap = parent->get_panel_snap_distance();
			const uint8_t bsnap = parent->get_border_snap_distance();

			const uint32_t pborder_distance_l = abs(left);
			const uint32_t pborder_distance_t = abs(top);
			const uint32_t pborder_distance_r = abs(max_x - (left + w));
			const uint32_t pborder_distance_b = abs(max_y - (top + h));

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
				nearest_snap_distance_x = std::min(nearest_snap_distance_x, psnap);
			}
			if (nearest_snap_distance_y == bsnap)
				nearest_snap_distance_y = psnap;
			else {
				assert(nearest_snap_distance_y < bsnap);
				nearest_snap_distance_y = std::min(nearest_snap_distance_y, psnap);
			}

			{  //  Snap to other Panels.
				const bool SOWO = parent->get_snap_windows_only_when_overlapping();
				const int32_t right = left + w, bot = top + h;

				for (const Panel* snap_target = parent->get_first_child(); snap_target;
				     snap_target = snap_target->get_next_sibling()) {
					if (snap_target != this && snap_target->is_snap_target()) {
						int32_t const other_left = snap_target->get_x();
						int32_t const other_top = snap_target->get_y();
						int32_t const other_right = other_left + snap_target->get_w();
						int32_t const other_bot = other_top + snap_target->get_h();

						if (other_top <= bot && other_bot >= top) {
							if (!SOWO || left <= other_right) {
								const int32_t distance = abs(left - other_right);
								if (distance < nearest_snap_distance_x) {
									nearest_snap_distance_x = distance;
									new_left = other_right;
								}
							}
							if (!SOWO || right >= other_left) {
								const int32_t distance = abs(right - other_left);
								if (distance < nearest_snap_distance_x) {
									nearest_snap_distance_x = distance;
									new_left = other_left - w;
								}
							}
						}
						if (other_left <= right && other_right >= left) {
							if (!SOWO || top <= other_bot) {
								const int32_t distance = abs(top - other_bot);
								if (distance < nearest_snap_distance_y) {
									nearest_snap_distance_y = distance;
									new_top = other_bot;
								}
							}
							if (!SOWO || bot >= other_top) {
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
				if (new_left <= 0 && new_left >= -VT_B_PIXMAP_THICKNESS) {
					new_left = -VT_B_PIXMAP_THICKNESS;
					docked_left_ = true;
				} else if (docked_left_) {
					docked_left_ = false;
				}
				if (new_left >= (max_x - w) && new_left <= (max_x - w) + VT_B_PIXMAP_THICKNESS) {
					new_left = (max_x - w) + VT_B_PIXMAP_THICKNESS;
					docked_right_ = true;
				} else if (docked_right_) {
					docked_right_ = false;
				}
				if (!is_minimal_) {  //  minimal windows can not be bottom-docked
					if (new_top >= (max_y - h) && new_top <= (max_y - h) + BT_B_PIXMAP_THICKNESS) {
						new_top = (max_y - h) + BT_B_PIXMAP_THICKNESS;
						docked_bottom_ = true;
					} else if (docked_bottom_) {
						docked_bottom_ = false;
					}
				}
			}
		}
		set_pos(Vector2i(new_left, new_top));
	}
	return true;
}

void Window::on_resolution_changed_note(const GraphicResolutionChanged& note) {
	const int old_center_x = note.old_width / 2;
	const int old_center_y = note.old_height / 2;
	constexpr int kEdgeTolerance = 50;
	constexpr int kCenterTolerance = 10;
	if (std::abs(get_w() - note.old_width) < kEdgeTolerance &&
	    std::abs(get_h() - note.old_height) < kEdgeTolerance) {
		// The window is sort-of fullscreen, e.g. help. So, we resize it.
		set_size(note.new_width, note.new_height);
		center_to_parent();
		layout();
	} else {
		// Adjust x position
		if (std::abs(old_center_x - get_x() - get_w() / 2) < kCenterTolerance) {
			// The window was centered horizontally. Keep it that way.
			set_pos(Vector2i((note.new_width - get_w()) / 2, get_y()));
		} else if (get_x() + get_w() / 2 > old_center_x) {
			// The window was in the right half of the screen. Shift to maintain distance to right edge
			// of the screen.
			set_pos(Vector2i(note.new_width - note.old_width + get_x(), get_y()));
		}
		// Adjust y position
		if (std::abs(old_center_y - get_y() - get_h() / 2) < kCenterTolerance) {
			// The window was centered vertically. Keep it that way.
			set_pos(Vector2i(get_x(), (note.new_height - get_h()) / 2));
		} else if (get_y() + get_h() / 2 > old_center_y) {
			// The window was in the bottom half of the screen. Shift to maintain distance to bottom
			// edge of the screen.
			set_pos(Vector2i(get_x(), note.new_height - note.old_height + get_y()));
		}
	}
}
}  // namespace UI
