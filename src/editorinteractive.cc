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

#include "widelands.h"
#include "editorinteractive.h"
#include "options.h"
#include "editor.h"
#include "map.h"
#include "player.h"
#include "minimap.h"
#include "editor_tools.h"

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
};

/*
===============
Editor_Main_Menu::Editor_Main_Menu

Create all the buttons etc...
===============
*/
Editor_Main_Menu::Editor_Main_Menu(Editor_Interactive *parent, UniqueWindow *registry)
	: Window(parent, (parent->get_w()-130)/2, (parent->get_h()-170)/2, 130, 170, "Main Menu")
{
	m_registry = registry;
	if (m_registry) {
		if (m_registry->window)
			delete m_registry->window;
		
		m_registry->window = this;
		if (m_registry->x >= 0)
			set_pos(m_registry->x, m_registry->y);
	}
   m_parent=parent;

   // Caption
   Textarea* tt=new Textarea(this, 0, 0, "Editor Main Menu", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   // Buttons
   const int offsx=5;
   const int offsy=30;
   const int spacing=5;
   const int width=get_inner_w()-offsx*2;
   const int height=20;
   int posx=offsx; 
   int posy=offsy;
   Button* b=new Button(this, posx, posy, width, height, 1);
   b->set_title("New Map");
   b->clicked.set(this, &Editor_Main_Menu::new_map_btn);
   posy+=height+spacing;
   
   b=new Button(this, posx, posy, width, height, 1);
   b->set_title("Load Map");
   b->clicked.set(this, &Editor_Main_Menu::load_btn);
   posy+=height+spacing;
   
   b=new Button(this, posx, posy, width, height, 1);
   b->set_title("Save Map");
   b->clicked.set(this, &Editor_Main_Menu::save_btn);
   posy+=height+spacing;
  
   posy+=spacing;
   b=new Button(this, posx, posy, width, height, 1);
   b->set_title("Map Options");
   b->clicked.set(this, &Editor_Main_Menu::map_options_btn);
   posy+=height+spacing;
   
   posy+=spacing;
   b=new Button(this, posx, posy, width, height, 0);
   b->set_title("Exit Editor");
   b->clicked.set(this, &Editor_Main_Menu::exit_btn);
   posy+=height+spacing;
}

/*
===========
Editor_Main_Menu Button functions

called, when buttons get clicked
===========
*/
void Editor_Main_Menu::new_map_btn() {
   // TODO
}
void Editor_Main_Menu::load_btn() {
   
   m_parent->get_editor()->get_objects()->cleanup(m_parent->get_editor());
   g_anim.flush();

   //map_select_menue(m_parent->get_editor());
   std::cerr << "ALIVE!" << std::endl;
   //m_parent->map_changed();
   
   g_gr->flush(PicMod_Menu);
		
   m_parent->get_editor()->postload();
   m_parent->get_editor()->load_graphics();
}

void Editor_Main_Menu::save_btn() {
   // TODO
}
void Editor_Main_Menu::map_options_btn() {
   // TODO
}
void Editor_Main_Menu::exit_btn() {
   m_parent->exit_editor();
}

/*
===============
Editor_Main_Menu::~Editor_Main_Menu

Unregister from the registry pointer
===============
*/
Editor_Main_Menu::~Editor_Main_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}


/*
=============================

class Editor_Tool_Menu

This class is the tool selection window/menu. 
Here, you can select the tool you wish to use the next time

=============================
*/

class Editor_Tool_Menu : public Window {
   public:
      Editor_Tool_Menu(Editor_Interactive*, UniqueWindow*, Editor_Interactive::Editor_Tools*);
      virtual ~Editor_Tool_Menu();

   private:
      Editor_Interactive::Editor_Tools* m_tools;
      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;
      Radiogroup* m_radioselect;

      void changed_to_function(int);
      void options_button_clicked(int);
};

