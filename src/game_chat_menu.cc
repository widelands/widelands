/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include "building_statistics_menu.h"
#include "game_chat_menu.h"
#include "game_options_menu.h"
#include "general_statistics_menu.h"
#include "interactive_player.h"
#include "network.h"
#include "player.h"
#include "stock_menu.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_multilinetextarea.h"
#include "ui_multilineeditbox.h"
#include "ui_textarea.h"
#include "util.h"

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
GameChatMenu::GameChatMenu(Interactive_Player *plr, UIUniqueWindowRegistry *registry, NetGame* netgame)
	: UIUniqueWindow(plr, registry, 340, 160, _("Chat Menu"))
{
   m_player=plr;
   m_netgame = netgame;
   
   int spacing = 5;
   int posy = 35;
   
   // Caption
   new UITextarea(this, 0, spacing, get_inner_w(), 20, _("Chat Menu"), Align_Center);
  
   // What has been said? 
   m_chatbox = new UIMultiline_Textarea(this, spacing, posy, get_inner_w()-spacing*2, 150, "", Align_Left, 1); 
   
   posy += 150+spacing+spacing;
   m_editbox = new UIMultiline_Editbox(this, spacing, posy, get_inner_w()-spacing*2, 85, "");
   posy += 85 + spacing + spacing;

   // Send button
   UIButton* b = new UIButton(this, spacing, posy, 80, STATEBOX_HEIGHT, 0, 0);
   b->set_title(_("Send"));
   b->clickedid.set(this, &GameChatMenu::clicked);

   // Textbox
   new UITextarea(this, 80+2*spacing, posy, 120, STATEBOX_HEIGHT, _("Show messages as overlays:"), Align_CenterLeft);
   UICheckbox* cb = new UICheckbox(this, get_inner_w()-STATEBOX_WIDTH-spacing, posy);
   
   cb->set_state( m_player->show_chat_overlay() );
   cb->changedto.set(this, &GameChatMenu::cb_changed);
   posy += STATEBOX_HEIGHT + spacing;
   
   set_inner_size(get_inner_w(), posy+5);
	if (get_usedefaultpos())
		center_to_parent();

   think();
}

/*
===============
GameChatMenu::~GameChatMenu
===============
*/
GameChatMenu::~GameChatMenu()
{
}

/*
 * think: updates the chat area
 */
void GameChatMenu::think( void ) {

   const std::vector<NetGame::Chat_Message>* msges = m_player->get_chatmsges();
   std::string str;
   
   for( uint i = 0; i < msges->size(); i++) {
      str += m_player->get_game()->get_player((*msges)[i].plrnum)->get_name();
      str += ": ";
      str += (*msges)[i].msg;
      str += "\n";
   }
   
   m_chatbox->set_text( str.c_str() );
}

/*
 * Checkbox has been changed
 */
void GameChatMenu::cb_changed( bool t ) {
   m_player->set_show_chat_overlay(t);
}

void GameChatMenu::clicked(int n) {
   std::string str = m_editbox->get_text();
   
   if( str.size() && m_netgame ) {
      NetGame::Chat_Message t;
      
      t.plrnum = m_player->get_player_number();
      t.msg = str;
      m_netgame->send_chat_message( t );
      m_editbox->set_text("");
   }
}


