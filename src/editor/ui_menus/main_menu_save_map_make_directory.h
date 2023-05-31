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

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_SAVE_MAP_MAKE_DIRECTORY_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_SAVE_MAP_MAKE_DIRECTORY_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"
#include "ui_basic/textinput.h"
#include "ui_basic/window.h"

/**
 * Show a small modal dialog allowing the user to enter
 * a directory name to be created
 *
 */
// TODO(unknown): This should be moved to src/ui, it's not specific to the editor
struct MainMenuSaveMapMakeDirectory : public UI::Window {
	MainMenuSaveMapMakeDirectory(UI::Panel*, char const*);

	void start() override;

	char const* get_dirname() {
		return dirname_.c_str();
	}

private:
	static constexpr int padding_{5};
	const int butw_;
	static constexpr int buth_{20};
	std::string dirname_;
	UI::Box vbox_;
	UI::Textarea label_;
	UI::EditBox edit_;
	UI::Button ok_button_;
	UI::Button cancel_button_;
	const std::string illegal_filename_tooltip_;
	void edit_changed();
	void clicked_ok();
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_SAVE_MAP_MAKE_DIRECTORY_H
