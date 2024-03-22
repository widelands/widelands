/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
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
#include "ui_basic/toolbar_setup.h"

namespace UI {

constexpr int16_t kWindowTitlebarButtonsSize = 18;
constexpr int16_t kWindowTitlebarButtonsSpacing = 1;

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
   : Panel(parent,
           s == WindowStyle::kWui ? PanelStyle::kWui : PanelStyle::kFsMenu,
           name,
           x,
           y,
           w + g_style_manager->window_style(s).left_border_thickness() + g_style_manager->window_style(s).right_border_thickness(),
           h + g_style_manager->window_style(s).top_border_thickness() + g_style_manager->window_style(s).bottom_border_thickness()),
     window_style_(s),

     oldh_(h + g_style_manager->window_style(s).top_border_thickness() + g_style_manager->window_style(s).bottom_border_thickness()),

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

	set_border(window_style_info().left_border_thickness(), window_style_info().right_border_thickness(), window_style_info().top_border_thickness(), window_style_info().bottom_border_thickness());
	set_top_on_click(true);
	set_layout_toplevel(true);
	set_snap_target(true);
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
	if ((center_panel_ != nullptr) && !is_minimal_) {
		int innerw;
		int innerh = 0;
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
	if ((center_panel_ != nullptr) && !is_minimal_) {
		center_panel_->set_pos(Vector2i::zero());
		center_panel_->set_size(get_inner_w(), get_inner_h());
	}

	const int16_t buttons_pos = (get_tborder() + kWindowTitlebarButtonsSize) / -2;

	button_close_->set_pos(Vector2i(
	   get_w() + buttons_pos - get_tborder(), buttons_pos));
	button_pin_->set_pos(Vector2i(buttons_pos, buttons_pos));
	button_minimize_->set_pos(
	   Vector2i(button_pin_->get_x() + button_pin_->get_w() + kWindowTitlebarButtonsSpacing,
	            buttons_pos));
}

/**
 * Position the window near the clicked position, but keeping the clicked field visible
 */
void Window::move_out_of_the_way() {
	constexpr int32_t kClearance = 100;

	const Panel* parent = get_parent();
	assert(parent != nullptr);

	// Actually this could be asserted to be true, because only field action-, building- and
	// ship windows call this function currently.
	const bool parent_is_main = parent->get_parent() == nullptr;

	const bool toolbar_at_bottom = main_toolbar_at_bottom();
	const int32_t toolbar_bottom_h =
	   (parent_is_main && toolbar_at_bottom) ? main_toolbar_button_size() : 0;
	const int32_t toolbar_top_h =
	   (parent_is_main && !toolbar_at_bottom) ? main_toolbar_button_size() : 0;

	// We have to do this because InfoPanel::think() pushes child windows off the toolbar, which
	// messes up the fastclick position.
	// We only care about the toolbar at the bottom because we prefer moving the window below
	// the mouse pointer, so it never covers the toolbar at the top, but it may cover it at the
	// bottom.
	const int ph = parent->get_inner_h() - toolbar_bottom_h;
	const int pw = parent->get_inner_w();

	// Pop up messages have higher priority, so they can steal mouse clicks. We try to avoid that.
	const int max_popup_h = UI::kMaxPopupMessages * (toolbar_bottom_h + toolbar_top_h);

	const Vector2i mouse = parent->get_mouse_position();

	int32_t nx = mouse.x;
	int32_t ny = mouse.y;

	// TODO(tothxa): This will have to be calculated when the field action window is converted to
	//               use styles instead of constants
	constexpr int kFlagActionHeight = 112;

	if (get_w() < get_h() && get_h() > kFlagActionHeight) {
		const bool fits_right = mouse.x + kClearance + get_w() < pw;
		const bool fits_left = mouse.x - kClearance - get_w() >= 0;

		bool to_right = fits_right;

		if (!fits_right && !fits_left) {
			to_right = mouse.x < pw / 2;
		}

		if (fits_right && fits_left) {
			bool need_check_popups = false;
			if (parent_is_main) {
				if (toolbar_at_bottom) {
					need_check_popups =
					   mouse.y + get_h() / 2 - get_bborder() > ph - max_popup_h;
				} else {
					need_check_popups =
					   mouse.y - get_h() / 2 + get_tborder() < toolbar_top_h + max_popup_h;
				}
			}

			if (need_check_popups) {
				// This is much simplified, because there may be no perfect solution for some tall
				// windows anyway
				to_right = mouse.x >= pw / 2;
			}
		}

		if (to_right) {
			nx += kClearance;
		} else {
			nx -= get_w() + kClearance;
		}
		ny -= get_h() / 2;
	} else {
		const int max_popup_h_bottom =
		   toolbar_bottom_h > 0 ? (max_popup_h - get_bborder()) : 0;
		if (mouse.y + kClearance + get_h() + max_popup_h_bottom < ph || mouse.y < ph / 2) {
			ny += kClearance;
		} else {
			ny -= get_h() + kClearance;
		}
		nx -= get_w() / 2;
	}

	// Don't use overridden functions in UniqueWindow, position is not final yet
	Panel::set_pos(Vector2i(nx, ny));
	Window::move_inside_parent();
	// move_inside_parent() always calls overridden set_pos(), so position is always finalised
	// for UniqueWindow
}

/**
 * Moves the mouse to the child panel that is activated as fast click panel
 */
void Window::warp_mouse_to_fastclick_panel() {
	if (fastclick_panel_ != nullptr && Panel::allow_fastclick()) {
		Vector2i pt(fastclick_panel_->get_w() / 2, fastclick_panel_->get_h() / 2);
		UI::Panel* p = fastclick_panel_;

		while ((p->get_parent() != nullptr) && p != this) {
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
		const bool parent_is_main = parent->get_parent() == nullptr;
		const bool toolbar_at_bottom = main_toolbar_at_bottom();
		const int32_t toolbar_bottom_h =
		   (parent_is_main && toolbar_at_bottom) ? main_toolbar_button_size() : 0;
		const int32_t toolbar_top_h =
		   (parent_is_main && !toolbar_at_bottom) ? main_toolbar_button_size() : 0;
		const int32_t ph = parent->get_inner_h() - toolbar_top_h - toolbar_bottom_h;

		int32_t px = get_x();
		int32_t py = get_y();

		if (parent->get_inner_w() < get_w()) {
			if ((px + get_w() < parent->get_inner_w() || px > 0)) {
				px = (parent->get_inner_w() - get_w()) / 2;
			}
		} else {
			if (px < 0) {
				px = 0;
				if (parent->get_dock_windows_to_edges()) {
					px -= get_lborder();
				}
			} else if (px + get_w() >= parent->get_inner_w()) {
				px = parent->get_inner_w() - get_w();
				if (parent->get_dock_windows_to_edges()) {
					px += get_rborder();
				}
			}
		}

		if (ph < get_h()) {
			if (parent_is_main) {
				// The toolbar would push it off anyway
				py = toolbar_at_bottom ? ph - get_h() : toolbar_top_h;
			} else if (py + get_h() < ph || py > 0) {
				py = 0;
			}
		} else {
			if (py < toolbar_top_h) {
				py = toolbar_top_h;
			} else if (const int32_t p_bottom = parent->get_inner_h() - toolbar_bottom_h;
			           py + get_h() > p_bottom) {
				py = p_bottom - get_h();
				if (!is_minimal_ && toolbar_bottom_h == 0 && parent->get_dock_windows_to_edges()) {
					py += get_bborder();
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
	const RGBAColor& focus_color =
	   ((get_parent() != nullptr) && get_parent()->focused_child() == this) || is_modal() ?
         window_style_info().window_border_focused() :
         window_style_info().window_border_unfocused();

	{  // Top border
		const int img_len_total = window_style_info().border_top()->width();
		const int corner_width = img_len_total > 2 * get_tborder() ? get_tborder() : 0;
		const int border_x1 = corner_width;
		const int border_x2 = get_w() - corner_width;
		const int img_len_inner = img_len_total - 2 * corner_width;

		// Left corner
		dst.blitrect(Vector2i::zero(), window_style_info().border_top(), Recti(Vector2i::zero(), border_x1, corner_width));

		// Middle
		int pos = border_x1;
		for (; pos + img_len_inner < border_x2; pos += img_len_inner) {
			dst.blitrect(Vector2i(pos, 0), window_style_info().border_top(), Recti(Vector2i(corner_width, 0), img_len_inner, get_tborder()));
		}

		// Trailing pixels at the right
		dst.blitrect(Vector2i(pos, 0), window_style_info().border_top(), Recti(Vector2i(corner_width, 0), border_x2 - pos, get_tborder()));

		// Right corner
		dst.blitrect(Vector2i(border_x2, 0), window_style_info().border_top(), Recti(Vector2i(img_len_total - corner_width, 0), corner_width, get_tborder()));
	}

	if (!is_minimal()) {
		{  // Bottom border
			const int img_len_total = window_style_info().border_bottom()->width();
			const int corner_width = img_len_total > 2 * get_bborder() ? get_bborder() : 0;
			const int border_y = get_h() - get_bborder();
			const int border_x1 = corner_width;
			const int border_x2 = get_w() - corner_width;
			const int img_len_inner = img_len_total - 2 * corner_width;

			// Left corner
			dst.blitrect(Vector2i(0, border_y), window_style_info().border_bottom(), Recti(Vector2i::zero(), border_x1, corner_width));

			// Middle
			int pos = border_x1;
			for (; pos + img_len_inner < border_x2; pos += img_len_inner) {
				dst.blitrect(Vector2i(pos, border_y), window_style_info().border_bottom(), Recti(Vector2i(corner_width, 0), img_len_inner, get_bborder()));
			}

			// Trailing pixels at the right
			dst.blitrect(Vector2i(pos, border_y), window_style_info().border_bottom(), Recti(Vector2i(corner_width, 0), border_x2 - pos, get_bborder()));

			// Right corner
			dst.blitrect(Vector2i(border_x2, border_y), window_style_info().border_bottom(), Recti(Vector2i(img_len_total - corner_width, 0), corner_width, get_bborder()));
		}

		{  // Left border
			const int img_len = window_style_info().border_left()->height();
			const int border_y1 = get_tborder();
			const int border_y2 = get_h() - get_bborder();

			// Middle
			int pos = border_y1;
			for (; pos + img_len < border_y2; pos += img_len) {
				dst.blitrect(Vector2i(0, pos), window_style_info().border_left(), Recti(Vector2i::zero(), get_lborder(), img_len));
			}

			// Trailing pixels at the bottom
			dst.blitrect(Vector2i(0, pos), window_style_info().border_left(), Recti(Vector2i::zero(), get_lborder(), border_y2 - pos));
		}

		{  // Right border
			const int img_len = window_style_info().border_right()->height();
			const int border_x = get_w() - get_rborder();
			const int border_y1 = get_tborder();
			const int border_y2 = get_h() - get_bborder();

			// Middle
			int pos = border_y1;
			for (; pos + img_len < border_y2; pos += img_len) {
				dst.blitrect(Vector2i(border_x, pos), window_style_info().border_right(), Recti(Vector2i::zero(), get_rborder(), img_len));
			}

			// Trailing pixels at the bottom
			dst.blitrect(Vector2i(border_x, pos), window_style_info().border_right(), Recti(Vector2i::zero(), get_rborder(), border_y2 - pos));
		}

		// Focus overlays
		// Bottom
		dst.fill_rect(Recti(0, get_h() - get_bborder(), get_w(), get_bborder()),
		              focus_color, BlendMode::Default);
		// Left
		dst.fill_rect(Recti(0, get_tborder(), get_lborder(),
		                    get_h() - get_tborder() - get_bborder()),
		              focus_color, BlendMode::Default);
		// Right
		dst.fill_rect(
		   Recti(get_w() - get_rborder(), get_tborder(), get_rborder(),
		         get_h() - get_tborder() - get_bborder()),
		   focus_color, BlendMode::Default);

	}  // end if (!is_minimal())

	// Top focus overlays
	dst.fill_rect(Recti(0, 0, get_w(), get_tborder()), focus_color, BlendMode::Default);

	// draw the title if we have one
	if (!title_.empty()) {
		// The title shouldn't be richtext, but we escape it just to make sure.
		std::shared_ptr<const UI::RenderedText> text =
		   autofit_text(richtext_escape(title_), title_style(), get_inner_w() - get_tborder());

		Vector2i pos(
		   get_lborder() + (get_inner_w() + get_tborder()) / 2, get_tborder() / 2);
		UI::center_vertically(text->height(), &pos);
		text->draw(dst, pos, UI::Align::kCenter);
	}

	// draw them again so they aren't hidden by the border
	for (Button* b : {button_close_, button_pin_, button_minimize_}) {
		b->set_pos(Vector2i(b->get_x() + get_tborder(), b->get_y() + get_tborder()));
		b->do_draw(dst);
		b->set_pos(Vector2i(b->get_x() - get_tborder(), b->get_y() - get_tborder()));
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
	if ((((SDL_GetModState() & KMOD_CTRL) != 0) && btn == SDL_BUTTON_LEFT &&
	     my < get_tborder()) ||
	    btn == SDL_BUTTON_MIDDLE) {
		is_minimal() ? restore() : minimize();
	} else if (btn == SDL_BUTTON_LEFT) {
		dragging_ = true;
		drag_start_win_x_ = get_x();
		drag_start_win_y_ = get_y();
		drag_start_mouse_x_ = get_x() + get_lborder() + mx;
		drag_start_mouse_y_ = get_y() + get_tborder() + my;
		grab_mouse(true);
		clicked();
		focus();
	} else if (btn == SDL_BUTTON_RIGHT && !pinned_) {
		play_click();
		die();
	}

	return true;
}
bool Window::handle_mouserelease(const uint8_t btn, int32_t /*x*/, int32_t /*y*/) {
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

bool Window::handle_mousewheel(int32_t /*x*/, int32_t /*y*/, uint16_t /*modstate*/) {
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
	set_border(get_lborder(), get_rborder(), get_tborder(), window_style_info().bottom_border_thickness());
	set_inner_size(get_inner_w(), oldh_);
	update_desired_size();
	move_inside_parent();
	set_handle_keypresses(true);
	update_toolbar_buttons();
}
void Window::minimize() {
	assert(!is_minimal_);
	int32_t y = get_y();
	int32_t x = get_x();
	if (y < 0) {
		y = 0;  //  Move into the screen
	}
	oldh_ = get_inner_h();
	is_minimal_ = true;
	set_border(get_lborder(), get_rborder(), get_tborder(), 0);
	set_size(get_w(), window_style_info().top_border_thickness());
	set_pos(Vector2i(x, y));  // If on border, this feels more natural
	set_handle_keypresses(false);
	update_toolbar_buttons();
}

/**
 * Drag the mouse if the left mouse button is clicked.
 * Ensure that the window isn't fully dragged out of the screen.
 */
bool Window::handle_mousemove(
   const uint8_t /*state*/, int32_t mx, int32_t my, int32_t /*xdiff*/, int32_t /*ydiff*/) {
	if (dragging_) {
		const int32_t mouse_x = get_x() + get_lborder() + mx;
		const int32_t mouse_y = get_y() + get_tborder() + my;
		int32_t left = drag_start_win_x_ + mouse_x - drag_start_mouse_x_;
		int32_t top = drag_start_win_y_ + mouse_y - drag_start_mouse_y_;
		int32_t new_left = left;
		int32_t new_top = top;

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
				const int32_t right = left + w;
				const int32_t bot = top + h;

				for (const Panel* snap_target = parent->get_first_child(); snap_target != nullptr;
				     snap_target = snap_target->get_next_sibling()) {
					if (snap_target != this && snap_target->is_snap_target()) {
						int32_t const other_left = snap_target->get_x();
						int32_t const other_top = snap_target->get_y();
						int32_t const other_right = other_left + snap_target->get_w();
						int32_t const other_bot = other_top + snap_target->get_h();

						if (other_top <= bot && other_bot >= top) {
							{
								const int32_t distance = abs(left - other_right);
								if (distance < nearest_snap_distance_x) {
									nearest_snap_distance_x = distance;
									new_left = other_right;
								}
							}
							{
								const int32_t distance = abs(right - other_left);
								if (distance < nearest_snap_distance_x) {
									nearest_snap_distance_x = distance;
									new_left = other_left - w;
								}
							}
						}
						if (other_left <= right && other_right >= left) {
							{
								const int32_t distance = abs(top - other_bot);
								if (distance < nearest_snap_distance_y) {
									nearest_snap_distance_y = distance;
									new_top = other_bot;
								}
							}
							{
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
				if (new_left <= 0 && new_left >= -get_lborder()) {
					new_left = -get_lborder();
				}
				if (new_left >= (max_x - w) && new_left <= (max_x - w) + get_rborder()) {
					new_left = (max_x - w) + get_rborder();
				}
				if (!is_minimal_) {  //  minimal windows can not be bottom-docked
					if (new_top >= (max_y - h) && new_top <= (max_y - h) + get_bborder()) {
						new_top = (max_y - h) + get_bborder();
					}
				}
			}
		}
		set_pos(Vector2i(new_left, new_top));
		moved_by_user_ = true;
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
