/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "building_statistics_menu.h"
#include "game_options_menu.h"
#include "general_statistics_menu.h"
#include "i18n.h"
#include "interactive_player.h"
#include "network.h"
#include "player.h"
#include "stock_menu.h"

#define CHAT_MSG_WAIT_TIME 1000

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
 NetGame * netgame)
:
UI::UniqueWindow
(&plr, &registry,
 340, 5 + 150 + 5 + 85 + 5 + STATEBOX_HEIGHT + 5,
 _("Chat Menu")),
m_player(plr),
m_netgame(netgame),
chatbox(this,  5,   5, get_inner_w() - 10,             150, "", Align_Left, 1),
editbox(this,  5, 160, get_inner_w() - 10,              85, ""),

send
(this,
 5, 250, 80, STATEBOX_HEIGHT,
 4,
 &GameChatMenu::clicked_send, this,
 _("Send")),

show_as_overlays_label
(this,
 5 + 80 + 5, 250, 120, STATEBOX_HEIGHT,
 _("Show messages as overlays:"), Align_CenterLeft),
show_as_overlays(this, get_inner_w() - STATEBOX_WIDTH - 5, 250)
{

	show_as_overlays.changedto.set
		(this, &GameChatMenu::changed_show_as_overlays);

	show_as_overlays.set_state(m_player.show_chat_overlay());

	if (get_usedefaultpos())
		center_to_parent();

   think();
}


/*
 * think: updates the chat area
 */
void GameChatMenu::think() {
	const std::vector<NetGame::Chat_Message>* msges = m_player.get_chatmsges();
   std::string str;

	for (uint32_t i = 0; i < msges->size(); ++i) {
		str += m_player.get_game()->player((*msges)[i].plrnum).get_name();
      str += ": ";
      str += (*msges)[i].msg;
      str += "\n";
	}

	chatbox.set_text(str.c_str());
}

/*
 * Checkbox has been changed
 */
void GameChatMenu::changed_show_as_overlays(bool t)
{m_player.set_show_chat_overlay(t);}

void GameChatMenu::clicked_send() {
	std::string str = editbox.get_text();

   if (str.size() && m_netgame) {
      NetGame::Chat_Message t;

      t.plrnum = m_player.get_player_number();
      t.msg = str;
      m_netgame->send_chat_message(t);
      editbox.set_text("");
	}
}
