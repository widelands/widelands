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

#include "widelands.h"
#include "editorinteractive.h"
#include "options.h"
#include "editor.h"
#include "map.h"
#include "player.h"
#include "minimap.h"
#include "editor_tools.h"
#include "editor_menus.h"


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
   Button *b;
   
   b = new Button(this, x, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_mainmenu);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_menu.png", RGBColor(0,0,255)));

   b = new Button(this, x+34, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::tool_menu_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/editor_menu_toggle_tool_menu.png", RGBColor(0,0,255)));

   b = new Button(this, x+68, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toolsize_menu_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/editor_menu_set_toolsize_menu.png", RGBColor(0,0,255)));

   b = new Button(this, x+102, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_minimap);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.png", RGBColor(0,0,255)));
   
   b = new Button(this, x+136, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_buildhelp);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_buildhelp.png", RGBColor(0,0,255)));

   // Init Tools
   tools.current_tool_index=1;
   tools.use_tool=0;
   tools.tools.push_back(new Editor_Info_Tool());
   Editor_Set_Height_Tool* sht=new Editor_Set_Height_Tool();
   tools.tools.push_back(new Editor_Increase_Height_Tool(new Editor_Decrease_Height_Tool(), sht));
   tools.tools.push_back(new Editor_Noise_Height_Tool(sht));
   tools.tools.push_back(new Editor_Set_Both_Terrain_Tool(new Editor_Set_Down_Terrain_Tool(), new Editor_Set_Right_Terrain_Tool()));
   tools.tools.push_back(new Editor_Place_Immovable_Tool(new Editor_Delete_Immovable_Tool()));
   
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
   unset_fsel_picture(); // reset default fsel
}

/*
===============
Editor_Interactive::start

Called just before the game starts, after postload, init and gfxload
===============
*/
void Editor_Interactive::start()
{
   int mapw;
	int maph;

   m_maprenderinfo.egbase = m_editor; 
	m_maprenderinfo.visibility = 0; 
	m_maprenderinfo.show_buildhelp = true;
	
	mapw = m_maprenderinfo.egbase->get_map()->get_width();
	maph = m_maprenderinfo.egbase->get_map()->get_height();
	m_maprenderinfo.overlay_basic = (uchar*)malloc(mapw*maph);
	m_maprenderinfo.overlay_roads = (uchar*)malloc(mapw*maph);
	memset(m_maprenderinfo.overlay_roads, 0, mapw*maph);
	
	for(int y = 0; y < maph; y++)
		for(int x = 0; x < mapw; x++) {
			FCoords coords = m_maprenderinfo.egbase->get_map()->get_fcoords(Coords(x,y));
			
			recalc_overlay(coords);
		}
}

/*
===============
Editor_Interactive::recalc_overlay

Recalculate build help and borders for the given field
===============
*/
void Editor_Interactive::recalc_overlay(FCoords fc)
{
   Map* map = m_maprenderinfo.egbase->get_map();

   // Only do recalcs after maprenderinfo has been setup
   if (!map)
      return;

   uchar code = 0;
   int owner = fc.field->get_owned_by();

   // A border is on every field that is owned by a player and has
   // neighbouring fields that are not owned by that player
   for(int dir = 1; dir <= 6; dir++) {
      FCoords neighb;

      map->get_neighbour(fc, dir, &neighb);

      if (neighb.field->get_owned_by() != owner)
         code = Overlay_Frontier_Base + owner;
   }

   int buildcaps=fc.field->get_caps();
   if(owner) {
      // Determine the buildhelp icon for that field
      buildcaps = m_editor->get_player(owner)->get_buildcaps(fc);
   }

   if (buildcaps & BUILDCAPS_MINE)
      code = Overlay_Build_Mine;
   else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG)
      code = Overlay_Build_Big;
   else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_MEDIUM)
      code = Overlay_Build_Medium;
   else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_SMALL)
      code = Overlay_Build_Small;
   else if (buildcaps & BUILDCAPS_FLAG)
      code = Overlay_Build_Flag;

   m_maprenderinfo.overlay_basic[fc.y*map->get_width() + fc.x] = code;
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
===========
Editor_Interactive::field_clicked()

This functions is called, when a field is clicked. it mainly calls
the function of the currently selected tool
===========
*/
void Editor_Interactive::field_clicked() {
   Map* m=get_map();
   int radius=tools.tools[tools.current_tool_index]->handle_click(tools.use_tool, &m_maprenderinfo.fieldsel, m->get_field(m_maprenderinfo.fieldsel), m, this);

   // Some things have changed, map is informed, logic is informed. But overlays may still be wrong. Recalc them
   MapRegion mr(m, m_maprenderinfo.fieldsel, radius);
   FCoords c;

   while(mr.next(&c)) {
      recalc_overlay(c);
   }
}

/*
===========
Editor_Interactive::toggle_buildhelp()

toggles the buildhelp on the map
===========
*/
void Editor_Interactive::toggle_buildhelp(void)
{
   m_maprenderinfo.show_buildhelp = !m_maprenderinfo.show_buildhelp;
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
		//new Editor_Preliminary_Tool_Menu(this, &m_toolmenu, &tools);
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
   tools.current_tool_index=n;
   tools.use_tool=which;
   
   set_fsel_picture(tools.tools[n]->get_fsel(which));
}


