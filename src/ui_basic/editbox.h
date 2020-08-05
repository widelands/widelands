/*
 * Copyright (C) 2003-2020 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_EDITBOX_H
#define WL_UI_BASIC_EDITBOX_H

#include <memory>

#include "graphic/styles/font_style.h"
#include "ui_basic/panel.h"

#define CHAT_HISTORY_SIZE 5

namespace UI {

struct EditBoxImpl;

/** An editbox can be clicked, then the user can change its text (title).
 *
 * When return is pressed, the editbox is unfocused, the keyboard
 * released and a callback function is called.
 * If h == 0, height will be dynamic according to font set.
 * If h > 0, margin_y has no effect.
 *
 * Text conventions: Sentence case for labels associated with the editbox
 */
struct EditBox : public Panel {
	EditBox(Panel*, int32_t x, int32_t y, uint32_t w, UI::PanelStyle style);
	~EditBox() override;

	boost::signals2::signal<void()> changed;
	boost::signals2::signal<void()> ok;
	boost::signals2::signal<void()> cancel;

	const std::string& text() const;
	void set_text(const std::string&);
	void set_max_length(int);
	void set_font_scale(float scale);
	void set_font_style(const UI::FontStyleInfo& style);
	void set_font_style_and_margin(const UI::FontStyleInfo& style, int margin);

	void activate_history(bool activate) {
		history_active_ = activate;
	}

	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_key(bool down, SDL_Keysym) override;
	bool handle_textinput(const std::string& text) override;

	void draw(RenderTarget&) override;

	void set_password(bool pass) {
		password_ = pass;
	}

	void set_warning(bool warn) {
		warning_ = warn;
	}

	bool has_warning() {
		return warning_;
	}

	bool is_password() {
		return password_;
	}

private:
	std::unique_ptr<EditBoxImpl> m_;

	void check_caret();
	std::string text_to_asterisk();

	bool history_active_;
	int16_t history_position_;
	std::string history_[CHAT_HISTORY_SIZE];
	bool password_;
	bool warning_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_EDITBOX_H
