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
 */

#ifndef WL_UI_BASIC_SLIDER_H
#define WL_UI_BASIC_SLIDER_H

#include "graphic/styles/text_panel_style.h"
#include "ui_basic/panel.h"

namespace UI {

struct DiscreteSlider;

/**
 * \brief This class defines a generic slide bar.
 *
 * The callbacks for the slider value are done by two signal instances.
 */
class Slider : public Panel {

	friend struct DiscreteSlider;

protected:
	/**
	 * Text conventions: Sentence case for the 'tooltip_text'
	 */
	Slider(Panel* parent,
	       int32_t x,
	       int32_t y,
	       uint32_t w,
	       uint32_t h,
	       int32_t min_value,
	       int32_t max_value,
	       int32_t value,
	       UI::SliderStyle style,
	       const std::string& tooltip_text,
	       uint32_t cursor_size,
	       bool enabled,
	       int32_t x_gap,
	       int32_t y_gap,
	       int32_t bar_size);

public:
	bool is_snap_target() const override {
		return true;
	}

	int32_t get_value() const {
		return value_;
	}
	int32_t get_max_value() const {
		return max_value_;
	}
	int32_t get_min_value() const {
		return min_value_;
	}

	void set_value(int32_t);
	void set_max_value(int32_t);
	void set_min_value(int32_t);

	void set_enabled(bool enabled);

	bool handle_key(bool, SDL_Keysym) override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;

	void set_cursor_fixed_height(int32_t h) {
		cursor_fixed_height_ = h;
	}

protected:
	void layout() override;
	void calculate_cursor_position();

	//  drawing
	int32_t get_x_gap() const {
		return x_gap_;
	}
	int32_t get_y_gap() const {
		return y_gap_;
	}
	int32_t get_bar_size() const {
		return bar_size_;
	}
	void draw_cursor(RenderTarget&, int32_t x, int32_t y, int32_t w, int32_t h);

	//  mouse events
	bool handle_mouserelease(uint8_t btn, int32_t, int32_t) override;
	void handle_mousein(bool inside) override;
	void cursor_moved(int32_t pointer, int32_t x, int32_t y);
	void cursor_pressed(int32_t pointer);
	void bar_pressed(int32_t pointer, int32_t ofs);

private:
	void send_value_changed();
	void set_highlighted(bool highlighted);

public:
	Notifications::Signal<> changed;
	Notifications::Signal<int32_t> changedto;

private:
	int32_t min_value_;  //  cursor values
	int32_t max_value_;
	int32_t value_;
	int32_t relative_move_;

	bool highlighted_;  //  mouse over
	bool pressed_;      //  the cursor is pressed
	bool enabled_;      //  enabled widget

	const UI::SliderStyle cursor_style_;  // Cursor color and texture. Not owned.
	const UI::PanelStyleInfo& cursor_style() const;

protected:
	int32_t x_gap_;  //  draw positions
	int32_t y_gap_;
	int32_t bar_size_;

	int32_t cursor_pos_;   //  cursor position
	int32_t cursor_size_;  //  cursor width
	int32_t cursor_fixed_height_;
};

/**
 * \brief This class defines an horizontal slide bar.
 */
struct HorizontalSlider : public Slider {
	HorizontalSlider(Panel* const parent,
	                 const int32_t x,
	                 const int32_t y,
	                 const uint32_t w,
	                 const uint32_t h,
	                 const int32_t min_value,
	                 const int32_t max_value,
	                 const int32_t value,
	                 UI::SliderStyle style,
	                 const std::string& tooltip_text = std::string(),
	                 const uint32_t cursor_size = 20,
	                 const bool enabled = true)
	   : Slider(parent,
	            x,
	            y,
	            w,
	            h,
	            min_value,
	            max_value,
	            value,
	            style,
	            tooltip_text,
	            cursor_size,
	            enabled,
	            cursor_size / 2,
	            h / 2 - 2,
	            w - cursor_size) {
	}

protected:
	void draw(RenderTarget& dst) override;
	bool handle_mousemove(uint8_t btn, int32_t x, int32_t y, int32_t, int32_t) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	void layout() override;
};

/**
 * \brief This class defines a verical slide bar.
 */
struct VerticalSlider : public Slider {
	VerticalSlider(Panel* const parent,
	               const int32_t x,
	               const int32_t y,
	               const uint32_t w,
	               const uint32_t h,
	               const int32_t min_value,
	               const int32_t max_value,
	               const int32_t value,
	               UI::SliderStyle style,
	               const uint32_t cursor_size = 20,
	               const std::string& tooltip_text = std::string(),
	               const bool enabled = true)
	   : Slider(parent,
	            x,
	            y,
	            w,
	            h,
	            min_value,
	            max_value,
	            value,
	            style,
	            tooltip_text,
	            cursor_size,
	            enabled,
	            w / 2 - 2,
	            cursor_size / 2,
	            h - cursor_size) {
	}

protected:
	void draw(RenderTarget& dst) override;
	bool handle_mousemove(uint8_t btn, int32_t x, int32_t y, int32_t, int32_t) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	void layout() override;
};

/**
 * \brief This class defines an discrete slide bar. We do not derive from
 * Slider, but rather embed it, as we need to re-size it and add the labels.
 */
struct DiscreteSlider : public Panel {
	DiscreteSlider(Panel* const parent,
	               const int32_t x,
	               const int32_t y,
	               const uint32_t w,
	               const uint32_t h,
	               const std::vector<std::string>& labels_in,
	               uint32_t value_,
	               UI::SliderStyle style,
	               const std::string& tooltip_text = std::string(),
	               const uint32_t cursor_size = 20,
	               const bool enabled = true);

	void set_labels(const std::vector<std::string>&);

	Notifications::Signal<> changed;
	Notifications::Signal<int32_t> changedto;

	Slider& get_slider() {
		return slider;
	}

protected:
	void draw(RenderTarget& dst) override;
	void layout() override;

private:
	// We need the style to initialize the slider, so it has to come first.
	const UI::SliderStyle style_;
	const UI::TextPanelStyleInfo& style() const;

protected:
	HorizontalSlider slider;

private:
	std::vector<std::string> labels;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_SLIDER_H
