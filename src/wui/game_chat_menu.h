/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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

#ifndef GAME_CHAT_MENU_H
#define GAME_CHAT_MENU_H

#include "wui/gamechatpanel.h"
#include "ui_basic/unique_window.h"


/**
 * Provides a window with chat message scrollback and the possibility to
 * enter and send chat messages.
 */
struct GameChatMenu : public UI::UniqueWindow {
	GameChatMenu(UI::Panel *, UI::UniqueWindow::Registry &, ChatProvider &);

	/**
	 * Configure the menu so that it is useful for writing chat messages.
	 * Put the focus on the message entry field, close the menu automatically
	 * when return is pressed, etc.
	 */
	void enter_chat_message(bool close_on_send = true);

private:
	void acknowledge();

	GameChatPanel m_chat;
	bool m_close_on_send;
};

#endif
