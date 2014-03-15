/*
 * Copyright (C) 2002-2004, 2006-2008, 2011 by the Widelands Development Team
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

#include "wui/game_chat_menu.h"

#include "i18n.h"

/*
==============================================================================

GameChatMenu IMPLEMENTATION

==============================================================================
*/

GameChatMenu::GameChatMenu
	(UI::Panel                  * parent,
	 UI::UniqueWindow::Registry & registry,
	 ChatProvider               & chat)
:
UI::UniqueWindow(parent, "chat", &registry, 440, 235, _("Chat")),
m_chat(this, 5, 5, get_inner_w() - 10, get_inner_h() - 10, chat)
{
	if (get_usedefaultpos())
		center_to_parent();

	m_close_on_send = false;

	m_chat.sent.connect(boost::bind(&GameChatMenu::acknowledge, this));
	m_chat.aborted.connect(boost::bind(&GameChatMenu::acknowledge, this));

	enter_chat_message(m_close_on_send);
}


void GameChatMenu::enter_chat_message(bool close_on_send)
{
	m_chat.focusEdit();
	m_close_on_send = close_on_send;
}


void GameChatMenu::acknowledge()
{
	if (m_close_on_send)
		die();
}
