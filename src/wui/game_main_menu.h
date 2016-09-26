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

#ifndef WL_WUI_GAME_MAIN_MENU_H
#define WL_WUI_GAME_MAIN_MENU_H

#include "ui_basic/button.h"
#include "wui/interactive_player.h"

// The GameMainMenu is a rather dumb window with lots of buttons
struct GameMainMenu : public UI::UniqueWindow {
	GameMainMenu(InteractivePlayer&,
	             UI::UniqueWindow::Registry&,
	             InteractivePlayer::GameMainMenuWindows&);

	~GameMainMenu();

private:
	/// Adds a button to the menu that will toggle its window
	/// \param image_basename:      File path for button image starting from 'images' and without
	///                             file extension
	/// \param name:                Internal name of the button
	/// \param tooltip:             The button tooltip
	/// \param window:              The window that's associated with this button.
	UI::Button* add_button(const std::string& image_basename,
	                       const std::string& name,
	                       const std::string& tooltip,
	                       UI::UniqueWindow::Registry* window);

	InteractivePlayer& player_;
	InteractivePlayer::GameMainMenuWindows& windows_;
	UI::Box box_;

	// These get collected by add_button
	// so we can call unassign_toggle_button on them in the destructor.
	std::vector<UI::UniqueWindow::Registry> registries_;
};

#endif  // end of include guard: WL_WUI_GAME_MAIN_MENU_H
