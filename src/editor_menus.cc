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
#include "editor_menus.h"
#include "ui.h"
#include "editor.h"
#include "editor_tools_option_menus.h"
#include "editor_tools.h"
#include "ui_editbox.h"
#include "ui/ui_fs_menus/fullscreen_menu_fileview.h"
#include <string>

static const char* EDITOR_README =
"   README for the Widelands Map Editor\n"
"\n"
"\n"
"Introduction\n"
"------------\n"
"\n"
"This Editor is intended for players who'd like to design their own maps to use with Widelands "
"As you can see, this Editor is heavy work in progress and as the Editor becomes better and better "
"this text will also get longer and more complete. Probably it will also contain a little tutorial "
"in a while\n"
"\n"
"\n"
"Keyboard shortcuts\n"
"------------------\n"
"\n"
"SPACE    Toggles build-help\n"
"M        Toggles minimap\n"
"T        Toggles tools menu\n"
"\n"
"1-7      Changes tools size\n"
"SHIFT    (Hold) Selects first alternative tool while pressed\n"
"ALT      (Hold) Selects second alternative tool while pressed\n"
"\n"
"\n";

/*
=================================================

class Main_Menu_Map_Options

this is the Main Options Menu. Here, informations
about the current map are displayed and you can change
author, name and description

=================================================
*/
class Main_Menu_Map_Options : public Window {
   public:
      Main_Menu_Map_Options(Editor_Interactive*);
      virtual ~Main_Menu_Map_Options();

   private:
      void changed(int);
      Editor_Interactive *m_parent;
 //     std::string m_descr, m_author, m_name;
      Multiline_Textarea *m_descr;
      Textarea* m_world, *m_nrplayers, *m_size;
      Edit_Box* m_name, *m_author;
      void update();
};

