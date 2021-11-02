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
 */

#include "ui_basic/slider.h"

#include <memory>

#include <SDL_mouse.h>

#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "ui_basic/mouse_constants.h"
#include "wlapplication_mousewheel_options.h"
#include "wlapplication_options.h"

namespace UI {

/**
 * \brief Initialize a Slider.
 *
 * \param parent The parent panel.
 * \param x The X position.
 * \param y The Y position.
 * \param w The widget width.
 * \param h The widget height.
 * \param min_value The minimal value accepted by the slider.
 * \param max_value The maximal value accepted by the slider.
 * \param value The initial value.
 * \param background_picture_id Identifier of the picture used for the
 * cursor background.
 * \param tooltip_text An optional help message.
 * \param cursor_size The size of the cursor, moving direction.
 * \param enabled The widget accessibility.
 * \param x_gap size between the cursor and the vertical border.
 * \param y_gap size between the cursor and the horizontal border.
 * \param bar_size Length of the cursor move.
 */
Slider::Slider(Panel* const parent,
               const int32_t x,
               const int32_t y,
               const uint32_t w,
               const uint32_t h,
               const int32_t min_value,
               const int32_t max_value,
               const int32_t value,
               const SliderStyle style,
               const std::string& tooltip_text,
               const uint32_t cursor_size,
               const bool enabled,
               const int32_t x_gap,
               const int32_t y_gap,
               const int32_t bar_size)
   : Panel(parent,
           style == SliderStyle::kFsMenu ? PanelStyle::kFsMenu : PanelStyle::kWui,
           x,
           y,
           w,
           h,
           tooltip_text),
     min_value_(min_value),
     max_value_(max_value),
     value_(value),
     relative_move_(0),
     highlighted_(false),
     pressed_(false),
     enabled_(enabled),
     cursor_style_(style),
     x_gap_(x_gap),
     y_gap_(y_gap),
     bar_size_(bar_size),
     cursor_size_(cursor_size),
     cursor_fixed_height_(-1) {
	set_thinks(false);
	set_can_focus(enabled_);
	calculate_cursor_position();
}

inline const UI::PanelStyleInfo& Slider::cursor_style() const {
	return g_style_manager->slider_style(cursor_style_).background();
}

void Slider::set_value(int32_t new_value) {
	new_value = std::max(min_value_, std::min(new_value, max_value_));

	if (new_value != value_) {
		value_ = new_value;
		calculate_cursor_position();
		send_value_changed();
	}
}

void Slider::calculate_cursor_position() {
	if (max_value_ == min_value_) {
		cursor_pos_ = min_value_;
	} else if (value_ == min_value_) {
		cursor_pos_ = 0;
	} else if (value_ == max_value_) {
		cursor_pos_ = get_bar_size();
	} else {
		cursor_pos_ = (value_ - min_value_) * get_bar_size() / (max_value_ - min_value_);
	}
}

void Slider::layout() {
	Panel::layout();
	calculate_cursor_position();
}

/**
 * \brief Sets max value.
 *
 * \param new_max The new max value.
 */
void Slider::set_max_value(int32_t new_max) {
	assert(min_value_ <= new_max);
	if (max_value_ != new_max) {
		calculate_cursor_position();
	}
	max_value_ = new_max;
	set_value(value_);
}

/**
 * \brief Sets min value.
 *
 * \param new_min The new min value.
 */
void Slider::set_min_value(int32_t new_min) {
	assert(max_value_ >= new_min);
	if (min_value_ != new_min) {
		calculate_cursor_position();
	}
	min_value_ = new_min;
	set_value(value_);
}

/**
 * \brief Draw the cursor.
 *
 * \param dst Graphic target.
 * \param x The cursor x position.
 * \param y The cursor y position.
 * \param w The cursor width.
 * \param h The cursor height.
 */
void Slider::draw_cursor(
   RenderTarget& dst, int32_t const x, int32_t const y, int32_t const w, int32_t const h) {

	RGBColor black(0, 0, 0);
	const Recti background_rect(x, y, w, h);

	draw_background(dst, background_rect, cursor_style());

	if (highlighted_) {
		dst.brighten_rect(background_rect, MOUSE_OVER_BRIGHT_FACTOR);
	}

	if (pressed_ || !enabled_) {
		dst.brighten_rect  //  bottom edge
		   (Recti(x, y + h - 2, w, 2), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect  //  right edge
		   (Recti(x + w - 2, y, 2, h - 2), BUTTON_EDGE_BRIGHT_FACTOR);

		//  top edge
		dst.fill_rect(Recti(x, y, w - 1, 1), black);
		dst.fill_rect(Recti(x, y + 1, w - 2, 1), black);

		//  left edge
		dst.fill_rect(Recti(x, y, 1, h - 1), black);
		dst.fill_rect(Recti(x + 1, y, 1, h - 2), black);
	} else {
		dst.brighten_rect  //  top edge
		   (Recti(x, y, w, 2), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect  //  left edge
		   (Recti(x, y + 2, 2, h - 2), BUTTON_EDGE_BRIGHT_FACTOR);

		//  bottom edge
		dst.fill_rect(Recti(x + 2, y + h - 2, w - 2, 1), black);
		dst.fill_rect(Recti(x + 1, y + h - 1, w - 1, 1), black);

		//  right edge
		dst.fill_rect(Recti(x + w - 2, y + 2, 1, h - 2), black);
		dst.fill_rect(Recti(x + w - 1, y + 1, 1, h - 1), black);
	}
}

/**
 * \brief Send an event when the slider is moved by used.
 */
void Slider::send_value_changed() {
	changed();
	changedto(value_);
}

/**
 * \brief Enable/Disable the slider.
 *
 * Disabled slider can't be clicked. Sliders are enabled by default.
 */
void Slider::set_enabled(const bool enabled) {
	// TODO(unknown): disabled should look different...
	if (enabled_ == enabled) {
		return;
	}

	enabled_ = enabled;
	if (!enabled) {
		pressed_ = false;
		highlighted_ = false;
		grab_mouse(false);
	}
	set_can_focus(enabled_);
}

/**
 * Set whether the sliding button should be highlighted.
 */
void Slider::set_highlighted(bool highlighted) {
	if (highlighted_ == highlighted) {
		return;
	}

	highlighted_ = highlighted;
}

bool Slider::handle_key(bool down, SDL_Keysym code) {
	if (down && enabled_) {
		switch (get_keyboard_change(code, false)) {
		case ChangeType::kPlus:
			set_value(get_value() + 1);
			return true;
		case ChangeType::kMinus:
			set_value(get_value() - 1);
			return true;
		case ChangeType::kSetMax:
			set_value(get_max_value());
			return true;
		case ChangeType::kSetMin:
			set_value(0);
			return true;
		default:
			break;
		}

		int32_t num = -1;
		if (code.sym >= SDLK_1 && code.sym <= SDLK_9) {
			num = code.sym - SDLK_1;
		}
		if (num >= 0) {
			constexpr int32_t max_num = 9 - 1;
			int32_t min = get_min_value();
			int32_t max = get_max_value();

			if (num == 0) {
				set_value(min);
			} else if (num == max_num) {
				set_value(max);
			} else if (max - min <= max_num) {
				set_value(min + num);
			} else {
				set_value(min + ((max - min) * num) / max_num);
			}
			return true;
		}
	}
	return Panel::handle_key(down, code);
}

bool Slider::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	if (!enabled_) {
		return false;
	}

	int32_t change = get_mousewheel_change(MousewheelHandlerConfigID::kChangeValue, x, y, modstate);
	if (change != 0) {
		set_value(get_value() + change);
		return true;
	}

	return false;
}

/**
 * \brief Mouse entered and exited events.
 *
 * Change the cursor style.
 */
void Slider::handle_mousein(bool inside) {
	if (!inside) {
		set_highlighted(false);
	}
}

/**
 * \brief Mouse released event.
 *
 * Update pressed status.
 */
bool Slider::handle_mouserelease(const uint8_t btn, int32_t, int32_t) {
	if (btn != SDL_BUTTON_LEFT) {
		return false;
	}
	if (pressed_) {
		grab_mouse(false);
		pressed_ = false;

		//  cursor position: align to integer value
		calculate_cursor_position();
		return true;
	}
	return false;
}

/**
 * \brief Update the value when the cursor is moved.
 *
 * \param pointer The relative position of the mouse pointer.
 * \param x The x position of the mouse pointer.
 * \param y The y position of the mouse pointer.
 */
void Slider::cursor_moved(int32_t pointer, int32_t x, int32_t y) {
	if (!enabled_) {
		return;
	}

	set_highlighted(pointer >= cursor_pos_ && pointer <= cursor_pos_ + cursor_size_ && y >= 0 &&
	                y < get_h() && x >= 0 && x < get_w());

	if (!pressed_) {
		return;
	}

	cursor_pos_ = pointer - relative_move_;
	if (cursor_pos_ < 0) {
		cursor_pos_ = 0;
	}
	if (cursor_pos_ > get_bar_size()) {
		cursor_pos_ = get_bar_size();
	}

	//  absolute value
	int32_t new_value = static_cast<int32_t>(
	   rint(static_cast<double>((max_value_ - min_value_) * cursor_pos_) / get_bar_size()));

	//  relative value in bounds
	new_value += min_value_;
	if (new_value < min_value_) {
		new_value = min_value_;
	}
	if (new_value > max_value_) {
		new_value = max_value_;
	}

	//  updating
	if (new_value != value_) {
		value_ = new_value;
		send_value_changed();
	}
}

/**
 * \brief Event when the cursor is presed.
 *
 * \param pointer The relative position of the mouse pointer.
 */
void Slider::cursor_pressed(int32_t pointer) {
	if (!enabled_) {
		return;
	}

	grab_mouse(true);
	pressed_ = true;
	highlighted_ = true;
	relative_move_ = pointer - cursor_pos_;

	clicked();
	play_click();
}

/**
 * \brief Event when the bar is pressed.
 *
 * \param pointer The relative position of the mouse pointer.
 * \param ofs The cursor offset.
 */
void Slider::bar_pressed(int32_t pointer, int32_t ofs) {
	if (!enabled_) {
		return;
	}

	grab_mouse(true);
	pressed_ = true;

	cursor_pos_ = pointer - ofs;

	//  absolute value
	if (get_bar_size() == 0) {
		value_ = 0;
	} else {
		value_ = static_cast<int32_t>(
		   rint(static_cast<double>((max_value_ - min_value_) * cursor_pos_) / get_bar_size()));
	}

	//  relative value in bounds
	if (value_ < min_value_) {
		value_ = min_value_;
	}
	if (value_ > max_value_) {
		value_ = max_value_;
	}

	play_click();
	send_value_changed();

	relative_move_ = ofs;
}

void VerticalSlider::layout() {
	x_gap_ = get_w() / 2 - 2;
	bar_size_ = get_h() - cursor_size_;
	Slider::layout();
}

/**
 * \brief Redraw the slide bar. The horizontal bar is painted.
 *
 * \param dst The graphic destination.
 */
void HorizontalSlider::draw(RenderTarget& dst) {
	RGBAColor black(0, 0, 0, 255);

	if (get_bar_size() > 0) {
		dst.brighten_rect  //  bottom edge
		   (Recti(get_x_gap(), get_h() / 2, get_bar_size(), 2), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect  //  right edge
		   (Recti(get_x_gap() + get_bar_size() - 2, get_y_gap(), 2, 2), BUTTON_EDGE_BRIGHT_FACTOR);

		//  top edge
		dst.fill_rect(Recti(get_x_gap(), get_y_gap(), get_bar_size() - 1, 1), black);
		dst.fill_rect(Recti(get_x_gap(), get_y_gap() + 1, get_bar_size() - 2, 1), black);
	}

	//  left edge
	dst.fill_rect(Recti(get_x_gap(), get_y_gap(), 1, 4), black);
	dst.fill_rect(Recti(get_x_gap() + 1, get_y_gap(), 1, 3), black);

	draw_cursor(dst, cursor_pos_,
	            cursor_fixed_height_ < 0 ? 0 : (get_h() - cursor_fixed_height_) / 2, cursor_size_,
	            cursor_fixed_height_ < 0 ? get_h() : cursor_fixed_height_);
}

/**
 * \brief Mouse move event
 *
 * \param btn The new stat of the mouse buttons (unused)
 * \param x The new X position of mouse pointer.
 * \param y The new Y position of mouse pointer.
 */
bool HorizontalSlider::handle_mousemove(
   uint8_t, int32_t const x, int32_t const y, int32_t, int32_t) {
	cursor_moved(x, x, y);
	return true;
}

/**
 * \brief Mouse pressed event.
 *
 * \param btn The new stat of the mouse buttons.
 * \param x The X position of mouse pointer.
 * \param y The Y position of mouse pointer.
 */
bool HorizontalSlider::handle_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT) {
		return false;
	}

	if (x >= cursor_pos_ && x <= cursor_pos_ + cursor_size_) {
		//  click on cursor
		cursor_pressed(x);
		return true;
	}
	if (y >= 0 && y < get_h() && x >= 0 && x < get_w()) {  //  click on bar
		bar_pressed(x, get_x_gap());
		return true;
	}
	return false;
}

void HorizontalSlider::layout() {
	y_gap_ = get_h() / 2 - 2;
	bar_size_ = get_w() - cursor_size_;
	Slider::layout();
}

/**
 * \brief Redraw the slide bar. The vertical bar is painted.
 *
 * \param dst The graphic destination.
 */
void VerticalSlider::draw(RenderTarget& dst) {
	RGBAColor black(0, 0, 0, 255);

	dst.brighten_rect  //  right edge
	   (Recti(get_w() / 2, get_y_gap(), 2, get_bar_size()), BUTTON_EDGE_BRIGHT_FACTOR);
	dst.brighten_rect  //  bottom edge
	   (Recti(get_x_gap(), get_y_gap() + get_bar_size() - 2, 2, 2), BUTTON_EDGE_BRIGHT_FACTOR);

	//  left edge
	dst.fill_rect(Recti(get_x_gap(), get_y_gap(), 1, get_bar_size() - 1), black);
	dst.fill_rect(Recti(get_x_gap() + 1, get_y_gap(), 1, get_bar_size() - 2), black);

	//  top edge
	dst.fill_rect(Recti(get_x_gap(), get_y_gap(), 4, 1), black);
	dst.fill_rect(Recti(get_x_gap(), get_y_gap() + 1, 3, 1), black);

	draw_cursor(dst, cursor_fixed_height_ < 0 ? 0 : (get_w() - cursor_fixed_height_) / 2,
	            cursor_pos_, cursor_fixed_height_ < 0 ? get_w() : cursor_fixed_height_,
	            cursor_size_);
}

/**
 * \brief Mouse move event
 *
 * \param btn The new stat of the mouse buttons (unused)
 * \param x The new X position of mouse pointer.
 * \param y The new Y position of mouse pointer.
 */
bool VerticalSlider::handle_mousemove(uint8_t, int32_t const x, int32_t const y, int32_t, int32_t) {
	cursor_moved(y, x, y);
	return true;
}

/**
 * \brief Mouse pressed event.
 *
 * \param btn The new stat of the mouse buttons (unused)
 * \param x The X position of mouse pointer.
 * \param y The Y position of mouse pointer.
 */
bool VerticalSlider::handle_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT) {
		return false;
	}

	if (y >= cursor_pos_ && y <= cursor_pos_ + cursor_size_) {
		//  click on cursor
		cursor_pressed(y);
		return true;
	}
	if (y >= get_y_gap() && y <= static_cast<int32_t>(get_h()) - get_y_gap() &&
	    x >= get_x_gap() - 2 &&
	    x < static_cast<int32_t>(get_w()) - get_x_gap() + 2) {  //  click on bar
		bar_pressed(y, get_y_gap());
		return true;
	}
	return false;
}

