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
 *
 */

#ifndef WL_UI_BASIC_MULTILINEEDITBOX_H
#define WL_UI_BASIC_MULTILINEEDITBOX_H

#include <memory>

#include "ui_basic/panel.h"

namespace UI {

/**
 * A panel that allows entering multi-line string, i.e. like a hybrid between
 * @ref Editbox and @ref MultilineTextarea
 *
 * Text conventions: Sentence case for labels associated with thie editbox
 */
struct MultilineEditbox : public Panel {
	MultilineEditbox(Panel*, int32_t x, int32_t y, uint32_t w, uint32_t h, PanelStyle style);

	Notifications::Signal<> changed;

	const std::string& get_text() const;
	void set_text(const std::string&);

	void focus(bool topcaller = true) override;

	void layout() override;

	bool has_selection() const;
	std::string get_selected_text();
	void replace_selected_text(const std::string&);
	size_t get_caret_pos() const;
	void set_caret_pos(size_t) const;
	void select_until(uint32_t end) const;

protected:
	void draw(RenderTarget&) override;

	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mousemove(uint8_t state, int32_t x, int32_t, int32_t, int32_t) override;
	bool handle_key(bool down, SDL_Keysym) override;
	bool handle_textinput(const std::string& text) override;

private:
	void scrollpos_changed(int32_t);
	void delete_selected_text() const;
	void copy_selected_text() const;
	struct Data;
	std::unique_ptr<Data> d_;
	void set_caret_to_cursor_pos(int32_t x, int32_t y);
	int calculate_text_width(std::string& text, int pos) const;
	int approximate_cursor(std::string& line, int32_t cursor_pos_x, int approx_caret_pos) const;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_MULTILINEEDITBOX_H
