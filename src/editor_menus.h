/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#ifndef __S__EDITOR_MENUS_H
#define __S__EDITOR_MENUS_H

#include "ui.h"
#include "editorinteractive.h"

/*
=============================

class Editor_Main_Menu

this represents the main menu

=============================
*/
class Editor_Main_Menu : public Window {
   public:
      Editor_Main_Menu(Editor_Interactive*, UniqueWindow*);
      virtual ~Editor_Main_Menu();

   private:
      UniqueWindow* m_registry;
      Editor_Interactive *m_parent;

      void exit_btn();
      void load_btn();
      void save_btn();
      void new_map_btn();
      void map_options_btn();     
      void readme_btn();
};

/*
=============================

class Editor_Preliminary_Tool_Menu

This class is the tool selection window/menu. 
Here, you can select the tool you wish to use the next time

It's preliminary, no graphics, no stuff. only for debugging

=============================
*/

class Editor_Preliminary_Tool_Menu : public Window {
   public:
      Editor_Preliminary_Tool_Menu(Editor_Interactive*, UniqueWindow*, Editor_Interactive::Editor_Tools*);
      virtual ~Editor_Preliminary_Tool_Menu();

   private:
      Editor_Interactive::Editor_Tools* m_tools;
      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;
      Radiogroup* m_radioselect;

      void changed_to_function(int);
      void options_button_clicked(int);
};


/*
=============================

class Editor_Toolsize_Menu

This class is the tool selection window/menu. 
Here, you can select the tool you wish to use the next time

=============================
*/

class Editor_Toolsize_Menu : public Window {
   public:
      Editor_Toolsize_Menu(Editor_Interactive*, UniqueWindow*);
      virtual ~Editor_Toolsize_Menu();

   private:
      void button_clicked(int);

      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;
      Textarea* m_textarea;
};


#endif // __S__EDITOR_MENUS_H
