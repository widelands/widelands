/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "ui_basic/panel.h"

#include <memory>

#include <SDL_timer.h>

#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/mouse_cursor.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"
#include "sound/sound_handler.h"
#include "wlapplication.h"
#include "wlapplication_options.h"

namespace UI {

Panel* Panel::modal_ = nullptr;
Panel* Panel::mousegrab_ = nullptr;
Panel* Panel::mousein_ = nullptr;

// The following variable can be set to false. If so, all mouse and keyboard
// events are ignored and not passed on to any widget. This is only useful
// for scripts that want to show off functionality without the user interfering.
bool Panel::allow_user_input_ = true;
FxId Panel::click_fx_ = kNoSoundEffect;

/**
 * Initialize a panel, link it into the parent's queue.
 */
Panel::Panel(Panel* const nparent,
             const int nx,
             const int ny,
             const int nw,
             const int nh,
             const std::string& tooltip_text)
   : parent_(nparent),
     first_child_(nullptr),
     last_child_(nullptr),
     mousein_child_(nullptr),
     focus_(nullptr),
     flags_(pf_handle_mouse | pf_thinks | pf_visible | pf_handle_keypresses),
     x_(nx),
     y_(ny),
     w_(nw),
     h_(nh),
     lborder_(0),
     rborder_(0),
     tborder_(0),
     bborder_(0),
     border_snap_distance_(0),
     panel_snap_distance_(0),
     desired_w_(nw),
     desired_h_(nh),
     running_(false),
     tooltip_(tooltip_text) {
	assert(nparent != this);
	if (parent_) {
		next_ = parent_->first_child_;
		prev_ = nullptr;
		if (next_) {
			next_->prev_ = this;
		} else {
			parent_->last_child_ = this;
		}
		parent_->first_child_ = this;
	} else {
		prev_ = next_ = nullptr;
	}
}

/**
 * Unlink the panel from the parent's queue
 */
Panel::~Panel() {
	// Release pointers to this object
	if (mousegrab_ == this) {
		mousegrab_ = nullptr;
	}
	if (mousein_ == this) {
		mousein_ = nullptr;
	}

	// Free children
	free_children();

	// Unlink
	if (parent_) {
		if (parent_->mousein_child_ == this) {
			parent_->mousein_child_ = nullptr;
		}
		if (parent_->focus_ == this) {
			parent_->focus_ = nullptr;
		}

		if (prev_) {
			prev_->next_ = next_;
		} else {
			parent_->first_child_ = next_;
		}
		if (next_) {
			next_->prev_ = prev_;
		} else {
			parent_->last_child_ = prev_;
		}
	}
}

/**
 * Free all of the panel's children.
 */
void Panel::free_children() {
	// Scan-build claims this results in double free.
	// This is a false positive.
	// The reason is that the variable will be reassigned in the destructor of the deleted child.
	// This is very uncommon behavior and bad style, but will be non trivial to fix.
	while (first_child_) {
		Panel* next_child = first_child_->next_;
		delete first_child_;
		first_child_ = next_child;
	}
	first_child_ = nullptr;
}

/**
 * Enters the event loop; all events will be handled by this panel.
 *
 * \return the return code passed to end_modal. This return code will be
 * negative when the event loop was quit in an abnormal way (e.g. the user
 * clicked the window's close button or similar).
 */
int Panel::do_run() {
	// TODO(sirver): the main loop should not be in UI, but in WLApplication.
	WLApplication* const app = WLApplication::get();
	Panel* const prevmodal = modal_;
	modal_ = this;
	mousegrab_ = nullptr;        // good ol' paranoia
	app->set_mouse_lock(false);  // more paranoia :-)

	Panel* forefather = this;
	while (forefather->parent_ != nullptr) {
		forefather = forefather->parent_;
	}

	// Loop
	running_ = true;

	// Panel-specific startup code. This might call end_modal()!
	start();

	// think() is called at most 15 times per second, that is roughly ever 66ms.
	const uint32_t kGameLogicDelay = 1000 / 15;

	// With the default of 30FPS, the game will be drawn every 33ms.
	const uint32_t draw_delay = 1000 / std::max(5, get_config_int("maxfps", 30));

	static InputCallback input_callback = {Panel::ui_mousepress, Panel::ui_mouserelease,
	                                       Panel::ui_mousemove,  Panel::ui_key,
	                                       Panel::ui_textinput,  Panel::ui_mousewheel};

	const uint32_t initial_ticks = SDL_GetTicks();
	uint32_t next_think_time = initial_ticks + kGameLogicDelay;
	uint32_t next_draw_time = initial_ticks + draw_delay;
	while (running_) {
		const uint32_t start_time = SDL_GetTicks();

		app->handle_input(&input_callback);

		if (start_time >= next_think_time) {
			if (app->should_die()) {
				end_modal<Returncodes>(Returncodes::kBack);
			}

			do_think();

			if (flags_ & pf_child_die) {
				check_child_death();
			}

			next_think_time = start_time + kGameLogicDelay;
		}

		if (start_time >= next_draw_time) {
			RenderTarget& rt = *g_gr->get_render_target();
			forefather->do_draw(rt);
			if (g_mouse_cursor->is_visible()) {
				g_mouse_cursor->change_cursor(app->is_mouse_pressed());
				g_mouse_cursor->draw(rt, app->get_mouse_position());
				if (is_modal()) {
					do_tooltip();
				} else {
					forefather->do_tooltip();
				}
			}

			g_gr->refresh();
			next_draw_time = start_time + draw_delay;
		}

		int32_t delay = std::min<int32_t>(next_draw_time, next_think_time) - SDL_GetTicks();
		if (delay > 0) {
			SDL_Delay(delay);
		}
	}
	end();

	// Done
	modal_ = prevmodal;

	return return_code_;
}

/**
 * \return \c true if this is the currently modal panel
 */
bool Panel::is_modal() {
	return running_;
}

/**
 * Called once before the event loop in run is started
 */
void Panel::start() {
}

/**
 * Called once after the event loop in run() has ended
 */
void Panel::end() {
}

/**
 * Resizes the panel.
 *
 * \note NEVER override this function. If you feel the urge to override this
 * function, you probably want to override \ref layout.
 */
void Panel::set_size(const int nw, const int nh) {
	if (nw == w_ && nh == h_) {
		return;
	}

	// Make sure that we never get negative width/height.
	w_ = std::max(0, nw);
	h_ = std::max(0, nh);

	if (parent_) {
		move_inside_parent();
	}

	layout();
}

/**
 * Move the panel. Panel's position is relative to the parent.
 */
void Panel::set_pos(const Vector2i n) {
	x_ = n.x;
	y_ = n.y;
	position_changed();
}

/**
 * Set \p w and \p h to the desired
 * width and height of this panel, respectively.
 */
void Panel::get_desired_size(int* w, int* h) const {
	*w = desired_w_;
	*h = desired_h_;
}

/**
 * Set this panel's desired size and invoke the recursive update of the parent.
 *
 * \note The desired size of a panel must only depend on the attributes of this
 * panel and its children that are not derived from layout routines.
 * In particular, it must be independent of the panel's position on the screen
 * or of its actual size.
 *
 * \note NEVER override this function
 */
void Panel::set_desired_size(int w, int h) {
	if (desired_w_ == w && desired_h_ == h) {
		return;
	}

	// Make sure that we never get negative width/height.
	desired_w_ = std::max(0, w);
	desired_h_ = std::max(0, h);
	if (!get_layout_toplevel() && parent_) {
		parent_->update_desired_size();
	} else {
		set_size(desired_w_, desired_h_);
	}
}

/**
 * Recompute this panel's desired size.
 *
 * This is automatically called whenever a child panel's desired size changes.
 */
void Panel::update_desired_size() {
}

/**
 * Set whether this panel acts as a layouting toplevel.
 *
 * Typically, only true for \ref Window.
 */
void Panel::set_layout_toplevel(bool ltl) {
	flags_ &= ~pf_layout_toplevel;
	if (ltl) {
		flags_ |= pf_layout_toplevel;
	}
}

bool Panel::get_layout_toplevel() const {
	return flags_ & pf_layout_toplevel;
}

/**
 * Interpret \p pt as a point in the interior of this panel,
 * and translate it into the interior coordinate system of the parent
 * and return the result.
 */
Vector2i Panel::to_parent(const Vector2i& pt) const {
	if (!parent_) {
		return pt;
	}

	return pt + Vector2i(lborder_ + x_, tborder_ + y_);
}

/**
 * Ensure the panel is inside the parent's visibile area.
 *
 * The default implementation does nothing, this is overridden
 * by \ref Window
 */
void Panel::move_inside_parent() {
}

/**
 * Automatically layout the children of this panel and adjust their size.
 *
 * \note This is always called when this panel's size is changed, so do not
 * call \ref set_size from this function!
 *
 * The default implementation does nothing.
 */
void Panel::layout() {
}

/**
 * Set the size of the inner area (total area minus border)
 */
void Panel::set_inner_size(int const nw, int const nh) {
	assert(nw >= 0 && nh >= 0);
	set_size(nw + lborder_ + rborder_, nh + tborder_ + bborder_);
}

/**
 * Change the border dimensions.
 * Note that since position and total size aren't changed, so that the size
 * and position of the inner area will change.
 */
void Panel::set_border(int l, int r, int t, int b) {
	lborder_ = l;
	rborder_ = r;
	tborder_ = t;
	bborder_ = b;
}

int Panel::get_inner_w() const {
	assert(w_ == 0 || lborder_ + rborder_ <= w_);
	return (w_ == 0 ? 0 : w_ - (lborder_ + rborder_));
}
int Panel::get_inner_h() const {
	assert(h_ == 0 || tborder_ + bborder_ <= h_);
	return (h_ == 0 ? 0 : h_ - (tborder_ + bborder_));
}

/**
 * Make this panel the top-most panel in the parent's Z-order.
 */
void Panel::move_to_top() {
	if (!parent_) {
		return;
	}

	// unlink
	if (prev_) {
		prev_->next_ = next_;
	} else {
		parent_->first_child_ = next_;
	}
	if (next_) {
		next_->prev_ = prev_;
	} else {
		parent_->last_child_ = prev_;
	}

	// relink
	prev_ = nullptr;
	next_ = parent_->first_child_;
	parent_->first_child_ = this;
	if (next_) {
		next_->prev_ = this;
	} else {
		parent_->last_child_ = this;
	}
}

/**
 * Makes the panel visible or invisible
 */
void Panel::set_visible(bool const on) {
	if (((flags_ & pf_visible) > 1) == on) {
		return;
	}

	flags_ &= ~pf_visible;
	if (on) {
		flags_ |= pf_visible;
	} else if (parent_ && parent_->focus_ == this) {
		parent_->focus_ = nullptr;
	}
	if (parent_) {
		parent_->on_visibility_changed();
	}
}

/**
 * Called on a child's parent when visibility of child changed
 * Overridden in UI::Box
 */
void Panel::on_visibility_changed() {
}

/**
 * Redraw the panel. Note that all drawing coordinates are relative to the
 * inner area: you cannot overwrite the panel border in this function.
 */
void Panel::draw(RenderTarget&) {
}

/**
 * Redraw the panel border.
 */
void Panel::draw_border(RenderTarget&) {
}

std::vector<Recti>
Panel::focus_overlay_rects(const int off_x, const int off_y, const int strength_diff) {
	const int f = g_style_manager->focus_border_thickness() + strength_diff;
	const int16_t w = get_w();
	const int16_t h = get_h();
	if (w < 2 * (f + off_x) || h < 2 * (f + off_y)) {
		return {Recti(0, 0, w, h)};
	}
	return {Recti(off_x, off_y, w - 2 * off_x, f), Recti(off_x, h - off_y - f, w - 2 * off_x, f),
	        Recti(off_x, off_y + f, f, h - 2 * f - 2 * off_y),
	        Recti(w - off_x - f, off_y + f, f, h - 2 * f - 2 * off_y)};
}

std::vector<Recti> Panel::focus_overlay_rects() {
	return focus_overlay_rects(0, 0, 0);
}

/**
 * Draw overlays that appear over all child panels.
 * This can be used e.g. for debug information.
 */
void Panel::draw_overlay(RenderTarget& dst) {
	if (has_focus()) {
		for (Panel* p = this; p->parent_; p = p->parent_) {
			if (p->parent_->focus_ != p) {
				// doesn't have toplevel focus
				return;
			}
			if (p->parent_->is_focus_toplevel()) {
				break;
			}
		}
		for (const Recti& r : focus_overlay_rects()) {
			dst.fill_rect(
			   r, focus_ ? g_style_manager->semi_focused_color() : g_style_manager->focused_color(),
			   BlendMode::Default);
		}
	}
}

/**
 * Draw texture and color from the info if they have been specified.
 */
void Panel::draw_background(RenderTarget& dst, const UI::PanelStyleInfo& info) {
	draw_background(dst, Recti(0, 0, get_w(), get_h()), info);
}
void Panel::draw_background(RenderTarget& dst, Recti rect, const UI::PanelStyleInfo& info) {
	if (info.image() != nullptr) {
		dst.fill_rect(rect, RGBAColor(0, 0, 0, 255));
		dst.tile(rect, info.image(), Vector2i(get_x(), get_y()));
	}
	if (info.color() != RGBAColor(0, 0, 0, 0)) {
		dst.fill_rect(rect, info.color(), BlendMode::UseAlpha);
	}
}

/**
 * Called once per event loop pass, unless set_think(false) has
 * been called. It is intended to be used for animations and game logic.
 */
void Panel::think() {
}

/**
 * Descend the panel hierarchy and call the \ref think() function of all
 * (grand-)children for which set_thinks(false) has not been called.
 */
void Panel::do_think() {
	if (thinks()) {
		think();
	}

	for (Panel* child = first_child_; child; child = child->next_) {
		child->do_think();
	}
}

/**
 * Get mouse position relative to this panel
 */
Vector2i Panel::get_mouse_position() const {
	return (parent_ ? parent_->get_mouse_position() : WLApplication::get()->get_mouse_position()) -
	       Vector2i(get_x() + get_lborder(), get_y() + get_tborder());
}

/**
 * Set mouse position relative to this panel
 */
void Panel::set_mouse_pos(const Vector2i p) {
	const Vector2i relative_p = p + Vector2i(get_x() + get_lborder(), get_y() + get_tborder());
	if (parent_) {
		parent_->set_mouse_pos(relative_p);
	} else {
		WLApplication::get()->warp_mouse(relative_p);
	}
}

/**
 * Center the mouse on this panel.
 */
void Panel::center_mouse() {
	set_mouse_pos(Vector2i(get_w() / 2, get_h() / 2));
}

/**
 * Called whenever the mouse enters or leaves the panel. The inside state
 * is relative to the outer area of a panel. This means that the mouse
 * position received in handle_mousemove may be negative while the mouse is
 * still inside the panel as far as handle_mousein is concerned.
 */
void Panel::handle_mousein(bool) {
}

/**
 * Called whenever the user presses a mouse button in the panel.
 * If the panel doesn't process the mouse-click, it is handed to the panel's
 * parent.
 *
 * \return true if the mouseclick was processed, false otherwise
 */
bool Panel::handle_mousepress(const uint8_t btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT && get_can_focus()) {
		focus();
		clicked();
	}
	return false;
}

