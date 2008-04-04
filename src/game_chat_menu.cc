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

#include "i18n.h"

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
	(UI::Panel                  * parent,
	 UI::UniqueWindow::Registry & registry,
	 ChatProvider               & chat)
:
UI::UniqueWindow(parent, &registry, 440, 235, _("Chat Menu")),
m_chat(this, 5, 5, get_inner_w() - 10, get_inner_h() - 10, chat)
{
	if (get_usedefaultpos())
		center_to_parent();

	close_on_send = false;

	m_chat.sent.set(this, &GameChatMenu::acknowledge);
	m_chat.aborted.set(this, &GameChatMenu::acknowledge);
}


void GameChatMenu::enter_chat_message()
{
	m_chat.focusEdit();
	close_on_send = true;
}


void GameChatMenu::acknowledge()
{
	if (close_on_send)
		die();
}
