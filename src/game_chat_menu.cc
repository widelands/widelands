/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "game_chat_menu.h"

#include "chat.h"
#include "interactive_player.h"


/*
==============================================================================

GameChatMenu IMPLEMENTATION

==============================================================================
*/

/*
===============
GameChatMenu::GameChatMenu

Create all the buttons etc...
===============
*/
GameChatMenu::GameChatMenu
(Interactive_Player & plr,
 UI::UniqueWindow::Registry & registry,
 ChatProvider & chat)
:
UI::UniqueWindow
(&plr, &registry,
 440, 5 + 200 + 5 + 20 + 5,
 _("Chat Menu")),
m_chat(chat),
chatbox(this,  5,   5, get_inner_w() - 10, 200, "", Align_Left, 1),
editbox(this,  5, 210, get_inner_w() - 10,  20)
{
	if (get_usedefaultpos())
		center_to_parent();

	chatbox.set_scrollmode(UI::Multiline_Textarea::ScrollLog);
	editbox.ok.set(this, &GameChatMenu::keyEnter);
	editbox.cancel.set(this, &GameChatMenu::keyEscape);
	editbox.setAlign(Align_Left);
	close_on_send = false;

	connect(m_chat);
	recalculate();
}


void GameChatMenu::enter_chat_message()
{
	editbox.focus();
	close_on_send = true;
}


/**
 * Updates the chat message area.
 */
void GameChatMenu::recalculate()
{
	const std::vector<ChatMessage> msgs = m_chat.getMessages();

	std::string str;
	for (uint32_t i = 0; i < msgs.size(); ++i) {
		str += msgs[i].toPrintable();
		str += '\n';
	}

	chatbox.set_text(str);
}

void GameChatMenu::receive(const ChatMessage&)
{
	recalculate();
}


void GameChatMenu::keyEnter()
{
	const std::string& str = editbox.text();

	if (str.size())
		m_chat.send(str);

	editbox.setText("");
	if (close_on_send)
		die();
}

void GameChatMenu::keyEscape()
{
	editbox.setText("");
	if (close_on_send)
		die();
}
