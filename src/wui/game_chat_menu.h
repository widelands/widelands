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

#ifndef WL_WUI_GAME_CHAT_MENU_H
#define WL_WUI_GAME_CHAT_MENU_H

#include "ui_basic/unique_window.h"
#include "wui/game_chat_panel.h"

/**
 * Provides a window with chat message scrollback and the possibility to
 * enter and send chat messages.
 */
struct GameChatMenu : public UI::UniqueWindow {

	/**
	 * Creates a default chat window for multiplayer games
	 */
	static GameChatMenu* create_chat_console(UI::Panel*, UI::UniqueWindow::Registry&, ChatProvider&);

#ifndef NDEBUG  //  only in debug builds
	             /**
	              * Creates a chat window only for scripting in debug mode
	              */
	static GameChatMenu*
	create_script_console(UI::Panel*, UI::UniqueWindow::Registry&, ChatProvider&);
#endif

	/**
	 * Configure the menu so that it is useful for writing chat messages.
	 * Put the focus on the message entry field, close the menu automatically
	 * when return is pressed, etc.
	 */
	bool enter_chat_message(bool close_on_send = true);

private:
	GameChatMenu(UI::Panel*, UI::UniqueWindow::Registry&, ChatProvider&, const std::string& title);

	void restore() override;
	void minimize() override;
	void acknowledge();
	GameChatPanel chat_;
	bool close_on_send_;
};

#endif  // end of include guard: WL_WUI_GAME_CHAT_MENU_H
