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

#ifndef __S__EDITOR_TOOL_CHANGE_RESOURCES_OPTIONS_MENU
#define __S__EDITOR_TOOL_CHANGE_RESOURCES_OPTIONS_MENU

#include "editor_tool_options_menu.h"
#include "ui_radiobutton.h"

class Editor_Interactive;
class Editor_Increase_Resources_Tool;
class Editor_Decrease_Resources_Tool;
class Editor_Set_Resources_Tool;
class UITextarea;

class Editor_Tool_Change_Resources_Options_Menu : public Editor_Tool_Options_Menu {
   public:
      Editor_Tool_Change_Resources_Options_Menu(Editor_Interactive*, int, Editor_Increase_Resources_Tool*,
            UIUniqueWindowRegistry*);
      ~Editor_Tool_Change_Resources_Options_Menu() { delete m_radiogroup; }

   private:
      void selected(void);
      void clicked(int);
      void update(void);
      UITextarea* m_increase, *m_set, *m_cur_selection;
      UIRadiogroup* m_radiogroup;
      Editor_Increase_Resources_Tool* m_irt;
      Editor_Decrease_Resources_Tool* m_drt;
      Editor_Set_Resources_Tool* m_srt;
      Editor_Interactive* m_parent;
};

#endif
