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

#ifndef __S__GAME_MAIN_MENU_LOAD_GAME_H
#define __S__GAME_MAIN_MENU_LOAD_GAME_H

#include "filesystem.h"
#include "ui_unique_window.h"

class Interactive_Player;
class UIEdit_Box;
class UITextarea;
class UIListselect;
class UIButton;


class Game_Main_Menu_Load_Game : public UIUniqueWindow {
   public:
      Game_Main_Menu_Load_Game(Interactive_Player* plr, UIUniqueWindowRegistry* registry) ;
      virtual ~Game_Main_Menu_Load_Game(void);

   private:
      void clicked(int);
      void selected(int);
      void edit_box_changed();
      void double_clicked(int);

      void fill_list(void);
      bool load_game(std::string);
      
      Interactive_Player* m_parent;
      UITextarea* m_name, *m_gametime;   
      UIListselect* m_ls;
      filenameset_t m_gamefiles;

      UIButton* m_ok_btn;
      std::string m_basedir;
      std::string m_curdir;
      std::string m_parentdir;
};

#endif
