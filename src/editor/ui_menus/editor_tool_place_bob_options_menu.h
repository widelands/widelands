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

#ifndef __S__EDITOR_TOOL_PLACE_BOB_OPTIONS_MENU
#define __S__EDITOR_TOOL_PLACE_BOB_OPTIONS_MENU

#include <vector>
#include "editor_tool_options_menu.h"

class Editor_Interactive;
class Editor_Place_Bob_Tool;
class UITextarea;
class UICheckbox;
 
class Editor_Tool_Place_Bob_Options_Menu : public Editor_Tool_Options_Menu {
   public:
      Editor_Tool_Place_Bob_Options_Menu(Editor_Interactive*, Editor_Place_Bob_Tool*,
		                                         UIUniqueWindowRegistry*);
      virtual ~Editor_Tool_Place_Bob_Options_Menu();
      virtual bool handle_key(bool, int, char);

   private:
      std::vector<UICheckbox*> m_checkboxes;
      Editor_Place_Bob_Tool* m_pit;
      UITextarea* m_name;
      void clicked(int, bool);
      void do_nothing(int, bool);
      bool m_multiselect;
};

#endif
