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

#include "ui_basic/button.h"

#include <memory>

#include <SDL_mouse.h>
#include <SDL_timer.h>

#include "graphic/font_handler.h"
#include "graphic/image.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "ui_basic/mouse_constants.h"

namespace UI {

// Margin around image. The image will be scaled down to fit into this rectangle with preserving
// size.
constexpr int kButtonImageMargin = 2;

Button::Button  //  Common constructor
   (Panel* const parent,
    const std::string& name,
    int32_t const x,
    int32_t const y,
    uint32_t const w,
    uint32_t const h,
    UI::ButtonStyle init_style,
    const Image* title_image,
    const std::string& title_text,
    const std::string& tooltip_text,
    UI::Button::VisualState init_state,
    ImageMode mode)
   : NamedPanel(parent, to_panel_style(init_style), name, x, y, w, h, tooltip_text),
     highlighted_(false),
     pressed_(false),
     enabled_(true),
     visual_state_(init_state),
     disable_style_(ButtonDisableStyle::kMonochrome),
     repeating_(false),
     image_mode_(mode),
     time_nextact_(0),
     title_(title_text),
     title_image_(title_image),
     button_style_(init_style) {
	set_thinks(false);
	set_can_focus(enabled_);
}

/// For textual buttons. If h = 0, h will resize according to the font's height. If both h = 0 and w
/// = 0, will resize for text width as well.
Button::Button(Panel* const parent,
               const std::string& name,
               int32_t const x,
               int32_t const y,
               uint32_t const w,
               uint32_t const h,
               UI::ButtonStyle init_style,
               const std::string& title_text,
               const std::string& tooltip_text,
               UI::Button::VisualState init_state)
   : Button(parent,
            name,
            x,
            y,
            w,
            h,
            init_style,
            nullptr,
            title_text,
            tooltip_text,
            init_state,
            UI::Button::ImageMode::kShrink) {
	expand(w, h);
}

Button::Button  //  for pictorial buttons
   (Panel* const parent,
    const std::string& name,
    const int32_t x,
    const int32_t y,
    const uint32_t w,
    const uint32_t h,
    UI::ButtonStyle init_style,
    const Image* title_image,
    const std::string& tooltip_text,
    UI::Button::VisualState init_state,
    ImageMode mode)
   : Button(parent, name, x, y, w, h, init_style, title_image, "", tooltip_text, init_state, mode) {
}

/**
 * Sets a new picture for the Button.
 */
void Button::set_pic(const Image* pic) {
	title_.clear();

	if (title_image_ == pic) {
		return;
	}

	title_image_ = pic;
}

void Button::expand(int w, int h) {
	if (h == 0) {
		// Automatically resize for font height and give it a margin.
		int new_width = get_w();
		const int new_height = std::max(text_height(button_style().enabled().font()),
		                                text_height(button_style().disabled().font())) +
		                       4 * kButtonImageMargin;
		if (w == 0) {
			// Automatically resize for text width too.
			new_width = std::max(text_width(title_, button_style().enabled().font()),
			                     text_width(title_, button_style().disabled().font())) +
			            8 * kButtonImageMargin;
		}
		set_desired_size(new_width, new_height);
		set_size(new_width, new_height);
	}
}

/**
 * Set a text title for the Button
 */
void Button::set_title(const std::string& title) {
	if (title_ == title) {
		return;
	}

	title_image_ = nullptr;
	title_ = title;
}

/**
 * Enable/Disable the button (disabled buttons can't be clicked).
 * Buttons are enabled by default
 */
void Button::set_enabled(bool const on) {
	if (enabled_ == on) {
		return;
	}

	// disabled buttons should look different...
	if (on) {
		enabled_ = true;
	} else {
		if (pressed_) {
			pressed_ = false;
			set_thinks(false);
			grab_mouse(false);
		}
		enabled_ = false;
		highlighted_ = false;
	}
	set_can_focus(enabled_);
}

std::vector<Recti> Button::focus_overlay_rects() {
	return Panel::focus_overlay_rects(2, 2, 0);
}

/**
 * Redraw the button
 */
void Button::draw(RenderTarget& dst) {
	const bool is_flat = (enabled_ && visual_state_ == VisualState::kFlat) ||
	                     (!enabled_ &&
	                      // Needs explicit cast to int to make clang-tidy happy.
	                      (static_cast<int>(disable_style_ & ButtonDisableStyle::kFlat) != 0));
	const bool is_permpressed =
	   (enabled_ && visual_state_ == VisualState::kPermpressed) ||
	   (!enabled_ && (static_cast<int>(disable_style_ & ButtonDisableStyle::kPermpressed) != 0));
	const bool is_monochrome =
	   !enabled_ && (static_cast<int>(disable_style_ & ButtonDisableStyle::kMonochrome) != 0);

	const UI::TextPanelStyleInfo& style_to_use =
	   is_monochrome ? button_style().disabled() : button_style().enabled();

	// Draw the background
	draw_background(dst, style_to_use.background());

	if (is_flat && highlighted_) {
		dst.brighten_rect(Recti(0, 0, get_w(), get_h()), MOUSE_OVER_BRIGHT_FACTOR);
	}

	//  If we've got a picture, draw it centered
	if (title_image_ != nullptr) {
		if (image_mode_ == UI::Button::ImageMode::kUnscaled) {
			if (!is_monochrome) {
				dst.blit(Vector2i((get_w() - static_cast<int32_t>(title_image_->width())) / 2,
				                  (get_h() - static_cast<int32_t>(title_image_->height())) / 2),
				         title_image_);
			} else {
				dst.blit_monochrome(
				   Vector2i((get_w() - static_cast<int32_t>(title_image_->width())) / 2,
				            (get_h() - static_cast<int32_t>(title_image_->height())) / 2),
				   title_image_, RGBAColor(255, 255, 255, 127));
			}
		} else {
			const int max_image_w = get_w() - 2 * kButtonImageMargin;
			const int max_image_h = get_h() - 2 * kButtonImageMargin;
			const float image_scale =
			   std::min(1.f, std::min(static_cast<float>(max_image_w) / title_image_->width(),
			                          static_cast<float>(max_image_h) / title_image_->height()));
			int blit_width = image_scale * title_image_->width();
			int blit_height = image_scale * title_image_->height();

			if (!is_monochrome) {
				dst.blitrect_scale(Rectf((get_w() - blit_width) / 2.f, (get_h() - blit_height) / 2.f,
				                         blit_width, blit_height),
				                   title_image_,
				                   Recti(0, 0, title_image_->width(), title_image_->height()), 1.,
				                   BlendMode::UseAlpha);
			} else {
				dst.blitrect_scale_monochrome(
				   Rectf((get_w() - blit_width) / 2.f, (get_h() - blit_height) / 2.f, blit_width,
				         blit_height),
				   title_image_, Recti(0, 0, title_image_->width(), title_image_->height()),
				   RGBAColor(255, 255, 255, 127));
			}
		}

	} else if (!title_.empty()) {
		//  Otherwise draw title string centered
		std::shared_ptr<const UI::RenderedText> rendered_text = autofit_text(
		   richtext_escape(title_), style_to_use.font(), get_inner_w() - 2 * kButtonImageMargin);

		// Blit on pixel boundary (not float), so that the text is blitted pixel perfect.
		rendered_text->draw(dst, Vector2i((get_w() - rendered_text->width()) / 2,
		                                  (get_h() - rendered_text->height()) / 2));
	}

	//  draw border
	//  a pressed but not highlighted button occurs when the user has pressed
	//  the left mouse button and then left the area of the button or the button
	//  stays pressed when it is pressed once
	RGBAColor black(0, 0, 0, 255);

	if (!is_flat) {
		assert(2 <= get_w());
		assert(2 <= get_h());
		//  Button is a normal one, not flat. We invert the behaviour for kPermpressed.
		if (is_permpressed == (pressed_ && highlighted_)) {
			//  top edge
			dst.brighten_rect(Recti(0, 0, get_w(), 2), BUTTON_EDGE_BRIGHT_FACTOR);
			//  left edge
			dst.brighten_rect(Recti(0, 2, 2, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
			//  bottom edge
			dst.fill_rect(Recti(2, get_h() - 2, get_w() - 2, 1), black);
			dst.fill_rect(Recti(1, get_h() - 1, get_w() - 1, 1), black);
			//  right edge
			dst.fill_rect(Recti(get_w() - 2, 2, 1, get_h() - 2), black);
			dst.fill_rect(Recti(get_w() - 1, 1, 1, get_h() - 1), black);
		} else {
			//  bottom edge
			dst.brighten_rect(Recti(0, get_h() - 2, get_w(), 2), BUTTON_EDGE_BRIGHT_FACTOR);
			//  right edge
			dst.brighten_rect(Recti(get_w() - 2, 0, 2, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
			//  top edge
			dst.fill_rect(Recti(0, 0, get_w() - 1, 1), black);
			dst.fill_rect(Recti(0, 1, get_w() - 2, 1), black);
			//  left edge
			dst.fill_rect(Recti(0, 0, 1, get_h() - 1), black);
			dst.fill_rect(Recti(1, 0, 1, get_h() - 2), black);
		}
	} else {
		//  Button is flat, do not draw borders, instead, if it is pressed, draw
		//  a box around it.
		if (enabled_ && highlighted_) {
			RGBAColor shade(100, 100, 100, 80);
			dst.fill_rect(Recti(0, 0, get_w(), 2), shade);
			dst.fill_rect(Recti(0, 2, 2, get_h() - 2), shade);
			dst.fill_rect(Recti(0, get_h() - 2, get_w(), get_h()), shade);
			dst.fill_rect(Recti(get_w() - 2, 0, get_w(), get_h()), shade);
		}
	}
}

void Button::think() {
	if (!repeating_ || !pressed_) {
		// race condition during initialization
		return;
	}

	Panel::think();

	if (highlighted_) {
		uint32_t const time = SDL_GetTicks();
		if (time_nextact_ <= time) {
			time_nextact_ += MOUSE_BUTTON_AUTOREPEAT_TICK;  //  schedule next tick
			if (time_nextact_ < time) {
				time_nextact_ = time;
			}
			play_click();
			sigclicked();
			//  The button may not exist at this point (for example if the button
			//  closed the dialog that it is part of). So member variables may no
			//  longer be accessed.
		}
	}
}

bool Button::handle_key(bool down, SDL_Keysym code) {
	if (down && code.sym == SDLK_SPACE) {
		play_click();
		sigclicked();
		return true;
	}
	return NamedPanel::handle_key(down, code);
}

/**
 * Update highlighted status
 */
void Button::handle_mousein(bool const inside) {
	bool oldhl = highlighted_;

	highlighted_ = inside && enabled_;

	if (oldhl == highlighted_) {
		return;
	}

	if (highlighted_) {
		sigmousein();
	} else {
		sigmouseout();
	}
}

/**
 * Update the pressed status of the button
 */
bool Button::handle_mousepress(uint8_t const btn, int32_t /*x*/, int32_t /*y*/) {
	if (btn != SDL_BUTTON_LEFT) {
		return false;
	}

	if (enabled_) {
		grab_mouse(true);
		pressed_ = true;
		if (repeating_) {
			time_nextact_ = SDL_GetTicks() + MOUSE_BUTTON_AUTOREPEAT_DELAY;
			set_thinks(true);
		}
	}
	return true;
}

bool Button::handle_mouserelease(uint8_t const btn, int32_t /*x*/, int32_t /*y*/) {
	if (btn != SDL_BUTTON_LEFT) {
		return false;
	}

	if (pressed_) {
		pressed_ = false;
		set_thinks(false);
		grab_mouse(false);
		if (highlighted_ && enabled_) {
			play_click();
			sigclicked();
			//  The button may not exist at this point (for example if the button
			//  closed the dialog that it is part of). So member variables may no
			//  longer be accessed.
		}
		return true;
	}
	return false;
}

bool Button::handle_mousemove(
   const uint8_t /*state*/, int32_t /*x*/, int32_t /*y*/, int32_t /*xdiff*/, int32_t /*ydiff*/) {
	return true;  // We handle this always by lighting up
}

void Button::set_visual_state(UI::Button::VisualState input_state) {
	visual_state_ = input_state;
}

void Button::set_disable_style(UI::ButtonDisableStyle input_style) {
	disable_style_ = input_style;
}

void Button::set_perm_pressed(bool pressed) {
	set_visual_state(pressed ? UI::Button::VisualState::kPermpressed :
                              UI::Button::VisualState::kRaised);
}

void Button::set_style(UI::ButtonStyle bstyle) {
	button_style_ = bstyle;
}

inline const UI::ButtonStyleInfo& Button::button_style() const {
	return g_style_manager->button_style(button_style_);
}

void Button::toggle() {
	switch (visual_state_) {
	case UI::Button::VisualState::kRaised:
		visual_state_ = UI::Button::VisualState::kPermpressed;
		break;
	case UI::Button::VisualState::kPermpressed:
		visual_state_ = UI::Button::VisualState::kRaised;
		break;
	case UI::Button::VisualState::kFlat:
		break;  // Do nothing for flat buttons
	}
}
}  // namespace UI
