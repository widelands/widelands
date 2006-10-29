/*
 * Copyright (C) 2002-2003, 2006 by the Widelands Development Team
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
#include "editorinteractive.h"
#include "editor_delete_immovable_tool.h"
#include "editor_event_menu.h"
#include "editor_objectives_menu.h"
#include "editor_variables_menu.h"
#include "editor_increase_height_tool.h"
#include "editor_increase_resources_tool.h"
#include "editor_info_tool.h"
#include "editor_main_menu.h"
#include "editor_main_menu_load_map.h"
#include "editor_main_menu_save_map.h"
#include "editor_make_infrastructure_tool.h"
#include "editor_noise_height_tool.h"
#include "editor_place_immovable_tool.h"
#include "editor_place_bob_tool.h"
#include "editor_player_menu.h"
#include "editor_set_both_terrain_tool.h"
#include "editor_set_down_terrain_tool.h"
#include "editor_set_right_terrain_tool.h"
#include "editor_set_starting_pos_tool.h"
#include "editor_tool_menu.h"
#include "editor_toolsize_menu.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_base.h"
#include "keycodes.h"
#include "map.h"
#include "mapview.h"
#include "overlay_manager.h"
#include "player.h"
#include "tribe.h"
#include "ui_button.h"
#include "ui_modal_messagebox.h"
#include "wlapplication.h"


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

   // Disable debug. it is no use for editor
#ifndef DEBUG
   set_display_flag(Interactive_Base::dfDebug, false);
#else
   set_display_flag(Interactive_Base::dfDebug, true);
#endif

   // The mapview. watch the map!!!
   Map_View* mm;
   mm = new Map_View(this, 0, 0, get_w(), get_h(), this);
   mm->warpview.set(this, &Editor_Interactive::mainview_move);
   mm->fieldclicked.set(this, &Editor_Interactive::field_clicked);
   set_mapview(mm);

   // user interface buttons
   int x = (get_w() - (7*34)) >> 1;
   int y = get_h() - 34;
   UIButton *b;

   b = new UIButton(this, x, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_mainmenu);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_toggle_menu.png" ));
   b->set_tooltip(_("Menu").c_str());

   b = new UIButton(this, x+34, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::tool_menu_btn);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/editor_menu_toggle_tool_menu.png" ));
   b->set_tooltip(_("Tool").c_str());

   b = new UIButton(this, x+68, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toolsize_menu_btn);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/editor_menu_set_toolsize_menu.png" ));
   b->set_tooltip(_("Toolsize").c_str());

   b = new UIButton(this, x+102, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_minimap);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_toggle_minimap.png" ));
   b->set_tooltip(_("Minimap").c_str());

   b = new UIButton(this, x+136, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_buildhelp);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_toggle_buildhelp.png" ));
   b->set_tooltip(_("Buildhelp").c_str());

   b = new UIButton(this, x+170, y, 34, 43, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_playermenu);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/editor_menu_player_menu.png" ));
   b->set_tooltip(_("Players").c_str());

   b = new UIButton(this, x+204, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_eventmenu);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_toggle_event_menu.png" ));
   b->set_tooltip(_("Events").c_str());

   b = new UIButton(this, x+238, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_variablesmenu);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_toggle_variables_menu.png" ));
   b->set_tooltip(_("Variables").c_str());

   b = new UIButton(this, x+272, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_objectivesmenu);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_toggle_objectives_menu.png" ));
   b->set_tooltip(_("Objectives").c_str());


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
   tools.tools.push_back(new Editor_Make_Infrastructure_Tool());

   // Option menus
   m_options_menus.resize(tools.tools.size());

   // Load all tribes into memory
   std::vector<std::string> tribes;
	Tribe_Descr::get_all_tribenames(tribes);
   uint i=0;
   for(i=0; i<tribes.size(); i++)
      e->manually_load_tribe(tribes[i].c_str());

   m_need_save=false;
   m_ctrl_down=false;

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
	if(m_need_save) {
      UIModal_Message_Box* mmb=new UIModal_Message_Box(this, _("Map unsaved"), _("The Map is unsaved, do you really want to quit?"), UIModal_Message_Box::YESNO);
      int code=mmb->run();
      delete mmb;
      if(code==0) return;
   }
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
===========
Editor_Interactive::toggle_objectivesmenu()

toggles the objectivesmenu
===========
*/
void Editor_Interactive::toggle_objectivesmenu(void) {
   if (m_objectivesmenu.window) {
      delete m_objectivesmenu.window;
   }
   else {
      new Editor_Objectives_Menu(this, &m_objectivesmenu);
   }
}