/**
 * Called whenever the user releases a mouse button in the panel.
 * If the panel doesn't process the mouse-click, it is handed to the panel's
 * parent.
 *
 * \return true if the mouseclick was processed, false otherwise
 */
bool Panel::handle_mouserelease(const uint8_t, int32_t, int32_t) {
	return false;
}

/**
 * Called whenever the user moves the mouse wheel.
 * If the panel doesn't process the mouse-wheel, it is handed to the panel's
 * parent.
 *
 * \return true if the mouseclick was processed, false otherwise
 */
bool Panel::handle_mousewheel(uint32_t, int32_t, int32_t) {
	return false;
}

/**
 * Called when the mouse is moved while inside the panel
 */
bool Panel::handle_mousemove(const uint8_t, int32_t, int32_t, int32_t, int32_t) {
	return !tooltip_.empty();
}

bool Panel::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_TAB:
			return handle_tab_pressed(SDL_GetModState() & KMOD_SHIFT);
		case SDLK_ESCAPE:
			if (parent_ && parent_->focus_ == this && get_can_focus()) {
				parent_->focus_ = nullptr;
				return true;
			}
			break;
		default:
			break;
		}
	}
	return false;
}

bool Panel::handle_textinput(const std::string& /* text */) {
	return false;
}

/**
 * Called whenever a tooltip could be drawn.
 * Return true if the tooltip has been drawn,
 * false otherwise.
 */
