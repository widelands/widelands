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

#ifndef __S__EDITOR_MAIN_MENU_NEW_MAP_H
#define __S__EDITOR_MAIN_MENU_NEW_MAP_H

#include <string>
#include "ui_window.h"

class Editor_Interactive;
class UITextarea;
class UIButton;

/*
=================================================

class Main_Menu_New_Map

This is the new map selection menu. It offers
the user to choose the new world and a few other
things like size, world ....

=================================================
*/
class Main_Menu_New_Map : public UIWindow {
   public:
      Main_Menu_New_Map(Editor_Interactive*);
      virtual ~Main_Menu_New_Map();

   private:
      Editor_Interactive *m_parent;
      UITextarea *m_width, *m_height;
      UIButton* m_world;
      int m_w, m_h;
      std::string m_worldstr;

      void button_clicked(int);
};

#endif
