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

#include "wui/game_chat_menu.h"

#include "base/i18n.h"

/*
==============================================================================

GameChatMenu IMPLEMENTATION

==============================================================================
*/

GameChatMenu::GameChatMenu(UI::Panel* parent,
                           ChatColorForPlayer fn,
                           UI::UniqueWindow::Registry& registry,
                           ChatProvider& chat,
                           const std::string& title)
   : UI::UniqueWindow(parent, UI::WindowStyle::kWui, "chat", &registry, 440, 235, title),
     chat_(this, fn, 5, 5, get_inner_w() - 10, get_inner_h() - 10, chat, UI::PanelStyle::kWui) {
	if (get_usedefaultpos()) {
		center_to_parent();
	}
	set_can_focus(true);

	chat_.sent.connect([this]() { acknowledge(); });
	chat_.aborted.connect([this]() { acknowledge(); });

	enter_chat_message(close_on_send_);

	initialization_complete();
}

GameChatMenu* GameChatMenu::create_chat_console(UI::Panel* parent,
                                                ChatColorForPlayer fn,
                                                UI::UniqueWindow::Registry& registry,
                                                ChatProvider& chat) {
	return new GameChatMenu(parent, fn, registry, chat, _("Chat"));
}

#ifdef SCRIPT_CONSOLE
GameChatMenu* GameChatMenu::create_script_console(UI::Panel* parent,
                                                  ChatColorForPlayer fn,
                                                  UI::UniqueWindow::Registry& registry,
                                                  ChatProvider& chat) {
	return new GameChatMenu(parent, fn, registry, chat, _("Script Console"));
}
#endif

bool GameChatMenu::enter_chat_message(bool close_on_send) {
	if (is_minimal()) {
		return false;
	}
	chat_.focus_edit();
	close_on_send_ = close_on_send;
	return true;
}

void GameChatMenu::restore() {
	Window::restore();
	chat_.focus_edit();
}

void GameChatMenu::minimize() {
	Window::minimize();
	chat_.unfocus_edit();
}

void GameChatMenu::acknowledge() {
	if (close_on_send_) {
		die();
	}
}
