/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_WUI_GAME_CLIENT_DISCONNECTED_H
#define WL_WUI_GAME_CLIENT_DISCONNECTED_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/unique_window.h"

class InteractiveGameBase;
class GameHost;

/**
 * Dialog that offers to replace a player with an AI or exit the game.
 * Even when the text only talks about a single player, actually all recently disconnected
 * players will be replaced by the taken choice.
 */
struct GameClientDisconnected : public UI::UniqueWindow {
	GameClientDisconnected(InteractiveGameBase*, UI::UniqueWindow::Registry&, GameHost*);

	void die() override;

private:
	void clicked_continue();
	void clicked_exit_game();
	void exit_game_aborted(UI::Panel*);

	void set_ai(const std::string& ai);

	InteractiveGameBase* igb_;
	GameHost* host_;

	UI::Box box_;
	UI::Box box_h_;
	UI::MultilineTextarea text_;
	UI::Button continue_;
	UI::Dropdown<std::string> type_dropdown_;
	UI::Button exit_game_;
};

#endif  // end of include guard: WL_WUI_GAME_CLIENT_DISCONNECTED_H
