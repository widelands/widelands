/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__EDITOR_PLAYER_MENU_ALLOWED_BUILDINGS_MENU_H
#define __S__EDITOR_PLAYER_MENU_ALLOWED_BUILDINGS_MENU_H

#include "ui_unique_window.h"

class Player;
template <typename T> struct UIListselect;
class UIButton;

/*
 * Let's the user choose which buildings should be available
 * for this player for this scenario. Used to throttle AI and
 * to advance technology slowly through the missions
 */
class Editor_Player_Menu_Allowed_Buildings_Menu : public UIUniqueWindow {
   public:
      Editor_Player_Menu_Allowed_Buildings_Menu(UIPanel* parent, Player* player, UIUniqueWindowRegistry* );
      virtual ~Editor_Player_Menu_Allowed_Buildings_Menu();

   private:
      Player* m_player;
      UIListselect<void *> * m_allowed, *m_forbidden;
      UIButton* m_rtl_button, *m_ltr_button;
      void allowed_selected(int);
      void forbidden_selected(int);
      void allowed_double_clicked(int);
      void forbidden_double_clicked(int);
      void clicked(int);
      void update();
};

#endif