bool Panel::handle_tooltip() {
	return draw_tooltip(tooltip());
}

// Whether TAB events should be handled by this panel's parent (`false`) or by `this` (`true`)
bool Panel::is_focus_toplevel() const {
	return !parent_ || this == modal_;
}

// Let the toplevel panel transfer the focus to the next/prev focusable child
bool Panel::handle_tab_pressed(const bool reverse) {
	if (!is_focus_toplevel()) {
		return parent_->handle_tab_pressed(reverse);
	}

	std::deque<Panel*> list = gather_focusable_children();
	if (list.empty()) {
		// nothing to do
		return false;
	}
	const size_t list_size = list.size();

	if (focus_ == nullptr || !focus_->is_visible() || list_size <= 1) {
		// no focus yet – select the first item
		list[reverse ? 0 : list_size - 1]->focus();
		return true;
	}

	Panel* currently_focused = focus_;
	while (currently_focused->focus_ && currently_focused->focus_->is_visible()) {
		currently_focused = currently_focused->focus_;
	}
	// tell the next/prev panel to focus
	for (size_t i = 0; i < list_size; ++i) {
		if (list[i] == currently_focused) {
			list[(i + (reverse ? 1 : list_size - 1)) % list_size]->focus();
			return true;
		}
	}

	list[reverse ? 0 : list_size - 1]->focus();
	return true;
}

