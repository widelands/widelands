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

#ifndef __S__EDITOR_TOOL_SET_STARTING_POS_OPTIONS_MENU_H
#define __S__EDITOR_TOOL_SET_STARTING_POS_OPTIONS_MENU_H

#include "editor_tool_options_menu.h"
#include "constants.h"

class Editor_Interactive;
class Editor_Set_Starting_Pos_Tool;
class UITextarea;
class UIButton;

class Editor_Tool_Set_Starting_Pos_Options_Menu : public Editor_Tool_Options_Menu {
   public:
      Editor_Tool_Set_Starting_Pos_Options_Menu(Editor_Interactive*, Editor_Set_Starting_Pos_Tool*, UIUniqueWindowRegistry*);
      virtual ~Editor_Tool_Set_Starting_Pos_Options_Menu() { }

   private:
      Editor_Set_Starting_Pos_Tool* m_spt;
      Editor_Interactive* m_parent;
      UITextarea *m_nr_of_players_ta;
      UITextarea* m_plr_textareas[MAX_PLAYERS];
      UIButton* m_plr_buttons[MAX_PLAYERS];
      int m_posy;

      void button_clicked(int);
      void update();
};

#endif