/*
===============
Editor_Tool_Menu::Editor_Tool_Menu

Create all the buttons etc...
===============
*/
Editor_Tool_Menu::Editor_Tool_Menu(Editor_Interactive *parent, UniqueWindow *registry, Editor_Interactive::Editor_Tools* tools)
	: Window(parent, (parent->get_w()-350)/2, (parent->get_h()-400)/2, 350, 400, "Tool Menu")
{
	m_registry = registry;
	if (m_registry) {
		if (m_registry->window)
			delete m_registry->window;
		
		m_registry->window = this;
		if (m_registry->x >= 0)
			set_pos(m_registry->x, m_registry->y);
	}
   m_tools=tools;
   m_parent=parent;

   m_radioselect=new Radiogroup();
   m_radioselect->changedto.set(this, &Editor_Tool_Menu::changed_to_function);

   int y = 5;
   uint i;
   for(i = 0; i < m_tools->tools.size(); i++, y+= 25) {
      char buf[32];
      m_radioselect->add_button(this, 5, y);
      sprintf(buf, "%s", m_tools->tools[i]->tool->get_name());
      new Textarea(this, 55, y+10, buf, Align_VCenter);
      if(m_tools->tools[i]->tool->has_options()) {
         Button* b = new Button(this, 30, y+3, 14, 14, 0, i);
         b->set_title("O");
         b->clickedid.set(this, &Editor_Tool_Menu::options_button_clicked);
      }
   }
   m_radioselect->set_state(m_tools->current_tool);

}

/*
===============
Editor_Tool_Menu::~Editor_Tool_Menu

Unregister from the registry pointer
===============
*/
Editor_Tool_Menu::~Editor_Tool_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}

/*
===========
Editor_Tool_Menu::changed_to_function()

called when the listselect changes
===========
*/
void Editor_Tool_Menu::changed_to_function(int n) {
   m_parent->select_tool(n, true);
}

/*
===========
Editor_Tool_Menu::options_button_clicked()

called when one of the options buttons has been clicked
===========
*/
void Editor_Tool_Menu::options_button_clicked(int n) {
   m_tools->tools[n]->tool->tool_options_dialog(m_parent);
   m_radioselect->set_state(n);
}

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