// Recursively create a sorted list of all children that can get the focus
std::deque<Panel*> Panel::gather_focusable_children() {
	if (get_can_focus() && !has_focus()) {
		return {this};
	}
	std::deque<Panel*> list;
	for (Panel* child = first_child_; child; child = child->next_) {
		if (child->is_visible()) {
			for (Panel* p : child->gather_focusable_children()) {
				list.push_back(p);
			}
		}
	}
	if (get_can_focus()) {
		list.push_back(this);
	}
	return list;
}

/**
 * Enable/Disable mouse handling by this panel
 * Default is enabled. Note that when mouse handling is disabled, child panels
 * don't receive mouse events either.
 *
 * \param yes true if the panel should receive mouse events
 */
void Panel::set_handle_mouse(bool const yes) {
	if (yes) {
		flags_ |= pf_handle_mouse;
	} else {
		flags_ &= ~pf_handle_mouse;
	}
}

/**
 * Enable/Disable mouse grabbing. If a panel grabs the mouse, all mouse
 * related events will be sent directly to that panel.
 * You should only grab the mouse as a response to a mouse event (e.g.
 * clicking a mouse button)
 *
 * \param grab true if the mouse should be grabbed
 */
void Panel::grab_mouse(bool const grab) {
	if (grab) {
		mousegrab_ = this;
	} else {
		assert(!mousegrab_ || mousegrab_ == this);
		mousegrab_ = nullptr;
	}
}

