/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_WUI_GAME_EXIT_CONFIRM_BOX_H
#define WL_WUI_GAME_EXIT_CONFIRM_BOX_H

#include "ui_basic/messagebox.h"
#include "wui/interactive_gamebase.h"

// The GameOptionsMenu is a rather dumb window with lots of buttons
struct GameOptionsMenu : public UI::UniqueWindow {
	GameOptionsMenu(InteractiveGameBase&,
	                UI::UniqueWindow::Registry&,
	                InteractiveGameBase::GameMainMenuWindows&);

	bool handle_key(bool down, SDL_Keysym code) override;


	~GameOptionsMenu();


private:
	InteractiveGameBase& igb_;
};

#endif  // end of include guard: WL_WUI_GAME_EXIT_CONFIRM_BOX_H
