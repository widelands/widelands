/*
 * Copyright (C) 2003 by The Widelands Development Team
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
// include ui.h before this

#ifndef __S__EDITOR_TOOLS_OPTION_MENUS_H
#define __S__EDITOR_TOOLS_OPTION_MENUS_H

#include "editor_tools.h"

class Editor_Interactive;

class Editor_Tool_Options_Menu : public UniqueWindow {
      public:
         Editor_Tool_Options_Menu(Editor_Interactive*, UniqueWindowRegistry*, char* title);
         ~Editor_Tool_Options_Menu();

         inline Editor_Interactive* get_parent() { return m_parent; }

      private:
         Editor_Interactive* m_parent;
};

class Editor_Tool_Change_Height_Options_Menu : public Editor_Tool_Options_Menu {
   public:
      Editor_Tool_Change_Height_Options_Menu(Editor_Interactive*, Editor_Increase_Height_Tool*,
            UniqueWindowRegistry*);
      ~Editor_Tool_Change_Height_Options_Menu() { }

   private:
      void clicked(int);
      void update(void);
      Textarea* m_increase, *m_set;
      Editor_Increase_Height_Tool* m_iht;
      Editor_Decrease_Height_Tool* m_dht;
      Editor_Set_Height_Tool* m_sht;
};

class Editor_Tool_Noise_Height_Options_Menu : public Editor_Tool_Options_Menu {
   public:
      Editor_Tool_Noise_Height_Options_Menu(Editor_Interactive*, Editor_Noise_Height_Tool*, UniqueWindowRegistry*);
      virtual ~Editor_Tool_Noise_Height_Options_Menu() { }

   private:
      Textarea* m_textarea_lower;
      Textarea* m_textarea_upper;
      Textarea* m_set;
      Editor_Noise_Height_Tool* m_nht;

      void button_clicked(int);
      void update();
};

class Editor_Tool_Set_Terrain_Tool_Options_Menu : public Editor_Tool_Options_Menu {
   public:
      Editor_Tool_Set_Terrain_Tool_Options_Menu(Editor_Interactive*, Editor_Set_Both_Terrain_Tool*,
		                                          UniqueWindowRegistry* );
      virtual ~Editor_Tool_Set_Terrain_Tool_Options_Menu() { };

   private:
      Textarea* m_textarea;
      Radiogroup* m_radiogroup;
      Editor_Set_Both_Terrain_Tool* m_sbt;
      void selected(int);
};

class Editor_Tool_Place_Immovable_Options_Menu : public Editor_Tool_Options_Menu {
   public:
      Editor_Tool_Place_Immovable_Options_Menu(Editor_Interactive*, Editor_Place_Immovable_Tool*,
		                                         UniqueWindowRegistry*);
      virtual ~Editor_Tool_Place_Immovable_Options_Menu() { };

   private:
      Editor_Place_Immovable_Tool* m_pit;
      Textarea* m_name;
      void clicked(int, bool);
};
   
#endif // __S__EDITOR_TOOLS_OPTION_MENUS_H