/**
 * Set if this panel can receive the keyboard focus
 */
void Panel::set_can_focus(bool const yes) {

	if (yes) {
		flags_ |= pf_can_focus;
	} else {
		flags_ &= ~pf_can_focus;

		if (parent_ && parent_->focus_ == this) {
			parent_->focus_ = nullptr;
		}
	}
}

/**
 * Grabs the keyboard focus, if it can,
 * topcaller identifies widget at the beginning of the recursion
 */
void Panel::focus(const bool topcaller) {
	if (topcaller) {
		if (handles_textinput()) {
			if (!SDL_IsTextInputActive()) {
				SDL_StartTextInput();
			}
		} else {
			if (SDL_IsTextInputActive()) {
				SDL_StopTextInput();
			}
		}
		focus_ = nullptr;
	}

	if (!parent_ || this == modal_) {
		return;
	}

	parent_->focus_ = this;
	parent_->focus(false);
}

/**
 * Enables/Disables calling think() during the event loop.
 * The default is enabled.
 *
 * \param yes true if the panel's think function should be called
 */
void Panel::set_thinks(bool const yes) {
	if (yes) {
		flags_ |= pf_thinks;
	} else {
		flags_ &= ~pf_thinks;
	}
}

/**
 * Cause this panel to be removed on the next frame.
 * Use this for a panel that needs to destroy itself after a button has
 * been pressed (e.g. non-modal dialogs).
 * Do NOT use this to delete a hierarchy of panels that have been modal.
 */
void Panel::die() {
	flags_ |= pf_die;

	for (Panel* p = parent_; p; p = p->parent_) {
		p->flags_ |= pf_child_die;
		if (p == modal_) {
			break;
		}
	}
}
/**
 * Called on a child's parent just before child is deleted.
 * Overridden in UI::Box
 */
void Panel::on_death(Panel*) {
}

/**
 * Wrapper around SoundHandler::play_fx() to prevent having to include
 * sound_handler.h in every UI subclass just for playing a 'click'
 */
void Panel::play_click() {
	g_sh->play_fx(SoundType::kUI, click_fx_);
}

/**
 * This needs to be called once after g_soundhandler has been instantiated and before play_click()
 * is called. We do it this way so that we don't have to register the same sound every time we
 * create a new panel.
 */
void Panel::register_click() {
	click_fx_ = SoundHandler::register_fx(SoundType::kUI, "sound/click");
}

