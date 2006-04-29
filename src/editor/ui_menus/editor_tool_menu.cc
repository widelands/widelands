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

#include "editor_tool_menu.h"
#include "editorinteractive.h"
#include "i18n.h"
#include "ui_radiobutton.h"
#include "ui_textarea.h"
#include "editor_tool_change_height_options_menu.h"
#include "editor_tool_set_terrain_options_menu.h"
#include "editor_tool_noise_height_options_menu.h"
#include "editor_tool_place_immovable_options_menu.h"
#include "editor_tool_place_bob_options_menu.h"
#include "editor_increase_height_tool.h"
#include "editor_decrease_height_tool.h"
#include "editor_noise_height_tool.h"
#include "editor_place_immovable_tool.h"
#include "editor_set_both_terrain_tool.h"
#include "editor_place_bob_tool.h"
#include "editor_tool_change_resources_options_menu.h"
#include "editor_increase_resources_tool.h"
#include "editor_decrease_resources_tool.h"

/*
===============
Editor_Tool_Menu::Editor_Tool_Menu

Create all the buttons etc...
===============
*/
Editor_Tool_Menu::Editor_Tool_Menu(Editor_Interactive *parent, UIUniqueWindowRegistry *registry,
                                   Editor_Interactive::Editor_Tools* tools, std::vector<UIUniqueWindowRegistry>* options)
	: UIUniqueWindow(parent, registry, 350, 400, _("Tool Menu"))
{
   m_tools=tools;
   m_parent=parent;
   m_options_menus=options;

   // UIButtons
   const int offsx=5;
   const int offsy=30;
   const int spacing=5;
   const int width=34;
   const int height=34;
   int posx=offsx;
   int posy=offsy;


   int num_tools=6;
   m_radioselect=new UIRadiogroup();
   m_radioselect->add_button(this, posx, posy, g_gr->get_picture( PicMod_Game,  "pics/editor_menu_tool_change_height.png" ));
   posx+=width+spacing;
   m_radioselect->add_button(this, posx, posy, g_gr->get_picture( PicMod_Game,  "pics/editor_menu_tool_noise_height.png" ));
   posx+=width+spacing;
   m_radioselect->add_button(this, posx, posy, g_gr->get_picture( PicMod_Game,  "pics/editor_menu_tool_set_terrain.png" ));
   posx+=width+spacing;
   m_radioselect->add_button(this, posx, posy, g_gr->get_picture( PicMod_Game,  "pics/editor_menu_tool_place_immovable.png" ));
   posx+=width+spacing;
   m_radioselect->add_button(this, posx, posy, g_gr->get_picture( PicMod_Game,  "pics/editor_menu_tool_place_bob.png" ));
   posx+=width+spacing;
   m_radioselect->add_button(this, posx, posy, g_gr->get_picture( PicMod_Game,  "pics/editor_menu_tool_change_resources.png" ));

   set_inner_size(offsx+(width+spacing)*num_tools, offsy+(height+spacing));

   UITextarea* ta=new UITextarea(this, 0, 0, _("Tool Menu"));
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);

   m_radioselect->set_state(parent->get_selected_tool()-1);

   m_radioselect->changed.set(this, &Editor_Tool_Menu::changed_to);
   m_radioselect->clicked.set(this, &Editor_Tool_Menu::changed_to);

	if (get_usedefaultpos())
		center_to_parent();
}

/*
===============
Editor_Tool_Menu::~Editor_Tool_Menu

Unregister from the registry pointer
===============
*/
Editor_Tool_Menu::~Editor_Tool_Menu()
{
   delete m_radioselect;
}

/*
===========
Editor_Tool_Menu::changed_to()

called when the radiogroup changes or is reclicked
===========
*/
void Editor_Tool_Menu::changed_to(void) {
   int n=m_radioselect->get_state();

   int index=-1;
   switch(n) {
      case 0: index=1; break;
      case 1: index=2; break;
      case 2: index=3; break;
      case 3: index=4; break;
      case 4: index=6; break;
      case 5: index=7; break;
      default: break;
   }
   assert(index!=-1);

   // Select tool
   m_parent->select_tool(index, 0);

   UIWindow* w=(*m_options_menus)[index].window;
   bool create_window=false;
   if(w) {
      // There is already a window. If it is small,
      // make it big.
      if(w->is_minimized()) {
         w->minimize(false);
      } else {
         delete w;
      }
   } else {
      create_window=true;
   }

   if(create_window) {
      // Create window
      switch(n) {
         case 0:
            new Editor_Tool_Change_Height_Options_Menu(m_parent, index,
                  static_cast<Editor_Increase_Height_Tool*>(m_tools->tools[index]),
                  &((*m_options_menus)[index]));
            break;

         case 1:
            new Editor_Tool_Noise_Height_Options_Menu(m_parent, index,
                  static_cast<Editor_Noise_Height_Tool*>(m_tools->tools[index]),
                  &((*m_options_menus)[index]));
            break;

         case 2:
            new Editor_Tool_Set_Terrain_Tool_Options_Menu(m_parent, index,
                  static_cast<Editor_Set_Both_Terrain_Tool*>(m_tools->tools[index]),
                  &((*m_options_menus)[index]));
            break;

         case 3:
            new Editor_Tool_Place_Immovable_Options_Menu(m_parent, index,
                  static_cast<Editor_Place_Immovable_Tool*>(m_tools->tools[index]),
                  &((*m_options_menus)[index]));
            break;

         case 4:
            new Editor_Tool_Place_Bob_Options_Menu(m_parent, index,
                  static_cast<Editor_Place_Bob_Tool*>(m_tools->tools[index]),
                  &((*m_options_menus)[index]));
            break;

         case 5:
            new Editor_Tool_Change_Resources_Options_Menu(m_parent, index,
                  static_cast<Editor_Increase_Resources_Tool*>(m_tools->tools[index]),
                  &((*m_options_menus)[index]));
            break;


         default: break;
      }
   }
}
