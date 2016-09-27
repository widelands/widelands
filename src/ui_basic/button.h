/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

/// This is simply a button. Override void clicked() to react to the click.
/// This is all that is needed in most cases, but if there is a need to give a
/// callback function to the button, there are some templates for that below.
struct Button : public NamedPanel {
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
	    bool const enabled = true,
	    bool const flat = false);
	// TODO(GunChleoc): We have a lot of bools here. Introduce an enum class.
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
	    bool const enabled = true,
	    bool const flat = false,
	    bool const keep_image_size = false);
	~Button();

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

	// Set the permanently pressed state of the button
	void set_perm_pressed(bool state);
	bool get_perm_pressed() const {
		return permpressed_;
	}

	// Set button to flat / not flat
	void set_flat(bool flat);
	// If no background is drawn, the button is drawn over the current background
	void set_draw_flat_background(bool set);

	boost::signals2::signal<void()> sigclicked;
	boost::signals2::signal<void()> sigmousein;
	boost::signals2::signal<void()> sigmouseout;

protected:
	virtual void clicked() {
	}  /// Override this to react on the click.

	bool highlighted_;  //  mouse is over the button
	bool pressed_;      //  mouse is clicked over the button
	bool permpressed_;  //  button should appear  pressed
	bool enabled_;
	bool repeating_;
	bool flat_;
	bool keep_image_size_;  // Keep image's original size and center it
	bool draw_flat_background_;

	uint32_t time_nextact_;

	std::string title_;  //  title string used when pic_custom_ == 0

	const Image* pic_background_;  //  background texture (picture ID)
	const Image* pic_custom_;      //  custom icon on the button

	RGBColor clr_down_;  //  color of border while a flat button is "down"
};

}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_BUTTON_H
