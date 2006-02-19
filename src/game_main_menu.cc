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
#include "game_main_menu.h"
#include "game_main_menu_save_game.h"
#include "game_main_menu_load_game.h"
#include "game_objectives_menu.h"
#include "game_options_menu.h"
#include "general_statistics_menu.h"
#include "interactive_player.h"
#include "stock_menu.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "ware_statistics_menu.h"

/*
==============================================================================

GameMainMenu IMPLEMENTATION

==============================================================================
*/

/*
===============
GameMainMenu::GameMainMenu

Create all the buttons etc...
===============
*/
GameMainMenu::GameMainMenu(Interactive_Player *plr, UIUniqueWindowRegistry *registry, Interactive_Player::Game_Main_Menu_Windows* windows)
	: UIUniqueWindow(plr, registry, 180, 160, _("Main Menu"))
{
   m_player=plr;
   m_windows = windows;
  

   int spacing = 5;
   int posy = 2*spacing;
   int posx = 2*spacing;
   
   // UIButtons
   int buttonw = (get_inner_w()-7*spacing) / 4;
   UIButton* b=new UIButton(this, posx, posy, buttonw, 34, 4, 1);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_general_stats.png" ));
   b->clickedid.set(this, &GameMainMenu::clicked);
   posx += buttonw + spacing;
  
   b=new UIButton(this, posx, posy, buttonw, 34, 4, 2);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_ware_stats.png" ));
   b->clickedid.set(this, &GameMainMenu::clicked);
   posx += buttonw + spacing;

   b=new UIButton(this, posx, posy, buttonw, 34, 4, 3);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_building_stats.png" ));
   b->clickedid.set(this, &GameMainMenu::clicked);
   posx += buttonw + spacing;
  
   b=new UIButton(this, posx, posy, buttonw, 34, 4, 4);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_stock.png" ));
   b->clickedid.set(this, &GameMainMenu::clicked);
   posx = 2*spacing;
   posy += 45;
       
   buttonw = (get_inner_w()-5*spacing) / 2;
   b=new UIButton(this, posx, posy, buttonw, 34, 4, 25);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_objectives.png" ));
   b->clickedid.set(this, &GameMainMenu::clicked);
   posx += buttonw + spacing;
   
   b=new UIButton(this, posx, posy, buttonw, 34, 4, 30);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_chat.png" ));
   b->clickedid.set(this, &GameMainMenu::clicked);
   posx = 2*spacing;
   posy += 45;
   
   buttonw = (get_inner_w()-4*spacing);
   b=new UIButton(this, posx, posy, buttonw, 34, 4, 50);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_options_menu.png" ));
   b->clickedid.set(this, &GameMainMenu::clicked);
   posx = 2*spacing;
   posy += 40;
   
   set_inner_size(get_inner_w(), posy+5);
	if (get_usedefaultpos())
		center_to_parent();
}

void GameMainMenu::clicked(int n) {
   switch(n) {
      case 1:
         // General Statistics
         if (m_windows->general_stats.window)
            delete m_windows->general_stats.window; 
         else
            new General_Statistics_Menu(m_player, &m_windows->general_stats);
         break;

      case 2:
         // Wares statistics
         if (m_windows->ware_stats.window)
            delete m_windows->ware_stats.window; 
         else
            new Ware_Statistics_Menu(m_player, &m_windows->ware_stats);
         break;

      case 3:
         // Buildings statistics
         if (m_windows->building_stats.window)
            delete m_windows->building_stats.window; 
         else
            new Building_Statistics_Menu(m_player, &m_windows->building_stats);
         break;

      case 4:
         // Global Stock
         if (m_windows->stock.window)
            delete m_windows->stock.window; 
         else
            new Stock_Menu(m_player, &m_windows->stock);
         break;

      case 25:
         // Mission Objectives 
         if (m_windows->objectives.window)
            delete m_windows->objectives.window; 
         else
            new GameObjectivesMenu(m_player, &m_windows->objectives, m_player->get_game());
         break;
  
      case 30:
         // Chat Window
         if (m_windows->chat.window)
            delete m_windows->chat.window; 
         else
            new GameChatMenu(m_player, &m_windows->chat, m_player->get_game()->get_netgame());
         break;

      case 50:
         // Options Menu
         if (m_windows->options.window)
            delete m_windows->options.window; 
         else
            new GameOptionsMenu(m_player, &m_windows->options, m_windows);
         break;

      default:
         break;
   }
}

/*
===============
GameMainMenu::~GameMainMenu
===============
*/
GameMainMenu::~GameMainMenu()
{
}


