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

#ifndef __S__EDITOR_MAIN_MENU_LOAD_MAP_H
#define __S__EDITOR_MAIN_MENU_LOAD_MAP_H

#include "ui_window.h"
#include "filesystem.h"

class Editor_Interactive;
class UIEdit_Box;
class UIListselect;
class UITextarea;
class UIMultiline_Textarea;

/*
=================================================

class Main_Menu_Load_Map

Choose a filename and save your brand new created map

=================================================
*/
class Main_Menu_Load_Map : public UIWindow {
   public:
      Main_Menu_Load_Map(Editor_Interactive*);
      virtual ~Main_Menu_Load_Map();

   private:
      void clicked(int);
      void selected(int);

      UITextarea *m_name, *m_author, *m_size, *m_world, *m_nrplayers;
      UIMultiline_Textarea* m_descr;
      Editor_Interactive *m_parent;
      UIListselect* m_ls;
      UIEdit_Box* m_editbox;
      filenameset_t m_mapfiles;
};

#endif