DiscreteSlider::DiscreteSlider(Panel* const parent,
                               const int32_t x,
                               const int32_t y,
                               const uint32_t w,
                               const uint32_t h,
                               const std::vector<std::string>& labels_in,
                               uint32_t init_value,
                               const SliderStyle init_style,
                               const std::string& tooltip_text,
                               const uint32_t cursor_size,
                               const bool enabled)
   : Panel(parent,
           init_style == SliderStyle::kFsMenu ? PanelStyle::kFsMenu : PanelStyle::kWui,
           x,
           y,
           w,
           h,
           tooltip_text),
     style_(init_style),
     slider(this,
            // here, we take into account the h_gap introduced by HorizontalSlider
            w / (2 * labels_in.size()) - cursor_size / 2,
            0,
            w - (w / labels_in.size()) + cursor_size,
            h - text_height(style().font()) - 2,
            0,
            labels_in.size() - 1,
            init_value,
            init_style,
            tooltip_text,
            cursor_size,
            enabled),
     labels(labels_in) {
	slider.changed.connect(changed);
	slider.changedto.connect(changedto);
}

inline const UI::TextPanelStyleInfo& DiscreteSlider::style() const {
	return g_style_manager->slider_style(style_);
}

/**
 * \brief Redraw the slide bar. The discrete horizontal bar is painted.
 *
 * \param dst The graphic destination.
 */
void DiscreteSlider::draw(RenderTarget& dst) {
	Panel::draw(dst);

	uint32_t gap_1 = get_w() / (2 * labels.size());
	uint32_t gap_n = get_w() / labels.size();

	for (uint32_t i = 0; i < labels.size(); i++) {
		std::shared_ptr<const UI::RenderedText> rendered_text =
		   UI::g_fh->render(as_richtext_paragraph(labels[i], style().font()));
		rendered_text->draw(
		   dst, Vector2i(gap_1 + i * gap_n, get_h() - rendered_text->height()), UI::Align::kCenter);
	}
}

void DiscreteSlider::set_labels(const std::vector<std::string>& labels_in) {
	labels = labels_in;
	slider.set_max_value(labels_in.size() - 1);
	layout();
}

void DiscreteSlider::layout() {
	uint32_t w = get_w();
	uint32_t h = get_h();
	assert(!labels.empty());
	slider.set_pos(Vector2i(w / (2 * labels.size()) - slider.cursor_size_ / 2, 0));
	slider.set_size(
	   w - (w / labels.size()) + slider.cursor_size_, h - text_height(style().font()) + 2);
	Panel::layout();
}
}  // namespace UI
