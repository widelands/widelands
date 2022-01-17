/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_CHECKBOX_H
#define WL_UI_BASIC_CHECKBOX_H

#include <memory>

#include "graphic/text/rendered_text.h"
#include "ui_basic/panel.h"

constexpr int kStateboxSize = 20;

namespace UI {

/**
 * Virtual base class providing a box that can be checked or unchecked.
 * Serves as base for Checkbox and Radiobutton.
 */
struct Statebox : public Panel {

	/**
	 * Pictorial Statebox
	 */
	Statebox(Panel* parent,
	         PanelStyle,
	         Vector2i,
	         const Image* pic,
	         const std::string& tooltip_text = std::string());

	/**
	 * Textual Statebox
	 * If width is set to 0, the checkbox will set its width automatically.
	 * Otherwise, it will take up multiple lines if necessary (automatic height).
	 */
	Statebox(Panel* parent,
	         PanelStyle,
	         Vector2i,
	         const std::string& label_text,
	         const std::string& tooltip_text = std::string(),
	         int width = 0);

	Notifications::Signal<> changed;
	Notifications::Signal<bool> changedto;
	Notifications::Signal<bool> clickedto;  // same as changedto but only called when clicked

	void set_enabled(bool enabled);

	bool get_state() const {
		return flags_ & Is_Checked;
	}
	void set_state(bool on, bool send_signal = true);

	// Drawing and event handlers
	void draw(RenderTarget&) override;
	void draw_overlay(RenderTarget&) override;
	void update_template() override;

	void handle_mousein(bool inside) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mousemove(uint8_t, int32_t, int32_t, int32_t, int32_t) override;
	bool handle_key(bool, SDL_Keysym) override;

protected:
	std::vector<Recti> focus_overlay_rects() override;

private:
	void layout() override;
	virtual void button_clicked() = 0;

	enum Flags {
		Is_Highlighted = 0x01,
		Is_Enabled = 0x02,
		Is_Checked = 0x04,
		Has_Custom_Picture = 0x08,
		Has_Text = 0x10
	};
	uint8_t flags_;
	void set_flags(uint8_t const flags, bool const enable) {
		flags_ &= ~flags;
		if (enable) {
			flags_ |= flags;
		}
	}
	const Image* pic_graphics_;
	std::shared_ptr<const UI::RenderedText> rendered_text_;
	const std::string label_text_;
};

/**
 * A checkbox is a simplistic panel which consists of just a small box which
 * can be either checked (on) or unchecked (off)
 * A checkbox only differs from a Statebox in that clicking on it toggles the
 * state
 */
struct Checkbox : public Statebox {

	/**
	 * Pictorial Checkbox
	 *
	 * Text conventions: Sentence case for the 'tooltip_text'
	 */
	Checkbox(Panel* const parent,
	         PanelStyle s,
	         Vector2i const p,
	         const Image* pic,
	         const std::string& tooltip_text = std::string())
	   : Statebox(parent, s, p, pic, tooltip_text) {
	}

	/**
	 * Textual Checkbox
	 * If width is set to 0, the checkbox will set its width automatically.
	 * Otherwise, it will take up multiple lines if necessary (automatic height).
	 *
	 *
	 * Text conventions: Sentence case both for the 'label_text' and the 'tooltip_text'
	 */
	Checkbox(Panel* const parent,
	         PanelStyle s,
	         Vector2i const p,
	         const std::string& label_text,
	         const std::string& tooltip_text = std::string(),
	         uint32_t width = 0)
	   : Statebox(parent, s, p, label_text, tooltip_text, width) {
	}

private:
	void button_clicked() override;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_CHECKBOX_H