/**
 * Recursively walk the panel tree, killing panels that are marked for death
 * using die().
 */
void Panel::check_child_death() {
	Panel* next = first_child_;
	while (next) {
		Panel* p = next;
		next = p->next_;

		if (p->flags_ & pf_die) {
			if (p->parent_) {
				p->parent_->on_death(p);
			}
			delete p;
			p = nullptr;
		} else if (p->flags_ & pf_child_die) {
			p->check_child_death();
		}
	}

	flags_ &= ~pf_child_die;
}

/**
 * Draw the inner region of the panel into the given target.
 *
 * \param dst target to render into, assumed to be prepared for the panel's
 * inner coordinate system.
 */
void Panel::do_draw_inner(RenderTarget& dst) {
	draw(dst);

	// draw back to front
	for (Panel* child = last_child_; child; child = child->prev_) {
		child->do_draw(dst);
	}

	draw_overlay(dst);
}

/**
 * Subset for the border first and draw the border, then subset for the inner
 * area and draw the inner area.
 * Draw child panels after drawing self.
 * Draw tooltip if required.
 *
 * \param dst RenderTarget for the parent Panel
 */
void Panel::do_draw(RenderTarget& dst) {
	// Make sure the panel's size is sane. If it's bigger than 10000 it's likely a bug.
	assert(desired_w_ <= std::max(10000, g_gr->get_xres()));
	assert(desired_h_ <= std::max(10000, g_gr->get_yres()));

	if (!is_visible()) {
		return;
	}

	Recti outerrc;
	Vector2i outerofs = Vector2i::zero();

	if (!dst.enter_window(Recti(Vector2i(x_, y_), w_, h_), &outerrc, &outerofs)) {
		return;
	}

	draw_border(dst);

	Recti innerwindow(
	   Vector2i(lborder_, tborder_), w_ - (lborder_ + rborder_), h_ - (tborder_ + bborder_));

	if (dst.enter_window(innerwindow, nullptr, nullptr)) {
		do_draw_inner(dst);
	}

	dst.set_window(outerrc, outerofs);
}

/**
 * Returns the child panel that receives mouse events at the given location.
 * Starts the search with child (which should usually be set to first_child_) and
 * returns the first match.
 */
inline Panel* Panel::child_at_mouse_cursor(int32_t const x, int32_t const y, Panel* child) {

	for (; child; child = child->next_) {
		if (!child->handles_mouse() || !child->is_visible()) {
			continue;
		}
		if (x < child->x_ + static_cast<int32_t>(child->w_) && x >= child->x_ &&
		    y < child->y_ + static_cast<int32_t>(child->h_) && y >= child->y_) {
			break;
		}
	}

	if (mousein_child_ && mousein_child_ != child) {
		mousein_child_->do_mousein(false);
	}
	mousein_child_ = child;
	if (child) {
		child->do_mousein(true);
	}

	return child;
}

/**
 * Propagate mouseleave events (e.g. for buttons that are inside a different
 * window)
 */
void Panel::do_mousein(bool const inside) {
	if (!inside && mousein_child_) {
		mousein_child_->do_mousein(false);
		mousein_child_ = nullptr;
	}
	handle_mousein(inside);
}

/**
 * Propagate mousepresses/-releases/-moves to the appropriate panel.
 *
 * Returns whether the event was processed.
 */
bool Panel::do_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	if (get_can_focus()) {
		focus();
	}
	x -= lborder_;
	y -= tborder_;
	if (flags_ & pf_top_on_click) {
		move_to_top();
	}

	if (mousegrab_ != this) {
		for (Panel* child = first_child_; (child = child_at_mouse_cursor(x, y, child));
		     child = child->next_) {
			if (child->do_mousepress(btn, x - child->x_, y - child->y_)) {
				return true;
			}
		}
	}
	return handle_mousepress(btn, x, y);
}

bool Panel::do_mousewheel(uint32_t which, int32_t x, int32_t y, Vector2i rel_mouse_pos) {
	// Check if a child-panel is beneath the mouse and processes the event
	for (Panel* child = first_child_; child; child = child->next_) {
		if (!child->handles_mouse() || !child->is_visible()) {
			continue;
		}
		if (rel_mouse_pos.x >= child->x_ + static_cast<int32_t>(child->w_) ||
		    rel_mouse_pos.x < child->x_ ||
		    rel_mouse_pos.y >= child->y_ + static_cast<int32_t>(child->h_) ||
		    rel_mouse_pos.y < child->y_) {
			continue;
		}
		// Found a child at the position
		if (child->do_mousewheel(which, x, y,
		                         rel_mouse_pos - Vector2i(child->get_x() + child->get_lborder(),
		                                                  child->get_y() + child->get_tborder()))) {
			return true;
		}
	}
	return handle_mousewheel(which, x, y);
}

