/*
 * Copyright (C) 2003-2021 by the Widelands Development Team
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

#include <base/scoped_timer.h>
#include <memory>

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

	Notifications::Signal<> changed;
	Notifications::Signal<> ok;
	Notifications::Signal<> cancel;

	const std::string& text() const;
	void set_text(const std::string&);
	void set_max_length(int);
	void set_font_scale(float scale);

	void activate_history(bool activate) {
		history_active_ = activate;
	}

	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
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

	void set_caret_pos(size_t pos);
	size_t caret_pos() const;
	void focus(bool topcaller = true) override;

private:
	std::unique_ptr<EditBoxImpl> m_;

	void check_caret();
	void reset_selection();
	void highlight_selection(RenderTarget& dst, const Vector2i& point, uint16_t fontheight);
	void draw_caret(RenderTarget& dst, const Vector2i& point, uint16_t fontheight);
	std::string text_to_asterisk();

	bool history_active_;
	int16_t history_position_;
	std::string history_[CHAT_HISTORY_SIZE];
	bool password_;
	bool warning_;
	uint32_t snap_to_char(uint32_t cursor) const;
	void select_until(uint32_t end) const;
	uint32_t next_char(uint32_t cursor) const;
	uint32_t prev_char(uint32_t cursor) const;
	void calculate_selection_boundaries(uint32_t& start, uint32_t& end);
	void delete_selected_text();
	void copy_selected_text();
	void set_caret_to_cursor_pos(int32_t cursor_pos_x);
	int calculate_text_width(int pos) const;
	int approximate_cursor(int32_t cursor_pos_x, int approx_caret_pos) const;

	ScopedTimer caret_timer_;
	uint32_t caret_ms;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_EDITBOX_H
