/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#ifndef __S__EDITORINTERACTIVE_H
#define __S__EDITORINTERACTIVE_H

#include "interactive_base.h"
#include "ui_unique_window.h"

class Editor;
class Editor_Tool;

/*
 * This is the EditorInteractive. It is like the InteractivePlayer class,
 * but for the Editor instead of the game
 */
class Editor_Interactive : public Interactive_Base {
   public:
      Editor_Interactive(Editor*);
      ~Editor_Interactive();

      inline Editor* get_editor(void) { return m_editor; }

      // leaf functions from base class
      void start();

      // gets called when a field is clicked
      void field_clicked();
	virtual void set_sel_pos(Node_and_Triangle);

      // gets called when a keyboard event occurs
      bool handle_key(bool down, int code, char c);

      void select_tool(int, int);
      int get_selected_tool(void) { return tools.current_tool_index; }
      struct Editor_Tools {
         int current_tool_index;
         int use_tool;
         std::vector<Editor_Tool*> tools;
      };

      std::vector<bool>* get_visibility(void) { return 0; }

      // action functions
      void exit_editor();

      // reference functions
	void reference_player_tribe(const int, const void * const);
	void unreference_player_tribe(const int, const void * const);
      bool is_player_tribe_referenced(int);
      void set_need_save(bool t) { m_need_save=t; }

   private:
      void toggle_buildhelp();
      void tool_menu_btn();
      void toolsize_menu_btn();
      void toggle_mainmenu();
      void toggle_eventmenu();
      void toggle_playermenu();
      void toggle_variablesmenu();
      void toggle_objectivesmenu();

      // State variables
      bool m_need_save;
      struct Player_References {
         int player;
         const void *object;
      };
      std::vector<Player_References> m_player_tribe_references;
      bool m_ctrl_down;

      // Tool
      Editor_Tools tools;

      // UI ownings
      Editor* m_editor;
      UI::UniqueWindow::Registry m_toolmenu;
      UI::UniqueWindow::Registry m_toolsizemenu;
      UI::UniqueWindow::Registry m_playermenu;
      UI::UniqueWindow::Registry m_mainmenu;
      UI::UniqueWindow::Registry m_eventmenu;
      UI::UniqueWindow::Registry m_variablesmenu;
      UI::UniqueWindow::Registry m_objectivesmenu;
      std::vector<UI::UniqueWindow::Registry> m_options_menus;
};
#endif // __S__EDITOR_H
