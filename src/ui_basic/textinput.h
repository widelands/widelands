/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_TEXTINPUT_H
#define WL_UI_BASIC_TEXTINPUT_H

#include <memory>

#include "ui_basic/panel.h"

namespace UI {

/**
 * A panel that allows entering a single- or multi-line string.
 *
 * Text conventions: Sentence case for labels associated with the text input field.
 */
class AbstractTextInputPanel : public Panel {
public:
	~AbstractTextInputPanel() override;

	Notifications::Signal<> changed;
	Notifications::Signal<> cancel;

	void set_text(const std::string&);
	[[nodiscard]] const std::string& get_text() const;

	void focus(bool topcaller = true) override;

	void layout() override;

	[[nodiscard]] bool has_selection() const;
	[[nodiscard]] std::string get_selected_text();
	void replace_selected_text(const std::string&);
	[[nodiscard]] size_t get_caret_pos() const;
	void set_caret_pos(size_t) const;
	void select_until(uint32_t end) const;

	void set_password(bool password);
	[[nodiscard]] bool is_password() const;

	void set_warning(bool warn) {
		warning_ = warn;
	}
	[[nodiscard]] bool has_warning() const {
		return warning_;
	}

protected:
	AbstractTextInputPanel(
	   UI::Panel*, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style);

	void draw(RenderTarget&) override;

	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mousemove(uint8_t state, int32_t x, int32_t, int32_t, int32_t) override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;
	bool handle_key(bool down, SDL_Keysym) override;
	bool handle_textinput(const std::string& text) override;

	void scrollpos_changed(int32_t);
	void delete_selected_text() const;
	void copy_selected_text() const;
	void update_primary_selection_buffer() const;

	void set_caret_to_cursor_pos(int32_t x, int32_t y);
	int calculate_text_width(const std::string& text, int pos) const;
	int
	approximate_cursor(const std::string& line, int32_t cursor_pos_x, int approx_caret_pos) const;

	[[nodiscard]] virtual uint32_t max_text_width_for_wrap() const;
	virtual void scroll_cursor_into_view();
	virtual void escape_illegal_characters() const;
	[[nodiscard]] virtual bool should_expand_selection() const {
		return false;
	}

	struct Data;
	std::unique_ptr<Data> d_;

	bool warning_{false};

	uint32_t multiclick_timer_{0U};
	uint32_t multiclick_counter_{0U};
};

/** Subclass for single-line text input. */
class EditBox : public AbstractTextInputPanel {
public:
	static constexpr unsigned kHistorySize = 16;

	EditBox(UI::Panel* parent, int32_t x, int32_t y, uint32_t w, UI::PanelStyle style);

	Notifications::Signal<> ok;

	void activate_history(bool activate) {
		history_active_ = activate;
	}

protected:
	bool handle_key(bool down, SDL_Keysym) override;
	uint32_t max_text_width_for_wrap() const override;
	void scroll_cursor_into_view() override;
	void escape_illegal_characters() const override;
	[[nodiscard]] bool should_expand_selection() const override {
		return true;
	}

	bool history_active_{false};
	int16_t history_position_{-1};
	std::string history_[kHistorySize];
};

/** Subclass for multi-line text input. */
class MultilineEditbox : public AbstractTextInputPanel {
public:
	MultilineEditbox(
	   UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style);
};

}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_TEXTINPUT_H
