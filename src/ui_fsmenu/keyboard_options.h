/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_KEYBOARD_OPTIONS_H
#define WL_UI_FSMENU_KEYBOARD_OPTIONS_H

#include <memory>

#include "logic/game.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/mousewheel_options.h"

namespace FsMenu {

class KeyboardOptions : public UI::Window {
public:
	explicit KeyboardOptions(Panel& parent);

	void layout() override;

	bool handle_key(bool, SDL_Keysym) override;

	WindowLayoutID window_layout_id() const override {
		return UI::Window::WindowLayoutID::kFsMenuKeyboardOptions;
	}

private:
	// TabPanels with scrolling content boxes do not layout properly
	// as box children. Therefore no main box here.
	UI::Box buttons_box_;
	UI::TabPanel tabs_;
	MousewheelOptionsDialog mousewheel_options_;
	UI::Button reset_, ok_;
	std::vector<UI::Box*> boxes_;

	std::unique_ptr<Widelands::Game> game_;
};
}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_KEYBOARD_OPTIONS_H
