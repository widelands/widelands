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

#ifndef __S__EDITOR_MAIN_MENU_SAVE_MAP_MAKE_DIRECTORY_H
#define __S__EDITOR_MAIN_MENU_SAVE_MAP_MAKE_DIRECTORY_H

#include <string>
#include "ui_window.h"

class UIEdit_Box;
class UIButton;

/*
 * Show a small modal dialog allowing the user to enter
 * a directory name to be created
 */
class Main_Menu_Save_Map_Make_Directory : public UIWindow {
   public:
      Main_Menu_Save_Map_Make_Directory(UIPanel*, const char*);

      const char* get_dirname(void) { return m_dirname.c_str(); }
     
      bool handle_mouseclick(uint btn, bool down, int x, int y);
      
   private:
      std::string m_dirname;
      UIEdit_Box* m_edit;
      UIButton* m_ok_button;
      void edit_changed(void);
      void clicked(int);
};

#endif