bool Panel::do_mouserelease(const uint8_t btn, int32_t x, int32_t y) {
	x -= lborder_;
	y -= tborder_;
	if (mousegrab_ != this) {
		for (Panel* child = first_child_; (child = child_at_mouse_cursor(x, y, child));
		     child = child->next_) {
			if (child->do_mouserelease(btn, x - child->x_, y - child->y_)) {
				return true;
			}
		}
	}
	return handle_mouserelease(btn, x, y);
}

bool Panel::do_mousemove(
   uint8_t const state, int32_t x, int32_t y, int32_t const xdiff, int32_t const ydiff) {
	x -= lborder_;
	y -= tborder_;
	if (mousegrab_ != this) {
		for (Panel* child = first_child_; (child = child_at_mouse_cursor(x, y, child));
		     child = child->next_) {
			if (child->do_mousemove(state, x - child->x_, y - child->y_, xdiff, ydiff)) {
				return true;
			}
		}
	}
	return handle_mousemove(state, x, y, xdiff, ydiff);
}

/**
 * Pass the key event to the focused child.
 * If it doesn't process the key, we'll see if we can use the event.
 */
bool Panel::do_key(bool const down, SDL_Keysym const code) {
	if (focus_ && focus_->do_key(down, code)) {
		return true;
	}

	if (!handles_keypresses()) {
		return false;
	}

	if (handle_key(down, code)) {
		return true;
	}

	// If we handle text, we want to block propagation of keypresses used for
	// text input. We don't know which ones they are, so we block all except
	// those we are reasonably sure that they aren't. This list may be expanded.
	if (handles_textinput()) {
		switch (code.sym) {
		case SDLK_ESCAPE:
		case SDLK_PAUSE:
		case SDLK_PRINTSCREEN:
		case SDLK_PAGEDOWN:
		case SDLK_PAGEUP:
		case SDLK_HOME:
		case SDLK_END:
		case SDLK_DELETE:
		case SDLK_INSERT:
		case SDLK_BACKSPACE:
		case SDLK_LEFT:
		case SDLK_RIGHT:
		case SDLK_UP:
		case SDLK_DOWN:
		case SDLK_LCTRL:
		case SDLK_RCTRL:
		case SDLK_LALT:
			return false;
		}
		if (code.mod & KMOD_CTRL || (code.sym >= SDLK_F1 && code.sym <= SDLK_F12)) {
			return false;
		}
		return true;
	}

	return false;
}

bool Panel::do_textinput(const std::string& text) {
	if (focus_ && focus_->do_textinput(text)) {
		return true;
	}

	if (!handles_textinput()) {
		return false;
	}

	return handle_textinput(text);
}

bool Panel::do_tooltip() {
	if (mousein_child_ && mousein_child_->do_tooltip()) {
		return true;
	}
	return handle_tooltip();
}

/**
 * \return \c true if the given key is currently pressed, or \c false otherwise
 */
bool Panel::get_key_state(const SDL_Scancode key) const {
	return WLApplication::get()->get_key_state(key);
}

UI::Panel* Panel::get_open_dropdown() {
	for (Panel* child = first_child_; child; child = child->next_) {
		if (UI::Panel* dd = child->get_open_dropdown()) {
			return dd;
		}
	}
	return nullptr;
}

/**
 * Determine which panel is to receive a mouse event.
 *
 * \return The panel which receives the mouse event
 */
Panel* Panel::ui_trackmouse(int32_t& x, int32_t& y) {
	Panel* mousein;
	Panel* rcv = nullptr;

	if (mousegrab_) {
		mousein = rcv = mousegrab_;
	} else {
		mousein = modal_;
	}

	// ugly hack to handle dropdowns in modal windows correctly
	if (mousein->get_parent()) {
		if (UI::Panel* dd = mousein->get_open_dropdown()) {
			mousein = rcv = dd;
		}
	}

	x -= mousein->x_;
	y -= mousein->y_;
	for (Panel* p = mousein->parent_; p; p = p->parent_) {
		x -= p->lborder_ + p->x_;
		y -= p->tborder_ + p->y_;
	}

	if (0 <= x && x < static_cast<int32_t>(mousein->w_) && 0 <= y &&
	    y < static_cast<int32_t>(mousein->h_)) {
		rcv = mousein;
	} else {
		mousein = nullptr;
	}

	if (mousein != mousein_) {
		if (mousein_) {
			mousein_->do_mousein(false);
		}
		mousein_ = mousein;
		if (mousein_) {
			mousein_->do_mousein(true);
		}
	}

	return rcv;
}

