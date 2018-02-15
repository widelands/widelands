/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_BUTTON_H
#define WL_UI_BASIC_BUTTON_H

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/signals2.hpp>

#include "graphic/color.h"
#include "graphic/text_layout.h"
#include "ui_basic/panel.h"

namespace UI {

struct Font;

enum class ButtonDisableStyle {
	kMonochrome = 2,   // Greyed out. Can be combined with the other 2 styles.
	kPermpressed = 4,  // Button will appear pressed.
	kFlat = 8,         // Button will appear flat.
};
inline ButtonDisableStyle operator&(ButtonDisableStyle a, ButtonDisableStyle b) {
	return static_cast<ButtonDisableStyle>(static_cast<int>(a) & static_cast<int>(b));
}
inline ButtonDisableStyle operator|(ButtonDisableStyle a, ButtonDisableStyle b) {
	return static_cast<ButtonDisableStyle>(static_cast<int>(a) | static_cast<int>(b));
}

/// This is simply a button. Override void clicked() to react to the click.
/// This is all that is needed in most cases, but if there is a need to give a
/// callback function to the button, there are some templates for that below.
struct Button : public NamedPanel {
	enum class Style {
		kRaised,       // Normal raised Button
		kPermpressed,  // Button will appear pressed
		kFlat          // Flat button with simple coloured outline
	};

	enum class ImageMode {
		kShrink,   // Shrink foreground image to fit into the button
		kUnscaled  // Show the foreground image without any scaling
	};

	/**
	 * Text conventions: Title Case for the 'title_text', Sentence case for the 'tooltip_text'
	 */
	Button  /// for textual buttons
	   (Panel* const parent,
	    const std::string& name,
	    int32_t const x,
	    int32_t const y,
	    uint32_t const w,
	    uint32_t const h,
	    const Image* background_picture_id,
	    const std::string& title_text,
	    const std::string& tooltip_text = std::string(),
	    UI::Button::Style init_style = UI::Button::Style::kRaised);

	/**
	 * Text conventions: Sentence case for the 'tooltip_text'
	 */
	Button  /// for pictorial buttons
	   (Panel* const parent,
	    const std::string& name,
	    const int32_t x,
	    const int32_t y,
	    const uint32_t w,
	    const uint32_t h,
	    const Image* background_picture_id,
	    const Image* foreground_picture_id,
	    const std::string& tooltip_text = std::string(),
	    UI::Button::Style init_style = UI::Button::Style::kRaised,
	    UI::Button::ImageMode mode = UI::Button::ImageMode::kShrink);
	~Button() override;

	void set_pic(const Image* pic);
	void set_title(const std::string&);
	const std::string& get_title() const {
		return title_;
	}

	bool enabled() const {
		return enabled_;
	}
	void set_enabled(bool on);
	void set_repeating(bool const on) {
		repeating_ = on;
	}
	bool is_snap_target() const override {
		return true;
	}

	// Drawing and event handlers
	void draw(RenderTarget&) override;
	void think() override;

	void handle_mousein(bool inside) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mousemove(uint8_t, int32_t, int32_t, int32_t, int32_t) override;

	/// Sets the visual style of the button
	void set_style(UI::Button::Style input_style);
	UI::Button::Style style() const {
		return style_;
	}

	/// Sets the visual style of the disabled button
	void set_disable_style(UI::ButtonDisableStyle input_style);

	/// Convenience function. If 'pressed', sets the style to kPermpressed, otherwise to kRaised.
	void set_perm_pressed(bool pressed);

	/// Convenience function. Toggles between raised and permpressed style
	void toggle();

	boost::signals2::signal<void()> sigclicked;
	boost::signals2::signal<void()> sigmousein;
	boost::signals2::signal<void()> sigmouseout;

protected:
	virtual void clicked() {
	}  /// Override this to react on the click.

	bool highlighted_;  //  mouse is over the button
	bool pressed_;      //  mouse is clicked over the button
	bool enabled_;
	UI::Button::Style style_;
	UI::ButtonDisableStyle disable_style_;
	bool repeating_;
	const UI::Button::ImageMode image_mode_;

	uint32_t time_nextact_;

	std::string title_;  //  title string used when pic_custom_ == 0

	const Image* pic_background_;  //  background texture (picture ID)
	const Image* pic_custom_;      //  custom icon on the button

	RGBColor clr_down_;  //  color of border while a flat button is "down"
};

}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_BUTTON_H
