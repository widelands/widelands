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
#include "fullscreen_menu_fileview.h"
#include "game_options_menu.h"
#include "game_main_menu_save_game.h"
#include "game_main_menu_load_game.h"
#include "game_options_menu.h"
#include "general_statistics_menu.h"
#include "i18n.h"
#include "interactive_player.h"
#include "sound_handler.h"
#include "stock_menu.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "ware_statistics_menu.h"

/*
==============================================================================

GameOptionsMenu IMPLEMENTATION

==============================================================================
*/

/*
===============
GameOptionsMenu::GameOptionsMenu

Create all the buttons etc...
===============
*/
GameOptionsMenu::GameOptionsMenu(Interactive_Player *plr, UIUniqueWindowRegistry *registry, Interactive_Player::Game_Main_Menu_Windows* windows)
	: UIUniqueWindow(plr, registry, 102, 160, _("Options"))
{
   m_player=plr;
   m_windows = windows;

   // Caption
   new UITextarea(this, 0, 5, get_inner_w(), 25, _("Options Menu"), Align_Center);
   int posy = 35;


   UIButton* b=new UIButton(this, 5, posy, get_inner_w()-10, 20, 4, 1);
   b->set_title(_("README"));
   b->clickedid.set(this, &GameOptionsMenu::clicked);
   posy += 25;

   b=new UIButton(this, 5, posy, get_inner_w()-10, 20, 4, 2);
   b->set_title(_("License"));
   b->clickedid.set(this, &GameOptionsMenu::clicked);
   posy += 25;

   b=new UIButton(this, 5, posy, get_inner_w()-10, 20, 4, 3);
   b->set_title(_("Authors"));
   b->clickedid.set(this, &GameOptionsMenu::clicked);
   posy += 25;

   UICheckbox* cb = new UICheckbox(this, 5, posy);
   cb->set_state(!g_sound_handler.get_disable_music());
   cb->changedto.set(this, &GameOptionsMenu::disable_music_clicked);
   UITextarea *t=new UITextarea(this, 30, posy+3, _("Ingame Music"));
   posy += 25;
   if (g_sound_handler.m_lock_audio_disabling) {
   	cb->set_enabled(false);
   }

   cb=new UICheckbox(this, 5, posy);
   cb->set_state(!g_sound_handler.get_disable_fx());
   cb->changedto.set(this, &GameOptionsMenu::disable_fx_clicked);
   t=new UITextarea(this, 30, posy+3, _("Sound FX"));
   posy += 25;
   if (g_sound_handler.m_lock_audio_disabling) {
		cb->set_enabled(false);
   }

   int buttonw = (get_inner_w()-3*5) / 2;
   b=new UIButton(this, 5, posy, buttonw, 35, 4, 4);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_save_game.png" ));
   b->clickedid.set(this, &GameOptionsMenu::clicked);

   b=new UIButton(this, 10+buttonw, posy, buttonw, 35, 4, 5);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_load_game.png" ));
   b->clickedid.set(this, &GameOptionsMenu::clicked);
   posy += 45;

   b=new UIButton(this, 5, posy, get_inner_w()-10, 35, 4, 6);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_exit_game.png" ));
   b->clickedid.set(this, &GameOptionsMenu::clicked);
   posy += 40;

   set_inner_size(get_inner_w(), posy+5);

	if (get_usedefaultpos())
		center_to_parent();
}

void GameOptionsMenu::clicked(int n) {
   switch(n) {
      case 1:
         // Readme
         fileview_window(m_player, &m_windows->readme, "txts/README");
         break;

      case 2:
         // License
         fileview_window(m_player, &m_windows->licence, "txts/COPYING");
         break;

      case 3:
         // Authors
         fileview_window(m_player, &m_windows->authors, "txts/developers");
         break;

      case 4:
         // Save
         new Game_Main_Menu_Save_Game(m_player, &m_windows->savegame);
         die();
         break;

      case 5:
         // Load
         new Game_Main_Menu_Load_Game(m_player, &m_windows->loadgame);
         die();
         break;

      case 6:
         // Exit Game
         m_player->end_modal(0);
         return;
         break;
   }
}

/*
 * One of the checkboxes have been toggled
 */
void GameOptionsMenu::disable_music_clicked( bool t ) {
   g_sound_handler.set_disable_music( !t );
}

void GameOptionsMenu::disable_fx_clicked( bool t ) {
   g_sound_handler.set_disable_fx( !t );
}

/*
===============
GameOptionsMenu::~GameOptionsMenu
===============
*/
GameOptionsMenu::~GameOptionsMenu()
{
}