/**
 * Input callback function. Pass the mouseclick event to the currently modal
 * panel.
 */
bool Panel::ui_mousepress(const uint8_t button, int32_t x, int32_t y) {
	if (!allow_user_input_) {
		return true;
	}

	Panel* const p = ui_trackmouse(x, y);
	if (p == nullptr) {
		return false;
	}
	return p->do_mousepress(button, x, y);
}

bool Panel::ui_mouserelease(const uint8_t button, int32_t x, int32_t y) {
	if (!allow_user_input_) {
		return true;
	}

	Panel* const p = ui_trackmouse(x, y);
	if (p == nullptr) {
		return false;
	}
	return p->do_mouserelease(button, x, y);
}

/**
 * Input callback function. Pass the mousemove event to the currently modal
 * panel.
 */
bool Panel::ui_mousemove(
   uint8_t const state, int32_t x, int32_t y, int32_t const xdiff, int32_t const ydiff) {
	if (!allow_user_input_) {
		return true;
	}

	if (!xdiff && !ydiff) {
		return true;
	}

	Panel* const p = ui_trackmouse(x, y);
	if (p == nullptr) {
		return false;
	}

	return p->do_mousemove(state, x, y, xdiff, ydiff);
}

/**
 * Input callback function. Pass the mousewheel event to the currently modal
 * panel.
 */
bool Panel::ui_mousewheel(uint32_t which, int32_t x, int32_t y) {
	if (!allow_user_input_) {
		return true;
	}
	if (!x && !y) {
		return true;
	}
	Panel* p = nullptr;
	if (mousein_) {
		p = mousein_;
	} else {
		p = mousegrab_ ? mousegrab_ : modal_;
	}
	if (!p) {
		return false;
	}
	return p->do_mousewheel(which, x, y, p->get_mouse_position());
}

/**
 * Input callback function. Pass the key event to the currently modal panel
 */
bool Panel::ui_key(bool const down, SDL_Keysym const code) {
	if (!allow_user_input_) {
		return true;
	}
	Panel* p = modal_;
	if (p->get_parent()) {
		if (UI::Panel* dd = p->get_open_dropdown()) {
			p = dd;
		}
	}
	return p->do_key(down, code);
}

/**
 * Input callback function. Pass the textinput event to the currently modal panel
 */
bool Panel::ui_textinput(const std::string& text) {
	if (!allow_user_input_) {
		return true;
	}
	return modal_->do_textinput(text);
}

/**
 * Draw the tooltip. Return true on success
 */
bool Panel::draw_tooltip(const std::string& text) {
	if (text.empty()) {
		return false;
	}

	RenderTarget& dst = *g_gr->get_render_target();
	std::string text_to_render = text;
	if (!is_richtext(text_to_render)) {
		text_to_render = as_richtext_paragraph(text_to_render, UI::FontStyle::kTooltip);
	}

	constexpr uint32_t kTipWidthMax = 360;
	std::shared_ptr<const UI::RenderedText> rendered_text =
	   g_fh->render(text_to_render, kTipWidthMax);
	if (rendered_text->rects.empty()) {
		return false;
	}

	const uint16_t tip_width = rendered_text->width() + 4;
	const uint16_t tip_height = rendered_text->height() + 4;

	Recti r(WLApplication::get()->get_mouse_position() + Vector2i(2, 32), tip_width, tip_height);
	const Vector2i tooltip_bottom_right = r.opposite_of_origin();
	const Vector2i screen_bottom_right(g_gr->get_xres(), g_gr->get_yres());
	if (screen_bottom_right.x < tooltip_bottom_right.x) {
		r.x -= 4 + r.w;
	}
	if (screen_bottom_right.y < tooltip_bottom_right.y) {
		r.y -= 35 + r.h;
	}

	dst.fill_rect(r, RGBColor(63, 52, 34));
	dst.draw_rect(r, RGBColor(0, 0, 0));
	rendered_text->draw(dst, r.origin() + Vector2i(2, 2));
	return true;
}
}  // namespace UI
