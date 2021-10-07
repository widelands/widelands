/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include <cstdlib>
#include <memory>

#include <SDL_mouse.h>

#include "base/i18n.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "io/filesystem/layered_filesystem.h"

namespace UI {

/// Width the horizontal border graphics must have.
constexpr int16_t kHorizonalBorderTotalLength = 100;

constexpr int16_t kWindowTitlebarButtonsSize = 18;
constexpr int16_t kWindowTitlebarButtonsSpacing = 1;
// Used for both vertical and horizontal position finetuning
constexpr int16_t kWindowTitlebarButtonsPos =
   (Window::kTopBorderThickness + kWindowTitlebarButtonsSize) / -2;

/// Width to use as the corner. This must be >= kVerticalBorderThickness.
constexpr int16_t kCornerWidth = 20;

constexpr int16_t kHorizontalBorderMiddleLength(kHorizonalBorderTotalLength - 2 * kCornerWidth);

/// Height the vertical border graphics must have
constexpr int16_t kVerticalBorderTotalLength = 100;

/// Height to use as the thingy.
// TODO(Nordfriese): What is this?
constexpr int16_t kVerticalBorderThingyHeight = 20;

constexpr int16_t kVerticalBorderMiddleLength =
   (kVerticalBorderTotalLength - 2 * kVerticalBorderThingyHeight);

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
               WindowStyle s,
               const std::string& name,
               int32_t const x,
               int32_t const y,
               uint32_t const w,
               uint32_t const h,
               const std::string& title)
   : NamedPanel(parent,
                s == WindowStyle::kWui ? PanelStyle::kWui : PanelStyle::kFsMenu,
                name,
                x,
                y,
                w + kVerticalBorderThickness * 2,
                kTopBorderThickness + h + kBottomBorderThickness),
     window_style_(s),
     is_minimal_(false),
     oldh_(kTopBorderThickness + h + kBottomBorderThickness),
     dragging_(false),
     drag_start_win_x_(0),
     drag_start_win_y_(0),
     drag_start_mouse_x_(0),
     drag_start_mouse_y_(0),
     pinned_(false),
     center_panel_(nullptr),
     fastclick_panel_(nullptr),
     button_close_(new Button(
        this,
        "b_close",
        // positions will be set by first call to layout()
        0,
        0,
        kWindowTitlebarButtonsSize,
        kWindowTitlebarButtonsSize,
        s == WindowStyle::kWui ? ButtonStyle::kWuiSecondary : ButtonStyle::kFsMenuSecondary,
        g_image_cache->get(window_style_info().button_close()),
        _("Close"))),
     button_pin_(new Button(
        this,
        "b_pin",
        0,
        0,
        kWindowTitlebarButtonsSize,
        kWindowTitlebarButtonsSize,
        s == WindowStyle::kWui ? ButtonStyle::kWuiSecondary : ButtonStyle::kFsMenuSecondary,
        "",
        "")),
     button_minimize_(new Button(
        this,
        "b_minimize",
        0,
        0,
        kWindowTitlebarButtonsSize,
        kWindowTitlebarButtonsSize,
        s == WindowStyle::kWui ? ButtonStyle::kWuiSecondary : ButtonStyle::kFsMenuSecondary,
        "",
        "")) {
	set_title(title);

	button_close_->sigclicked.connect([this] {
		if (!pinned_) {
			clicked_button_close();
		}
	});
	button_pin_->sigclicked.connect([this] {
		pinned_ = !pinned_;
		update_toolbar_buttons();
	});
	button_minimize_->sigclicked.connect([this] {
		if (is_minimal_) {
			restore();
		} else {
			minimize();
		}
	});
	update_toolbar_buttons();

	set_border(kVerticalBorderThickness, kVerticalBorderThickness, kTopBorderThickness,
	           kBottomBorderThickness);
	set_top_on_click(true);
	set_layout_toplevel(true);
	layout();
	focus();

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& note) { on_resolution_changed_note(note); });
}

inline const WindowStyleInfo& Window::window_style_info() const {
	return g_style_manager->window_style(window_style_);
}
inline const FontStyleInfo& Window::title_style() const {
	return g_style_manager->font_style(window_style_ == WindowStyle::kWui ?
                                         FontStyle::kWuiWindowTitle :
                                         FontStyle::kFsMenuWindowTitle);
}

