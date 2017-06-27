/*
 * Copyright (C) 2003-2017 by the Widelands Development Team
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

#include <SDL_keyboard.h>
#include <boost/signals2.hpp>

#include "graphic/align.h"
#include "ui_basic/button.h"

#define CHAT_HISTORY_SIZE 5

namespace UI {

struct EditBoxImpl;

/** An editbox can be clicked, then the user can change its text (title).
 *
 * When return is pressed, the editbox is unfocused, the keyboard
 * released and a callback function is called.
 * If h == 0, height will be dynamic according to font set.
 * If h > 0, margin_y has no effect.
 */
struct EditBox : public Panel {
	EditBox(Panel*,
	        int32_t x,
	        int32_t y,
	        uint32_t w,
	        uint32_t h,
	        int margin_y,
	        UI::PanelStyle style,
	        int font_size = UI_FONT_SIZE_SMALL);
	virtual ~EditBox();

	boost::signals2::signal<void()> changed;
	boost::signals2::signal<void()> ok;
	boost::signals2::signal<void()> cancel;

	const std::string& text() const;
	void set_text(const std::string&);
	void set_max_length(uint32_t);

	void activate_history(bool activate) {
		history_active_ = activate;
	}

	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_key(bool down, SDL_Keysym) override;
	bool handle_textinput(const std::string& text) override;

	void draw(RenderTarget&) override;

private:
	std::unique_ptr<EditBoxImpl> m_;

	void check_caret();

	bool history_active_;
	int16_t history_position_;
	std::string history_[CHAT_HISTORY_SIZE];
};
}

#endif  // end of include guard: WL_UI_BASIC_EDITBOX_H