/*
===========
Editor_Interactive::toggle_variablesmenu()

toggles the variablesmenu
===========
*/
void Editor_Interactive::toggle_variablesmenu(void) {
   if (m_variablesmenu.window) {
      delete m_variablesmenu.window;
   }
   else {
      new Editor_Variables_Menu(this, &m_variablesmenu);
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
   get_mapview()->need_complete_redraw();
   set_need_save(true);
}

/*
 * Set the current fieldsel position and, if
 * a tool is selected and the first mouse button is pressed
 * click this field
 */
void Editor_Interactive::set_fieldsel_pos(Coords c) {
	Interactive_Base::set_fieldsel_pos(c);
	if
		(c != get_fieldsel_pos()
		 and
		 SDL_GetMouseState(0, 0) & SDL_BUTTON(SDL_BUTTON_LEFT))
      field_clicked();
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

Bring up or close the tool menu
===============
*/
void Editor_Interactive::tool_menu_btn()
{
	if (m_toolmenu.window)
		delete m_toolmenu.window;
	else
		new Editor_Tool_Menu(this, &m_toolmenu, &tools, &m_options_menus);
}

/*
===============
Editor_Interactive::toggle_playermenu

Bring up or close the Player Menu
===============
*/
void Editor_Interactive::toggle_playermenu()
{
	if (m_playermenu.window)
		delete m_playermenu.window;
	else {
         this->select_tool(5,0);
         new Editor_Player_Menu(this,
               &tools, 5, 8,
               &m_playermenu);
   }

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
bool Editor_Interactive::handle_key(bool down, int code, char) {
   if(code==KEY_LCTRL || code==KEY_RCTRL) m_ctrl_down=down;

   if(down) {
      // only on down events
      switch(code) {
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
         case KEY_8:
            set_fieldsel_radius(7);
            return true;
         case KEY_9:
            set_fieldsel_radius(8);
            return true;
         case KEY_0:
            set_fieldsel_radius(9);
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

         case KEY_SPACE:
            toggle_buildhelp();
            return true;

         case KEY_c:
            set_display_flag(Interactive_Base::dfShowCensus,
                  !get_display_flag(Interactive_Base::dfShowCensus));
            return true;

         case KEY_e:
            toggle_eventmenu();
            return true;

         case KEY_f:
            if( down )
               g_gr->toggle_fullscreen();
            return true;

         case KEY_h:
            toggle_mainmenu();
            return true;

         case KEY_i:
            select_tool(0, 0);
            return true;

         case KEY_m:
            toggle_minimap();
            return true;

         case KEY_l:
            if(m_ctrl_down)
               new Main_Menu_Load_Map(this);
            return true;

         case KEY_p:
            toggle_playermenu();
            return true;

         case KEY_s:
            if(m_ctrl_down)
               new Main_Menu_Save_Map(this);
            return true;

         case KEY_t:
            tool_menu_btn();
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

/*
 * Reference functions
 *
 *  data is either a pointer to a trigger, event
 *  or a tribe (for buildings)
 */
void Editor_Interactive::reference_player_tribe
(const int player, const void * const data)
{
   assert(player>0 && player<=m_editor->get_map()->get_nrplayers());

   Player_References r;
   r.player=player;
   r.object=data;

   m_player_tribe_references.push_back(r);
}

/*
 * unreference !once!, if referenced many times, this
 * will leace a reference
 */
void Editor_Interactive::unreference_player_tribe
(const int player, const void * const data)
{
   assert(player>=0 && player<=m_editor->get_map()->get_nrplayers());
   assert(data);

   int i=0;
   if(player>0) {
      for(i=0; i<static_cast<int>(m_player_tribe_references.size()); i++)
         if(m_player_tribe_references[i].player==player && m_player_tribe_references[i].object==data) break;

      m_player_tribe_references.erase(m_player_tribe_references.begin() + i);
   } else {
      // Player is invalid, remove all references from this object
      for(i=0; i<static_cast<int>(m_player_tribe_references.size()); i++) {
         if(m_player_tribe_references[i].object==data) {
            m_player_tribe_references.erase(m_player_tribe_references.begin() + i); i=-1;
         }
      }
   }
}

bool Editor_Interactive::is_player_tribe_referenced(int player) {
   assert(player>0 && player<=m_editor->get_map()->get_nrplayers());

   uint i=0;
   for(i=0; i<m_player_tribe_references.size(); i++)
         if(m_player_tribe_references[i].player==player) return true;

   return false;
}