void Window::update_toolbar_buttons() {
	button_minimize_->set_pic(g_image_cache->get(is_minimal_ ?
                                                   window_style_info().button_unminimize() :
                                                   window_style_info().button_minimize()));
	button_minimize_->set_tooltip(is_minimal_ ? _("Restore") : _("Minimize"));
	button_minimize_->set_visual_state(is_minimal_ ? Button::VisualState::kPermpressed :
                                                    Button::VisualState::kRaised);
	button_pin_->set_pic(g_image_cache->get(pinned_ ? window_style_info().button_unpin() :
                                                     window_style_info().button_pin()));
	button_pin_->set_tooltip(pinned_ ? _("Unpin") : _("Pin"));
	button_pin_->set_visual_state(pinned_ ? Button::VisualState::kPermpressed :
                                           Button::VisualState::kRaised);
	button_close_->set_enabled(!pinned_);
}

void Window::clicked_button_close() {
	die();
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
	assert(!panel || panel->get_parent() == this);

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
 * Update the titlebar buttons' locations, and change the center panel's size
 * so that it fills the window entirely (the latter only if not minimized).
 */
void Window::layout() {
	if (center_panel_ && !is_minimal_) {
		center_panel_->set_pos(Vector2i::zero());
		center_panel_->set_size(get_inner_w(), get_inner_h());
	}
	button_close_->set_pos(Vector2i(
	   get_w() + kWindowTitlebarButtonsPos - kTopBorderThickness, kWindowTitlebarButtonsPos));
	button_pin_->set_pos(Vector2i(kWindowTitlebarButtonsPos, kWindowTitlebarButtonsPos));
	button_minimize_->set_pos(
	   Vector2i(button_pin_->get_x() + button_pin_->get_w() + kWindowTitlebarButtonsSpacing,
	            kWindowTitlebarButtonsPos));
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
		if ((parent->get_inner_w() < get_w()) && (px + get_w() <= parent->get_inner_w() || px >= 0)) {
			px = (parent->get_inner_w() - get_w()) / 2;
		}
		if ((parent->get_inner_h() < get_h()) && (py + get_h() < parent->get_inner_h() || py > 0)) {
			py = 0;
		}

		if (parent->get_inner_w() >= get_w()) {
			if (px < 0) {
				px = 0;
				if (parent->get_dock_windows_to_edges()) {
					px -= kVerticalBorderThickness;
				}
			} else if (px + get_w() >= parent->get_inner_w()) {
				px = parent->get_inner_w() - get_w();
				if (parent->get_dock_windows_to_edges()) {
					px += kVerticalBorderThickness;
				}
			}
		}
		if (parent->get_inner_h() >= get_h()) {
			if (py < 0) {
				py = 0;
			} else if (py + get_h() > parent->get_inner_h()) {
				py = parent->get_inner_h() - get_h();
				if (!is_minimal_ && parent->get_dock_windows_to_edges()) {
					py += kBottomBorderThickness;
				}
			}
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
		dst.tile(Recti(Vector2i::zero(), get_inner_w(), get_inner_h()),
		         window_style_info().background(), Vector2i::zero());
	}
}

/**
 * Redraw the window frame
 */
void Window::draw_border(RenderTarget& dst) {
	static_assert(
	   kCornerWidth >= kVerticalBorderThickness, "kCornerWidth < kVerticalBorderThickness");
	static_assert(kHorizontalBorderMiddleLength > 0, "kHorizontalBorderMiddleLength <= 0");

	const int32_t hz_bar_end = get_w() - kCornerWidth;
	const int32_t hz_bar_end_minus_middle = hz_bar_end - kHorizontalBorderMiddleLength;

	const RGBAColor& focus_color =
	   (get_parent() && get_parent()->focused_child() == this) || is_modal() ?
         window_style_info().window_border_focused() :
         window_style_info().window_border_unfocused();

	{  //  Top border.
		int32_t pos = kCornerWidth;

		dst.blitrect  //  top left corner
		   (Vector2i::zero(), window_style_info().border_top(),
		    Recti(Vector2i::zero(), pos, kTopBorderThickness));

		//  top bar
		static_assert(0 <= kCornerWidth, "assert(0 <= kCornerWidth) failed.");
		for (; pos < hz_bar_end_minus_middle; pos += kHorizontalBorderMiddleLength) {
			dst.blitrect(
			   Vector2i(pos, 0), window_style_info().border_top(),
			   Recti(Vector2i(kCornerWidth, 0), kHorizontalBorderMiddleLength, kTopBorderThickness));
		}

		// odd pixels of top bar and top right corner
		const int32_t width = hz_bar_end - pos + kCornerWidth;
		assert(0 <= kHorizonalBorderTotalLength - width);
		dst.blitrect(
		   Vector2i(pos, 0), window_style_info().border_top(),
		   Recti(Vector2i(kHorizonalBorderTotalLength - width, 0), width, kTopBorderThickness));

		// Focus overlay
		dst.fill_rect(Recti(0, 0, get_w(), kTopBorderThickness), focus_color, BlendMode::Default);
	}

	// draw the title if we have one
	if (!title_.empty()) {
		// The title shouldn't be richtext, but we escape it just to make sure.
		std::shared_ptr<const UI::RenderedText> text =
		   autofit_text(richtext_escape(title_), title_style(), get_inner_w() - kTopBorderThickness);

		Vector2i pos(
		   get_lborder() + (get_inner_w() + kTopBorderThickness) / 2, kTopBorderThickness / 2);
		UI::center_vertically(text->height(), &pos);
		text->draw(dst, pos, UI::Align::kCenter);
	}

	if (!is_minimal_) {
		const int32_t vt_bar_end = get_h() - kBottomBorderThickness - kVerticalBorderThingyHeight;
		const int32_t vt_bar_end_minus_middle = vt_bar_end - kVerticalBorderMiddleLength;

		{  // Left border

			static_assert(
			   0 <= kVerticalBorderThickness, "assert(0 <= kVerticalBorderThickness) failed.");
			dst.blitrect  // left top thingy
			   (Vector2i(0, kTopBorderThickness), window_style_info().border_left(),
			    Recti(Vector2i::zero(), kVerticalBorderThickness, kVerticalBorderThingyHeight));

			int32_t pos = kTopBorderThickness + kVerticalBorderThingyHeight;

			//  left bar
			static_assert(
			   0 <= kVerticalBorderThingyHeight, "assert(0 <= kVerticalBorderThingyHeight) failed.");
			for (; pos < vt_bar_end_minus_middle; pos += kVerticalBorderMiddleLength) {
				dst.blitrect(Vector2i(0, pos), window_style_info().border_left(),
				             Recti(Vector2i(0, kVerticalBorderThingyHeight), kVerticalBorderThickness,
				                   kVerticalBorderMiddleLength));
			}

			//  odd pixels of left bar and left bottom thingy
			const int32_t height = vt_bar_end - pos + kVerticalBorderThingyHeight;
			assert(0 <= kVerticalBorderTotalLength - height);
			dst.blitrect(Vector2i(0, pos), window_style_info().border_left(),
			             Recti(Vector2i(0, kVerticalBorderTotalLength - height),
			                   kVerticalBorderThickness, height));
		}

		{  // Right border
			const int32_t right_border_x = get_w() - kVerticalBorderThickness;

			dst.blitrect  // right top thingy
			   (Vector2i(right_border_x, kTopBorderThickness), window_style_info().border_right(),
			    Recti(Vector2i::zero(), kVerticalBorderThickness, kVerticalBorderThingyHeight));

			int32_t pos = kTopBorderThickness + kVerticalBorderThingyHeight;

			//  right bar
			static_assert(
			   0 <= kVerticalBorderThingyHeight, "assert(0 <= kVerticalBorderThingyHeight) failed.");
			for (; pos < vt_bar_end_minus_middle; pos += kVerticalBorderMiddleLength) {
				dst.blitrect(Vector2i(right_border_x, pos), window_style_info().border_right(),
				             Recti(Vector2i(0, kVerticalBorderThingyHeight), kVerticalBorderThickness,
				                   kVerticalBorderMiddleLength));
			}

			// odd pixels of right bar and right bottom thingy
			const int32_t height = vt_bar_end - pos + kVerticalBorderThingyHeight;
			dst.blitrect(Vector2i(right_border_x, pos), window_style_info().border_right(),
			             Recti(Vector2i(0, kVerticalBorderTotalLength - height),
			                   kVerticalBorderThickness, height));
		}

		{  // Bottom border
			int32_t pos = kCornerWidth;

			dst.blitrect  //  bottom left corner
			   (Vector2i(0, get_h() - kBottomBorderThickness), window_style_info().border_bottom(),
			    Recti(Vector2i::zero(), pos, kBottomBorderThickness));

			//  bottom bar
			for (; pos < hz_bar_end_minus_middle; pos += kHorizontalBorderMiddleLength) {
				dst.blitrect(Vector2i(pos, get_h() - kBottomBorderThickness),
				             window_style_info().border_bottom(),
				             Recti(Vector2i(kCornerWidth, 0), kHorizontalBorderMiddleLength,
				                   kBottomBorderThickness));
			}

			// odd pixels of bottom bar and bottom right corner
			const int32_t width = hz_bar_end - pos + kCornerWidth;
			dst.blitrect(
			   Vector2i(pos, get_h() - kBottomBorderThickness), window_style_info().border_bottom(),
			   Recti(Vector2i(kHorizonalBorderTotalLength - width, 0), width, kBottomBorderThickness));
		}

		// Focus overlays
		// Bottom
		dst.fill_rect(Recti(0, get_h() - kBottomBorderThickness, get_w(), kBottomBorderThickness),
		              focus_color, BlendMode::Default);
		// Left
		dst.fill_rect(Recti(0, kTopBorderThickness, kVerticalBorderThickness,
		                    get_h() - kTopBorderThickness - kBottomBorderThickness),
		              focus_color, BlendMode::Default);
		// Right
		dst.fill_rect(
		   Recti(get_w() - kVerticalBorderThickness, kTopBorderThickness, kVerticalBorderThickness,
		         get_h() - kTopBorderThickness - kBottomBorderThickness),
		   focus_color, BlendMode::Default);
	}

	// draw them again so they aren't hidden by the border
	for (Button* b : {button_close_, button_pin_, button_minimize_}) {
		b->set_pos(Vector2i(b->get_x() + kTopBorderThickness, b->get_y() + kTopBorderThickness));
		b->do_draw(dst);
		b->set_pos(Vector2i(b->get_x() - kTopBorderThickness, b->get_y() - kTopBorderThickness));
	}
}

void Window::think() {
	if (!is_minimal()) {
		Panel::think();
	}
}

/**
 * Left-click: drag the window
 * Right-click: close the window
 */
bool Window::handle_mousepress(const uint8_t btn, int32_t mx, int32_t my) {
	//  TODO(unknown): This code is erroneous. It checks the current key state. What it
	//  needs is the key state at the time the mouse was clicked. See the
	//  usage comment for get_key_state.
	if ((SDL_GetModState() & KMOD_CTRL && btn == SDL_BUTTON_LEFT && my < kVerticalBorderThickness) ||
	    btn == SDL_BUTTON_MIDDLE) {
		is_minimal() ? restore() : minimize();
	} else if (btn == SDL_BUTTON_LEFT) {
		dragging_ = true;
		drag_start_win_x_ = get_x();
		drag_start_win_y_ = get_y();
		drag_start_mouse_x_ = get_x() + get_lborder() + mx;
		drag_start_mouse_y_ = get_y() + get_tborder() + my;
		grab_mouse(true);
	} else if (btn == SDL_BUTTON_RIGHT && !pinned_) {
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

bool Window::handle_mousewheel(int32_t, int32_t, uint16_t) {
	// Mouse wheel events should not propagate to objects below us, so we claim
	// that they have been handled.
	return true;
}

bool Window::handle_key(bool down, SDL_Keysym code) {
	// Handles a key input and event and will close when pressing ESC

	if (down) {
		switch (code.sym) {
		case SDLK_ESCAPE: {
			if (!pinned_) {
				die();
				if (Panel* ch = get_next_sibling()) {
					ch->focus();
				}
				return true;
			}
		} break;
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

Window::~Window() {
	set_visible(false);
}

void Window::restore() {
	assert(is_minimal_);
	is_minimal_ = false;
	set_border(get_lborder(), get_rborder(), get_tborder(), kBottomBorderThickness);
	set_inner_size(get_inner_w(), oldh_);
	update_desired_size();
	move_inside_parent();
	set_handle_keypresses(true);
	update_toolbar_buttons();
}
void Window::minimize() {
	assert(!is_minimal_);
	int32_t y = get_y(), x = get_x();
	if (y < 0) {
		y = 0;  //  Move into the screen
	}
	oldh_ = get_inner_h();
	is_minimal_ = true;
	set_border(get_lborder(), get_rborder(), get_tborder(), 0);
	set_size(get_w(), kTopBorderThickness);
	set_pos(Vector2i(x, y));  // If on border, this feels more natural
	set_handle_keypresses(false);
	update_toolbar_buttons();
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

			if (nearest_snap_distance_x == bsnap) {
				nearest_snap_distance_x = psnap;
			} else {
				assert(nearest_snap_distance_x < bsnap);
				nearest_snap_distance_x = std::min(nearest_snap_distance_x, psnap);
			}
			if (nearest_snap_distance_y == bsnap) {
				nearest_snap_distance_y = psnap;
			} else {
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
				if (new_left <= 0 && new_left >= -kVerticalBorderThickness) {
					new_left = -kVerticalBorderThickness;
				}
				if (new_left >= (max_x - w) && new_left <= (max_x - w) + kVerticalBorderThickness) {
					new_left = (max_x - w) + kVerticalBorderThickness;
				}
				if (!is_minimal_) {  //  minimal windows can not be bottom-docked
					if (new_top >= (max_y - h) && new_top <= (max_y - h) + kBottomBorderThickness) {
						new_top = (max_y - h) + kBottomBorderThickness;
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
