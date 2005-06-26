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

#ifndef __S__EDITOR_MAIN_MENU_SAVE_MAP_H
#define __S__EDITOR_MAIN_MENU_SAVE_MAP_H

#include "filesystem.h"
#include "ui_window.h"

class Editor_Interactive;
class UIButton;
class UIEdit_Box;
class UIListselect;
class UIMultiline_Textarea;
class UITextarea;

/*
=================================================

class Main_Menu_Save_Map

Choose a filename and save your brand new created map

=================================================
*/
class Main_Menu_Save_Map : public UIWindow {
   public:
      Main_Menu_Save_Map(Editor_Interactive*);
      virtual ~Main_Menu_Save_Map();

   private:
      void clicked(int);
      void selected(int);
      void edit_box_changed();
      void double_clicked(int);

      void fill_list(void);
      bool save_map(std::string, bool);
      
      UIEdit_Box* m_editbox;
      UITextarea *m_name, *m_author, *m_size, *m_world, *m_nrplayers;
      UIMultiline_Textarea* m_descr;
      Editor_Interactive *m_parent;
      UIListselect* m_ls;
      UIButton* m_ok_btn;
      
      std::string m_basedir;
      std::string m_curdir;
      std::string m_parentdir;
      filenameset_t m_mapfiles;
};

#endif