/*
===============
Editor_Toolsize_Menu::Editor_Toolsize_Menu

Create all the buttons etc...
===============
*/
Editor_Toolsize_Menu::Editor_Toolsize_Menu(Editor_Interactive *parent, UniqueWindow *registry)
	: Window(parent, (parent->get_w()-102)/2, (parent->get_h()-136)/2, 160, 65, "Toolsize Menu")
{
   m_registry = registry;
   if (m_registry) {
      if (m_registry->window)
         delete m_registry->window;

      m_registry->window = this;
      if (m_registry->x >= 0)
         set_pos(m_registry->x, m_registry->y);
   }
   m_parent=parent;

   new Textarea(this, 15, 5, "Set Tool Size Menu", Align_Left);
   char buf[250];
   sprintf(buf, "Current Size: %i", m_parent->get_fieldsel_radius()+1);
   m_textarea=new Textarea(this, 25, 25, buf);

   int bx=60;
   Button* b = new Button(this, bx, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.bmp", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Toolsize_Menu::button_clicked);
   b=new Button(this, bx+20, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.bmp", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Toolsize_Menu::button_clicked);
}

/*
===============
Editor_Toolsize_Menu::~Editor_Toolsize_Menu

Unregister from the registry pointer
===============
*/
Editor_Toolsize_Menu::~Editor_Toolsize_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}

/*
===========
Editor_Toolsize_Menu::button_clicked()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Toolsize_Menu::button_clicked(int n) {
   int val=m_parent->get_fieldsel_radius();
   if(n==0) {
      ++val;
      if(val>MAX_TOOL_AREA) val=MAX_TOOL_AREA;
   } else if(n==1) {
      --val;
      if(val<0) val=0;
   }
   m_parent->set_fieldsel_radius(val);

   char buf[250];
   sprintf(buf, "Current Size: %i", m_parent->get_fieldsel_radius()+1);
   m_textarea->set_text(buf);
}

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
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_menu.bmp", RGBColor(0,0,255)));

   b = new Button(this, x+34, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::tool_menu_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/editor_menu_toggle_tool_menu.bmp", RGBColor(0,0,255)));

   b = new Button(this, x+68, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toolsize_menu_btn);
//   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.bmp", RGBColor(0,0,255)));
   b->set_title("TS");

   b = new Button(this, x+102, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_minimap);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.bmp", RGBColor(0,0,255)));
   
   b = new Button(this, x+136, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_buildhelp);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_buildhelp.bmp", RGBColor(0,0,255)));

   // Init Tools
   tools.current_tool=0;
   tools.last_tool=-1;
   tools.using_linked_tool=false;
   tools.tools.push_back(new Tool_Info(0, 0, new Editor_Info_Tool()));
   tools.tools.push_back(new Tool_Info(2, 3, new Editor_Increase_Height_Tool()));
   tools.tools.push_back(new Tool_Info(1, 3, new Editor_Decrease_Height_Tool()));
   tools.tools.push_back(new Tool_Info(1, 2, new Editor_Set_Height_Tool()));
   tools.tools.push_back(new Tool_Info(4, 4, new Editor_Noise_Height_Tool()));
   tools.tools.push_back(new Tool_Info(6, 7, new Editor_Set_Right_Terrain_Tool()));
   tools.tools.push_back(new Tool_Info(5, 7, new Editor_Set_Down_Terrain_Tool()));
   tools.tools.push_back(new Tool_Info(5, 6, new Editor_Set_Both_Terrain_Tool()));
   select_tool(1, true);
}

/****************************************
 * Editor_Interactive::~EditorInteractive()
 *
 * cleanup
 */
Editor_Interactive::~Editor_Interactive() {
   while(tools.tools.size()) {
      delete tools.tools.back()->tool;
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
	m_maprenderinfo.map = m_editor->get_map();
	m_maprenderinfo.visibility = 0; 
	m_maprenderinfo.show_buildhelp = true;
	
	mapw = m_maprenderinfo.map->get_width();
	maph = m_maprenderinfo.map->get_height();
	m_maprenderinfo.overlay_basic = (uchar*)malloc(mapw*maph);
	m_maprenderinfo.overlay_roads = (uchar*)malloc(mapw*maph);
	memset(m_maprenderinfo.overlay_roads, 0, mapw*maph);
	
	for(int y = 0; y < maph; y++)
		for(int x = 0; x < mapw; x++) {
			FCoords coords(x, y, m_maprenderinfo.map->get_field(x,y));
			
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
   Map* map = m_maprenderinfo.map;

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
   int radius=tools.tools[tools.current_tool]->tool->handle_click(&m_maprenderinfo.fieldsel, m->get_field(m_maprenderinfo.fieldsel), m, this);

   // Some things have changed, map is informed, logic is informed. But overlays may still be wrong. Recalc them
   Map_Region_Coords mrc(m_maprenderinfo.fieldsel, radius, m);
   Map_Region mr(m_maprenderinfo.fieldsel, radius, m);
   FCoords f;
   while((f.field=mr.next())) {
      mrc.next(&f.x, &f.y);
      recalc_overlay(f);
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
            if(!tools.using_linked_tool) {
               tools.using_linked_tool=true;
               select_tool(tools.tools[tools.current_tool]->f_linked_tool, false);
            }
            return true;

         case KEY_LALT:
         case KEY_RALT:
         case KEY_MODE:
            if(!tools.using_linked_tool) {
               tools.using_linked_tool=true;
               select_tool(tools.tools[tools.current_tool]->s_linked_tool, false);
            }
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
            if(tools.using_linked_tool) {
               select_tool(tools.last_tool, true);
               tools.using_linked_tool=false;
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
void Editor_Interactive::select_tool(int n, bool new_default_tool) {
   tools.last_tool=tools.current_tool;
   tools.current_tool=n;

   set_fsel_picture(tools.tools[n]->tool->get_fsel());

   if(new_default_tool) {
      tools.last_tool=-1;
      tools.using_linked_tool=false;
   }
}