/*
===============
Main_Menu_Map_Options::Main_Menu_Map_Options

Create all the buttons etc...
===============
*/
Main_Menu_Map_Options::Main_Menu_Map_Options(Editor_Interactive *parent)
	: Window(parent, (parent->get_w()-200)/2, (parent->get_h()-190)/2, 200, 190, "Map Options")
{
   m_parent=parent;

   // Caption
   Textarea* tt=new Textarea(this, 0, 0, "New Map Options", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   // Buttons
   const int offsx=5;
   const int offsy=30;
   const int spacing=3;
   const int height=20;
   int posx=offsx;
   int posy=offsy;
   Textarea* ta= new Textarea(this, posx, posy+5, "Map Name:", Align_Left);
   m_name=new Edit_Box(this, posx+ta->get_w()+spacing, posy, get_inner_w()-(posx+ta->get_w()+spacing)-spacing, 20, 1, 0);
   m_name->changedid.set(this, &Main_Menu_Map_Options::changed);
   posy+=height+spacing;
   ta=new Textarea(this, posx, posy+5, "Size:");
   m_size=new Textarea(this, posx+ta->get_w()+spacing, posy+5, "512x512", Align_Left);
   posy+=height+spacing;
   ta=new Textarea(this, posx, posy+5, "Nr Players:");
   m_nrplayers=new Textarea(this, posx+ta->get_w()+spacing, posy+5, "4", Align_Left);
   posy+=height+spacing;
   ta=new Textarea(this, posx, posy+5, "World:");
   m_world=new Textarea(this, posx+ta->get_w()+spacing, posy+5, "\"Greenland\"", Align_Left);
   posy+=height+spacing;
   ta=new Textarea(this, posx, posy+5, "Author:", Align_Left);
   m_author=new Edit_Box(this, posx+ta->get_w()+spacing, posy, get_inner_w()-(posx+ta->get_w()+spacing)-spacing, 20, 1, 1);
   m_author->changedid.set(this, &Main_Menu_Map_Options::changed);
   posy+=height+spacing;
   m_descr=new Multiline_Textarea(this, posx, posy, get_inner_w()-spacing-posx, get_inner_h()-spacing-posy, "Nothing defined!", Align_Left);
   update();
}

/*
===============
Main_Menu_Map_Options::update()

Updates all Textareas in the Window to represent currently
set values
==============
*/
void Main_Menu_Map_Options::update(void) {
   Map* map=m_parent->get_map();

   char buf[200];
   sprintf(buf, "%ix%i", map->get_width(), map->get_height());
   m_size->set_text(buf);
   m_author->set_text(map->get_author());
   m_name->set_text(map->get_name());
   sprintf(buf, "%i", map->get_nrplayers());
   m_nrplayers->set_text(buf);
   m_world->set_text(map->get_world_name());
   m_descr->set_text("TODO: multiline editbox!!!");
}



/*
===============
Main_Menu_Map_Options::~Main_Menu_Map_Options

Unregister from the registry pointer
===============
*/
Main_Menu_Map_Options::~Main_Menu_Map_Options()
{
}

/*
===========
Main_Menu_Map_Options::changed()

called when one of the editboxes are changed
===========
*/
void Main_Menu_Map_Options::changed(int id) {
   if(id==0) {
      // name
      m_parent->get_map()->set_name(m_name->get_text());
   } else if(id==1) {
      // author
      m_parent->get_map()->set_author(m_author->get_text());
   }
   update();
}

/*
=================================================

class Main_Menu_New_Map

This is the new map selection menu. It offers 
the user to choose the new world and a few other 
things like size, world ....

=================================================
*/
class Main_Menu_New_Map : public Window {
   public:
      Main_Menu_New_Map(Editor_Interactive*);
      virtual ~Main_Menu_New_Map();

   private:
      Editor_Interactive *m_parent;
      Textarea *m_width, *m_height;
      Button* m_world;
      int m_w, m_h;
      std::string m_worldstr;
      
      void button_clicked(int);
};

/*
===============
Main_Menu_New_Map::Main_Menu_New_Map

Create all the buttons etc...
===============
*/
Main_Menu_New_Map::Main_Menu_New_Map(Editor_Interactive *parent)
	: Window(parent, (parent->get_w()-140)/2, (parent->get_h()-150)/2, 140, 150, "New Map")
{
   m_parent=parent;

   // Caption
   Textarea* tt=new Textarea(this, 0, 0, "New Map Options", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   // Buttons
   char buf[250];
   const int offsx=5;
   const int offsy=30;
   const int spacing=5;
   const int width=get_inner_w()-offsx*2;
   const int height=20;
   int posx=offsx; 
   int posy=offsy;
   m_w=0; m_h=0;
   sprintf(buf, "Width: %i", MAP_DIMENSIONS[m_w]);
   m_width=new Textarea(this, posx+spacing+20, posy+7, buf, Align_Left);
   Button* b = new Button(this, posx, posy, 20, 20, 1, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   b = new Button(this, get_inner_w()-spacing-20, posy, 20, 20, 1, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   posy+=20+spacing+spacing;
   
   sprintf(buf, "Height: %i", MAP_DIMENSIONS[m_h]);
   m_height=new Textarea(this, posx+spacing+20, posy+7, buf, Align_Left);
   b = new Button(this, posx, posy, 20, 20, 1, 2);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   b = new Button(this, get_inner_w()-spacing-20, posy, 20, 20, 1, 3);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   posy+=20+spacing+spacing;

   m_worldstr="greenland";
   m_world=new Button(this, posx, posy, width, height, 1, 4);
   m_world->set_title(m_worldstr.c_str());
   m_world->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   posy+=height+spacing+spacing+spacing;

   b=new Button(this, posx, posy, width, height, 0, 5);
   b->set_title("Create Map");
   b->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   posy+=height+spacing;
}


/*
===========
Main_Menu_New_Map Button functions

called, when button get clicked
===========
*/
void Main_Menu_New_Map::button_clicked(int n) {
   switch(n) {
      case 0: m_w++; break;
      case 1: m_w--; break;
      case 2: m_h++; break;
      case 3: m_h--; break;
      case 4: log("TODO: switch worlds!\n"); break;
      case 5: log("TODO: create new map!\n"); break;
   }

   char buf[200];
   if(m_w<0) m_w=0;
   if(m_w>=NUMBER_OF_MAP_DIMENSIONS) m_w=NUMBER_OF_MAP_DIMENSIONS-1;
   if(m_h<0) m_h=0;
   if(m_h>=NUMBER_OF_MAP_DIMENSIONS) m_h=NUMBER_OF_MAP_DIMENSIONS-1;
   sprintf(buf, "Width: %i", MAP_DIMENSIONS[m_w]);
   m_width->set_text(buf);
   sprintf(buf, "Height: %i", MAP_DIMENSIONS[m_h]);
   m_height->set_text(buf);
}

/*
===============
Main_Menu_New_Map::~Main_Menu_New_Map

Unregister from the registry pointer
===============
*/
Main_Menu_New_Map::~Main_Menu_New_Map()
{
}


/*
=================================================

class Editor_Main_Menu

=================================================
*/

/*
===============
Editor_Main_Menu::Editor_Main_Menu

Create all the buttons etc...
===============
*/
Editor_Main_Menu::Editor_Main_Menu(Editor_Interactive *parent, UniqueWindowRegistry *registry)
	: UniqueWindow(parent, registry, 130, 200, "Main Menu")
{
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
   b=new Button(this, posx, posy, width, height, 1);
   b->set_title("View Readme");
   b->clicked.set(this, &Editor_Main_Menu::readme_btn);
   posy+=height+spacing;

   posy+=spacing;
   b=new Button(this, posx, posy, width, height, 0);
   b->set_title("Exit Editor");
   b->clicked.set(this, &Editor_Main_Menu::exit_btn);
   posy+=height+spacing;

	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

/*
===========
Editor_Main_Menu Button functions

called, when buttons get clicked
===========
*/
void Editor_Main_Menu::new_map_btn() {
   new Main_Menu_New_Map(m_parent);
   delete this;
}
void Editor_Main_Menu::load_btn() {
   log("TODO: Editor_Main_Menu::load_btn()\n");
   /*
   m_parent->get_editor()->get_objects()->cleanup(m_parent->get_editor());
   g_anim.flush();

   //map_select_menue(m_parent->get_editor());
   //m_parent->map_changed();

   g_gr->flush(PicMod_Menu);

   m_parent->get_editor()->postload();
   m_parent->get_editor()->load_graphics();
*/
}

void Editor_Main_Menu::save_btn() {
   log("TODO: Editor_Main_Menu::save_btn()\n");
}
void Editor_Main_Menu::map_options_btn() {
   new Main_Menu_Map_Options(m_parent);
   delete this;
}
void Editor_Main_Menu::exit_btn() {
   m_parent->exit_editor();
}
void Editor_Main_Menu::readme_btn() {
   textview_window(m_parent, &m_window_readme, "Editor README", EDITOR_README);
}

/*
===============
Editor_Main_Menu::~Editor_Main_Menu

Unregister from the registry pointer
===============
*/
Editor_Main_Menu::~Editor_Main_Menu()
{
}

/*
=================================================

class Editor_Tool_Menu

=================================================
*/

/*
===============
Editor_Tool_Menu::Editor_Tool_Menu

Create all the buttons etc...
===============
*/
Editor_Tool_Menu::Editor_Tool_Menu(Editor_Interactive *parent, UniqueWindowRegistry *registry,
                                   Editor_Interactive::Editor_Tools* tools)
	: UniqueWindow(parent, registry, 350, 400, "Tool Menu")
{
   m_tools=tools;
   m_parent=parent;


   // Buttons
   const int offsx=5;
   const int offsy=30;
   const int spacing=5;
   const int width=34;
   const int height=34;
   int posx=offsx;
   int posy=offsy;

   m_radioselect=new Radiogroup();
   m_radioselect->add_button(this, posx, posy, g_gr->get_picture(PicMod_Game, "pics/editor_menu_tool_change_height.png", RGBColor(0, 0, 255)));
   posx+=width+spacing;
   m_radioselect->add_button(this, posx, posy, g_gr->get_picture(PicMod_Game, "pics/editor_menu_tool_noise_height.png", RGBColor(0,0,255)));
   posx=offsx;
   posy+=spacing+height;
   m_radioselect->add_button(this, posx, posy, g_gr->get_picture(PicMod_Game, "pics/editor_menu_tool_set_terrain.png", RGBColor(0,0,255)));
   posx+=width+spacing;
   m_radioselect->add_button(this, posx, posy, g_gr->get_picture(PicMod_Game, "pics/editor_menu_tool_place_bob.png", RGBColor(0,0,255)));

   set_inner_size(offsx+(width+spacing)*2, offsy+(height+spacing)*2);

   Textarea* ta=new Textarea(this, 0, 0, "Tool Menu");
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

   if (m_options.window) {
      delete m_options.window;
   }
  
   switch(n) {
      case 0: 
         m_parent->select_tool(1, 0); 
         new Editor_Tool_Change_Height_Options_Menu(m_parent, 
               static_cast<Editor_Increase_Height_Tool*>(m_tools->tools[1]),
               &m_options);
         break;
   
      case 1:
         m_parent->select_tool(2,0);
         new Editor_Tool_Noise_Height_Options_Menu(m_parent,
               static_cast<Editor_Noise_Height_Tool*>(m_tools->tools[2]),
               &m_options);
         break;

      case 2:
         m_parent->select_tool(3,0);
         new Editor_Tool_Set_Terrain_Tool_Options_Menu(m_parent,
               static_cast<Editor_Set_Both_Terrain_Tool*>(m_tools->tools[3]),
               &m_options);
         break;

      case 3:
         m_parent->select_tool(4,0);
         new Editor_Tool_Place_Immovable_Options_Menu(m_parent,
               static_cast<Editor_Place_Immovable_Tool*>(m_tools->tools[4]),
               &m_options);
      default: break;
   }
}

/*
=================================================

class Editor_Toolsize_Menu

=================================================
*/

/*
===============
Editor_Toolsize_Menu::Editor_Toolsize_Menu

Create all the buttons etc...
===============
*/
Editor_Toolsize_Menu::Editor_Toolsize_Menu(Editor_Interactive *parent, UniqueWindowRegistry *registry)
	: UniqueWindow(parent, registry, 160, 65, "Toolsize Menu")
{
   m_parent=parent;

   new Textarea(this, 15, 5, "Set Tool Size Menu", Align_Left);
   char buf[250];
   sprintf(buf, "Current Size: %i", m_parent->get_fieldsel_radius()+1);
   m_textarea=new Textarea(this, 25, 25, buf);

   int bx=60;
   Button* b = new Button(this, bx, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Toolsize_Menu::button_clicked);
   b=new Button(this, bx+20, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Toolsize_Menu::button_clicked);

	if (get_usedefaultpos())
		center_to_parent();
}

/*
===============
Editor_Toolsize_Menu::~Editor_Toolsize_Menu

Unregister from the registry pointer
===============
*/
Editor_Toolsize_Menu::~Editor_Toolsize_Menu()
{
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

