/*
 * Copyright (C) 2021-2023 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_TEXT_PROMPT_H
#define WL_UI_BASIC_TEXT_PROMPT_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/textinput.h"
#include "ui_basic/window.h"

namespace UI {

/** A compact modal dialog with a caption, an editbox, and optionally more widgets. */
class TextPrompt : public UI::Window {
public:
	explicit TextPrompt(UI::Panel& parent,
	                    UI::WindowStyle,
	                    const std::string& title,
	                    const std::string& text);
	~TextPrompt() override = default;

	[[nodiscard]] const std::string& get_text() const {
		return editbox_.get_text();
	}

	/** Add custom content here, if desired. */
	[[nodiscard]] UI::Box& content_box() {
		return content_box_;
	}

	bool handle_key(bool down, SDL_Keysym code) override;

private:
	UI::Box box_, content_box_, buttonsbox_;
	UI::EditBox& editbox_;
	UI::Button ok_, cancel_;
};

}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_TEXT_PROMPT_H
