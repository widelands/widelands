/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#include "editor.h"
#include "interactive_base.h"
#include "editorinteractive.h"
#include "keycodes.h"
#include "overlay_manager.h"
#include "map.h"
#include "mapview.h"
#include "player.h"
#include "ui_button.h"
#include "editor_main_menu.h"
#include "editor_event_menu.h"
#include "editor_tool_menu.h"
#include "editor_toolsize_menu.h"
#include "editor_delete_immovable_tool.h"
#include "editor_info_tool.h"
#include "editor_place_immovable_tool.h"
#include "editor_set_both_terrain_tool.h"
#include "editor_set_down_terrain_tool.h"
#include "editor_set_right_terrain_tool.h"
#include "editor_increase_height_tool.h"
#include "editor_noise_height_tool.h"
#include "editor_set_starting_pos_tool.h"
#include "editor_place_bob_tool.h"
#include "editor_increase_resources_tool.h"


/**********************************************
 *
 * class EditorInteractive
 *
 **********************************************/

/*
==========
Editor_Interactive::Editor_Interactive()

construct editor sourroundings
==========
*/
Editor_Interactive::Editor_Interactive(Editor *e) : Interactive_Base(e) {
   m_editor = e;

   // The mapview. watch the map!!!
   Map_View* mm;
   mm = new Map_View(this, 0, 0, get_w(), get_h(), this);
   mm->warpview.set(this, &Editor_Interactive::mainview_move);
   mm->fieldclicked.set(this, &Editor_Interactive::field_clicked);
   set_mapview(mm);

   // user interface buttons
   int x = (get_w() - (5*34)) >> 1;
   int y = get_h() - 34;
   UIButton *b;

   b = new UIButton(this, x, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_mainmenu);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_menu.png", true)); 

   b = new UIButton(this, x+34, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::tool_menu_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/editor_menu_toggle_tool_menu.png", true)); 

   b = new UIButton(this, x+68, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toolsize_menu_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/editor_menu_set_toolsize_menu.png", true));

   b = new UIButton(this, x+102, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_minimap);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.png", true));

   b = new UIButton(this, x+136, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_buildhelp);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_buildhelp.png", true));

   b = new UIButton(this, x+170, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_eventmenu);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_event_menu.png", true));

   // Init Tools
   tools.current_tool_index=1;
   tools.use_tool=0;
   tools.tools.push_back(new Editor_Info_Tool());
   Editor_Set_Height_Tool* sht=new Editor_Set_Height_Tool();
   tools.tools.push_back(new Editor_Increase_Height_Tool(new Editor_Decrease_Height_Tool(), sht));
   tools.tools.push_back(new Editor_Noise_Height_Tool(sht));
   tools.tools.push_back(new Editor_Set_Both_Terrain_Tool(new Editor_Set_Down_Terrain_Tool(), new Editor_Set_Right_Terrain_Tool()));
   tools.tools.push_back(new Editor_Place_Immovable_Tool(new Editor_Delete_Immovable_Tool()));
   tools.tools.push_back(new Editor_Set_Starting_Pos_Tool());
   tools.tools.push_back(new Editor_Place_Bob_Tool(new Editor_Delete_Bob_Tool()));
   tools.tools.push_back(new Editor_Increase_Resources_Tool(new Editor_Decrease_Resources_Tool(), new Editor_Set_Resources_Tool()));

/*   tools.tools.push_back(new Tool_Info(1, 3, new Editor_Decrease_Height_Tool()));
   tools.tools.push_back(new Tool_Info(1, 2, new Editor_Set_Height_Tool()));
   tools.tools.push_back(new Tool_Info(4, 4, new Editor_Noise_Height_Tool()));
   tools.tools.push_back(new Tool_Info(6, 7, new Editor_Set_Right_Terrain_Tool()));
   tools.tools.push_back(new Tool_Info(5, 7, new Editor_Set_Down_Terrain_Tool()));
   tools.tools.push_back(new Tool_Info(5, 6, new Editor_Set_Both_Terrain_Tool()));
  */
   select_tool(1, 0);
}

/****************************************
 * Editor_Interactive::~EditorInteractive()
 *
 * cleanup
 */
Editor_Interactive::~Editor_Interactive() {
   while(tools.tools.size()) {
      delete tools.tools.back();
      tools.tools.pop_back();
   }
   unset_fieldsel_picture(); // reset default fsel
}

/*
===============
Editor_Interactive::start

Called just before the game starts, after postload, init and gfxload
===============
*/
void Editor_Interactive::start()
{
   m_maprenderinfo.egbase = m_editor;
	m_maprenderinfo.visibility = 0;

   get_map()->get_overlay_manager()->show_buildhelp(true);
}

/*
===========
Editor_Interactive::exit_editor()

exit the editor
===========
*/
void Editor_Interactive::exit_editor()
{
	end_modal(0);
}

/*
===========
Editor_Interactive::toggle_mainmenu()

toggles the mainmenu
===========
*/
void Editor_Interactive::toggle_mainmenu(void) {
   if (m_mainmenu.window) {
      delete m_mainmenu.window;
   }
   else {
      new Editor_Main_Menu(this, &m_mainmenu);
   }
}

/*
 * Create the event menu
 */
void Editor_Interactive::toggle_eventmenu(void) {
   if(m_eventmenu.window) {
      delete m_eventmenu.window;
   } else {
      new Editor_Event_Menu(this, &m_eventmenu);
   }
}

/*
===========
Editor_Interactive::field_clicked()

This functions is called, when a field is clicked. it mainly calls
the function of the currently selected tool
===========
*/
void Editor_Interactive::field_clicked() {
   Map* m=get_map();
   FCoords cords(get_fieldsel_pos(), m->get_field(get_fieldsel_pos()));
   tools.tools[tools.current_tool_index]->handle_click(tools.use_tool, cords, m, this);
}

/*
===========
Editor_Interactive::toggle_buildhelp()

toggles the buildhelp on the map
===========
*/
void Editor_Interactive::toggle_buildhelp(void)
{
   get_map()->get_overlay_manager()->toggle_buildhelp();
}

/*
===============
Editor_Interactive::tool_menu_btn

Bring up or close the main menu
===============
*/
void Editor_Interactive::tool_menu_btn()
{
	if (m_toolmenu.window)
		delete m_toolmenu.window;
	else
		new Editor_Tool_Menu(this, &m_toolmenu, &tools);
}

/*
===============
Editor_Interactive::toolsize_menu_btn

Bring up or close the main menu
===============
*/
void Editor_Interactive::toolsize_menu_btn()
{
	if (m_toolsizemenu.window)
		delete m_toolsizemenu.window;
	else
		new Editor_Toolsize_Menu(this, &m_toolsizemenu);
}




/*
===========
Editor_Interactive::handle_key()

Handles a keyboard event
===========
*/
bool Editor_Interactive::handle_key(bool down, int code, char c) {
   if(down) {
      // only on down events
      switch(code) {
         case KEY_SPACE:
            toggle_buildhelp();
            return true;

         case KEY_m:
            toggle_minimap();
            return true;

         case KEY_t:
            tool_menu_btn();
            return true;

            // Fieldsel radius
         case KEY_1:
            set_fieldsel_radius(0);
            return true;
         case KEY_2:
            set_fieldsel_radius(1);
            return true;
         case KEY_3:
            set_fieldsel_radius(2);
            return true;
         case KEY_4:
            set_fieldsel_radius(3);
            return true;
         case KEY_5:
            set_fieldsel_radius(4);
            return true;
         case KEY_6:
            set_fieldsel_radius(5);
            return true;
         case KEY_7:
            set_fieldsel_radius(6);
            return true;

         case KEY_LSHIFT:
         case KEY_RSHIFT:
            if(!tools.use_tool) {
               select_tool(tools.current_tool_index, 1);
            }
            return true;

         case KEY_LALT:
         case KEY_RALT:
         case KEY_MODE:
            if(!tools.use_tool) {
               select_tool(tools.current_tool_index, 2);
            }
            return true;

         case KEY_i:
            select_tool(0, 0);
            return true;
      }
   } else {
      // key up events
      switch(code) {
         case KEY_LSHIFT:
         case KEY_RSHIFT:
         case KEY_LALT:
         case KEY_RALT:
         case KEY_MODE:
            if(tools.use_tool) {
               select_tool(tools.current_tool_index, 0);
            }
            return true;
      }
   }
   return false;
}

/*
===========
Editor_Interactive::select_tool()

select a new tool
===========
*/
void Editor_Interactive::select_tool(int n, int which) {
   if(which==0 && n!=tools.current_tool_index) {
      // A new tool has been selected. Remove all 
      // registered overlay callback functions
      get_map()->get_overlay_manager()->register_overlay_callback_function(0,0);
      get_map()->recalc_whole_map();

   }
   tools.current_tool_index=n;
   tools.use_tool=which;

   const char* fselpic= tools.tools[n]->get_fsel(which);
   if(!fselpic) unset_fieldsel_picture();
   else set_fieldsel_picture(fselpic);
}


