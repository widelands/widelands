/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "ui_basic/scrollbar.h"

#include <SDL_mouse.h>
#include <SDL_timer.h>

#include "graphic/image_cache.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "ui_basic/mouse_constants.h"
#include "wlapplication_mousewheel_options.h"

namespace UI {

/*
==============================================================================

Scrollbar

==============================================================================
*/

/**
Initialize the scrollbar using default values.
*/
Scrollbar::Scrollbar(Panel* const parent,
                     int32_t const x,
                     int32_t const y,
                     uint32_t const w,
                     uint32_t const h,
                     UI::PanelStyle style,
                     bool const horiz)
   : Panel(parent, style, x, y, w, h),
     horizontal_(horiz),
     force_draw_(false),
     pos_(0),
     singlestepsize_(1),
     pagesize_(5),
     buttonsize_(kSize),
     steps_(100),
     pressed_(Area::None),
     time_nextact_(0),
     knob_grabdelta_(0),
     pic_minus_(g_image_cache->get(horiz ? "images/ui_basic/scrollbar_left.png" :
                                           "images/ui_basic/scrollbar_up.png")),
     pic_plus_(g_image_cache->get(horiz ? "images/ui_basic/scrollbar_right.png" :
                                          "images/ui_basic/scrollbar_down.png")) {
	set_thinks(true);
	layout();

	// This is such a low-level UI element that we can safely mark it initialized immediately
	initialization_complete();
}

inline const UI::PanelStyleInfo& Scrollbar::button_style() const {
	return *g_style_manager->scrollbar_style(panel_style_);
}

/**
 * Change the number of steps of the scrollbar.
 */
void Scrollbar::set_steps(int32_t steps) {
	if (steps < 1) {
		steps = 1;
	}

	if (pos_ >= static_cast<uint32_t>(steps)) {
		set_scrollpos(steps - 1);
	}

	steps_ = steps;
	layout();
}

bool Scrollbar::is_enabled() const {
	return steps_ != 1 || force_draw_;
}

/**
 * \return the number of steps
 */
uint32_t Scrollbar::get_steps() const {
	return steps_;
}

/**
 * Change the number of steps one click on one of the arrow buttons will scroll.
 */
void Scrollbar::set_singlestepsize(uint32_t singlestepsize) {
	if (singlestepsize < 1) {
		singlestepsize = 1;
	}

	singlestepsize_ = singlestepsize;
	layout();
}

/**
 * Change the number of steps a pageup/down will scroll.
 */
void Scrollbar::set_pagesize(int32_t const pagesize) {
	pagesize_ = pagesize < 1 ? 1 : pagesize;
	layout();
}

/**
 * Change the current scrolling position.
 *
 * \param pos the new position, which will be snapped to the range [0,
 *            get_steps() - 1]
 */
void Scrollbar::set_scrollpos(int32_t pos) {
	if (pos < 0) {
		pos = 0;
	}
	if (static_cast<uint32_t>(pos) >= steps_) {
		pos = steps_ - 1;
	}

	if (pos_ == static_cast<uint32_t>(pos)) {
		return;
	}

	pos_ = pos;
	moved(pos);
}

Scrollbar::Area Scrollbar::get_area_for_point(int32_t x, int32_t y) {
	int32_t extent = 0;

	// Out of panel
	if (x < 0 || x >= static_cast<int32_t>(get_w()) || y < 0 || y >= static_cast<int32_t>(get_h())) {
		return Area::None;
	}

	// Normalize coordinates
	if (horizontal_) {
		std::swap(x, y);
		extent = get_w();
	} else {
		extent = get_h();
	}

	// Determine the area
	int32_t knob = get_knob_pos();
	int32_t knobsize = get_knob_size();

	if (y < static_cast<int32_t>(buttonsize_)) {
		return Area::Minus;
	}

	if (y < knob - knobsize / 2) {
		return Area::MinusPage;
	}

	if (y < knob + knobsize / 2) {
		return Area::Knob;
	}

	if (y < extent - static_cast<int32_t>(buttonsize_)) {
		return Area::PlusPage;
	}

	return Area::Plus;
}

/**
 * Return the center of the knob, in pixels, depending on the current position.
 */
uint32_t Scrollbar::get_knob_pos() {
	assert(0 != steps_);
	uint32_t result = buttonsize_ + get_knob_size() / 2;
	if (uint32_t const d = steps_ - 1) {
		result += pos_ * ((horizontal_ ? get_w() : get_h()) - 2 * result) / d;
	}
	return result;
}

/**
 * Change the position according to knob movement.
 */
void Scrollbar::set_knob_pos(int32_t pos) {
	uint32_t knobsize = get_knob_size();
	int32_t extent = horizontal_ ? get_w() : get_h();

	extent -= 2 * buttonsize_ + knobsize;
	pos -= buttonsize_ + knobsize / 2;

	pos = (pos * static_cast<int32_t>(steps_)) / extent;
	set_scrollpos(pos);
}

/**
 * Returns the desired size of the knob. The knob scales with the page size.
 * The returned knob size is always a multiple of 2 to avoid problems in
 * computations elsewhere.
 */
uint32_t Scrollbar::get_knob_size() {
	uint32_t extent = horizontal_ ? get_w() : get_h();

	if (extent <= 3 * buttonsize_) {
		return buttonsize_;
	}

	uint32_t maxhalfsize = extent / 2 - buttonsize_;
	uint32_t halfsize = (maxhalfsize * get_pagesize()) / (steps_ + get_pagesize());
	uint32_t size = 2 * halfsize;
	if (size < buttonsize_) {
		size = buttonsize_;
	}
	return size;
}

/// Perform the action for clicking on the given area.
void Scrollbar::action(Area const area) {
	int32_t diff = 0;
	int32_t pos = 0;

	switch (area) {
	case Area::Minus:
		diff = -1 * singlestepsize_;
		break;
	case Area::MinusPage:
		diff = -1 * pagesize_;
		break;
	case Area::Plus:
		diff = singlestepsize_;
		break;
	case Area::PlusPage:
		diff = pagesize_;
		break;
	case Area::Knob:
	case Area::None:
		return;
	}

	pos = static_cast<int32_t>(pos_) + diff;
	set_scrollpos(pos);
}

void Scrollbar::draw_button(RenderTarget& dst, Area area, const Recti& r) {
	draw_background(dst, r.cast<int>(), button_style());

	// Draw the picture
	const Image* pic = nullptr;
	if (area == Area::Minus) {
		pic = pic_minus_;
	} else if (area == Area::Plus) {
		pic = pic_plus_;
	}

	if (pic) {
		double image_scale = std::min(1., std::min(static_cast<double>(r.w - 4) / pic->width(),
		                                           static_cast<double>(r.h - 4) / pic->height()));
		int blit_width = image_scale * pic->width();
		int blit_height = image_scale * pic->height();

		dst.blitrect_scale(
		   Rectf(r.origin() + Vector2i((r.w - blit_width) / 2, (r.h - blit_height) / 2), blit_width,
		         blit_height),
		   pic, Recti(0, 0, pic->width(), pic->height()), 1., BlendMode::UseAlpha);
	}

	// Draw border
	RGBColor black(0, 0, 0);

	if (area != pressed_) {
		// top edge
		dst.brighten_rect(Recti(r.origin(), r.w, 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// left edge
		dst.brighten_rect(Recti(r.origin() + Vector2i(0, 2), 2, r.h - 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// bottom edge
		dst.fill_rect(Recti(r.origin() + Vector2i(2, r.h - 2), r.w - 2, 1), black);
		dst.fill_rect(Recti(r.origin() + Vector2i(1, r.h - 1), r.w - 1, 1), black);
		// right edge
		dst.fill_rect(Recti(r.origin() + Vector2i(r.w - 2, 2), 1, r.h - 2), black);
		dst.fill_rect(Recti(r.origin() + Vector2i(r.w - 1, 1), 1, r.h - 1), black);
	} else {
		// bottom edge
		dst.brighten_rect(
		   Recti(r.origin() + Vector2i(0, r.h - 2), r.w, 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dst.brighten_rect(
		   Recti(r.origin() + Vector2i(r.w - 2, 0), 2, r.h - 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dst.fill_rect(Recti(r.origin(), r.w - 1, 1), black);
		dst.fill_rect(Recti(r.origin() + Vector2i(0, 1), r.w - 2, 1), black);
		// left edge
		dst.fill_rect(Recti(r.origin(), 1, r.h - 1), black);
		dst.fill_rect(Recti(r.origin() + Vector2i(1, 0), 1, r.h - 2), black);
	}
}

void Scrollbar::draw_area(RenderTarget& dst, Area area, const Recti& r) {
	//  background
	dst.brighten_rect(r, area == pressed_ ? 2 * MOUSE_OVER_BRIGHT_FACTOR : MOUSE_OVER_BRIGHT_FACTOR);
	if (horizontal_) {
		//  top edge
		dst.brighten_rect(Recti(r.x, r.y, r.w - 1, 1), -BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect(Recti(r.x, r.y + 1, r.w - 2, 1), -BUTTON_EDGE_BRIGHT_FACTOR);
		//  bottom edge
		dst.brighten_rect(Recti(r.x, r.h - 2, r.w, 2), BUTTON_EDGE_BRIGHT_FACTOR);
	} else {
		//  right edge
		dst.brighten_rect(Recti(r.w - 2, r.y, 2, r.h), BUTTON_EDGE_BRIGHT_FACTOR);
		//  left edge
		dst.brighten_rect(Recti(r.x, r.y, 1, r.h - 1), -BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect(Recti(r.x + 1, r.y, 1, r.h - 2), -BUTTON_EDGE_BRIGHT_FACTOR);
	}
}

/**
 * Draw the scrollbar.
 */
void Scrollbar::draw(RenderTarget& dst) {
	if (!is_enabled()) {
		return;  // Don't draw a scrollbar that doesn't do anything
	}
	uint32_t knobpos = get_knob_pos();
	uint32_t knobsize = get_knob_size();

	if (horizontal_) {
		if ((2 * buttonsize_ + knobsize) > static_cast<uint32_t>(get_w())) {
			// Our owner allocated too little space
			if (static_cast<uint32_t>(get_w()) >= 2 * buttonsize_) {
				draw_button(dst, Area::Minus, Recti(0, 0, get_w() / 2, get_h()));
				draw_button(dst, Area::Plus, Recti(get_w() - buttonsize_, 0, get_w() / 2, get_h()));
			} else {
				draw_button(dst, Area::Minus, Recti(0, 0, get_w(), get_h()));
			}
			return;
		}

		draw_button(dst, Area::Minus, Recti(0, 0, buttonsize_, get_h()));
		draw_button(dst, Area::Plus, Recti(get_w() - buttonsize_, 0, buttonsize_, get_h()));
		draw_button(dst, Area::Knob, Recti(knobpos - knobsize / 2, 0, knobsize, get_h()));

		assert(buttonsize_ + knobsize / 2 <= knobpos);
		draw_area(dst, Area::MinusPage,
		          Recti(buttonsize_, 0, knobpos - buttonsize_ - knobsize / 2, get_h()));
		assert(knobpos + knobsize / 2 + buttonsize_ <= static_cast<uint32_t>(get_w()));
		draw_area(
		   dst, Area::PlusPage,
		   Recti(knobpos + knobsize / 2, 0, get_w() - knobpos - knobsize / 2 - buttonsize_, get_h()));
	} else {
		if ((2 * buttonsize_ + knobsize) > static_cast<uint32_t>(get_h())) {
			// Our owner allocated too little space
			if (static_cast<uint32_t>(get_h()) >= 2 * buttonsize_) {
				draw_button(dst, Area::Minus, Recti(0, 0, get_w(), get_h() / 2));
				draw_button(dst, Area::Plus, Recti(0, get_h() - buttonsize_, get_w(), get_h() / 2));
			} else {
				draw_button(dst, Area::Minus, Recti(0, 0, get_w(), get_h()));
			}
			return;
		}

		draw_button(dst, Area::Minus, Recti(0, 0, get_w(), buttonsize_));
		draw_button(dst, Area::Plus, Recti(0, get_h() - buttonsize_, get_w(), buttonsize_));
		draw_button(dst, Area::Knob, Recti(0, knobpos - knobsize / 2, get_w(), knobsize));

		assert(buttonsize_ + knobsize / 2 <= knobpos);
		draw_area(dst, Area::MinusPage,
		          Recti(0, buttonsize_, get_w(), knobpos - buttonsize_ - knobsize / 2));
		assert(knobpos + knobsize / 2 + buttonsize_ <= static_cast<uint32_t>(get_h()));
		draw_area(
		   dst, Area::PlusPage,
		   Recti(0, knobpos + knobsize / 2, get_w(), get_h() - knobpos - knobsize / 2 - buttonsize_));
	}
}

/**
 * Check for possible auto-repeat scrolling.
 */
void Scrollbar::think() {
	Panel::think();

	if (pressed_ == Area::None || pressed_ == Area::Knob) {
		return;
	}

	uint32_t const time = SDL_GetTicks();
	if (time < time_nextact_) {
		return;
	}

	action(pressed_);

	// Schedule next tick
	time_nextact_ += MOUSE_BUTTON_AUTOREPEAT_TICK;
	if (time_nextact_ < time) {
		time_nextact_ = time;
	}
}

bool Scrollbar::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	// Only vertical scrollbars are used currently
	int32_t change =
	   get_mousewheel_change(MousewheelHandlerConfigID::kScrollbarVertical, x, y, modstate);
	if (change == 0) {
		return false;
	}
	if (change > 0) {
		action(Area::Plus);
	} else {
		action(Area::Minus);
	}
	return true;
}

bool Scrollbar::handle_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	bool result = false;

	switch (btn) {
	case SDL_BUTTON_LEFT:
		pressed_ = get_area_for_point(x, y);
		if (pressed_ != Area::None) {
			grab_mouse(true);
			if (pressed_ != Area::Knob) {
				action(pressed_);
				time_nextact_ = SDL_GetTicks() + MOUSE_BUTTON_AUTOREPEAT_DELAY;
			} else {
				knob_grabdelta_ = (horizontal_ ? x : y) - get_knob_pos();
			}
		}
		result = true;
		break;

	default:
		break;
	}
	return result;
}
bool Scrollbar::handle_mouserelease(const uint8_t btn, int32_t, int32_t) {
	bool result = false;

	switch (btn) {
	case SDL_BUTTON_LEFT:
		if (pressed_ != Area::None) {
			grab_mouse(false);
			pressed_ = Area::None;
		}
		result = true;
		break;

	default:
		break;
	}
	return result;
}

/**
 * Move the knob while pressed.
 */
bool Scrollbar::handle_mousemove(uint8_t, int32_t const mx, int32_t const my, int32_t, int32_t) {
	if (pressed_ == Area::Knob) {
		set_knob_pos((horizontal_ ? mx : my) - knob_grabdelta_);
	}
	return true;
}

bool Scrollbar::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		if (horizontal_) {
			switch (code.sym) {
			case SDLK_RIGHT:
				action(Area::Plus);
				return true;
			case SDLK_LEFT:
				action(Area::Minus);
				return true;
			default:
				break;  // not handled
			}
		} else {
			switch (code.sym) {
			case SDLK_DOWN:
				action(Area::Plus);
				return true;
			case SDLK_UP:
				action(Area::Minus);
				return true;
			case SDLK_PAGEDOWN:
				action(Area::PlusPage);
				return true;
			case SDLK_PAGEUP:
				action(Area::MinusPage);
				return true;
			default:
				break;  // not handled
			}
		}
	}
	return Panel::handle_key(down, code);
}

void Scrollbar::layout() {
	if ((2 * kSize + get_knob_size()) > static_cast<uint32_t>((horizontal_ ? get_w() : get_h()))) {
		buttonsize_ = kSize / 2;
	} else {
		buttonsize_ = kSize;
	}
	set_can_focus(is_enabled());
}

}  // namespace UI
