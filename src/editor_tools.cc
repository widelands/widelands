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

#include "widelands.h"
#include "editorinteractive.h"
#include "editor_tools.h"
#include "ui.h"
#include "map.h"
#include "graphic.h"
#include "sw16_graphic.h"
#include <string>

using std::string;

/*
=============================

class Editor_Info_Tool

=============================
*/

/*
===========
Editor_Info_Tool::handle_click()

show a simple info dialog with infos about this field
===========
*/
int Editor_Info_Tool::handle_click(const Coords* coordinates, Field* f, Map* map, Editor_Interactive* parent) {
   Window* w = new Window(parent, 30, 30, 400, 200, "Field Information");
   Multiline_Textarea* multiline_textarea = new Multiline_Textarea(w, 0, 0, w->get_inner_w(), w->get_inner_h(), 0);

   string buf;
   char buf1[1024];
   
   buf += "1) Field Infos\n";
   sprintf(buf1, " Coordinates: (%i/%i)\n", coordinates->x, coordinates->y); buf+=buf1;
   sprintf(buf1, " Height: %i\n", f->get_height()); buf+=buf1;
   buf+=" Caps: ";
   switch((f->get_caps() & BUILDCAPS_SIZEMASK)) {
      case BUILDCAPS_SMALL: buf+="small"; break; 
      case BUILDCAPS_MEDIUM: buf+="medium"; break; 
      case BUILDCAPS_BIG: buf+="big"; break;
      default: break;
   }
   if(f->get_caps() & BUILDCAPS_FLAG) buf+=" flag";
   if(f->get_caps() & BUILDCAPS_MINE) buf+=" mine";
   if(f->get_caps() & BUILDCAPS_PORT) buf+=" port";
   if(f->get_caps() & MOVECAPS_WALK) buf+=" walk";
   if(f->get_caps() & MOVECAPS_SWIM) buf+=" swim";
   buf+="\n";
   sprintf(buf1, " Owned by: %i\n", f->get_owned_by()); buf+=buf1;
   sprintf(buf1, " Has base immovable: %s (TODO! more info)\n", f->get_immovable() ? "Yes" : "No"); buf+=buf1;
   sprintf(buf1, " Has bobs: %s (TODO: more informations)\n", f->get_first_bob() ? "Yes" : "No"); buf+=buf1;
   sprintf(buf1, " Roads: TODO!\n"); buf+=buf1;
   
   buf += "\n";
   Terrain_Descr* ter=f->get_terr();
   buf += "2) Right Terrain Info\n";
   sprintf(buf1, " Name: %s\n", ter->get_name()); buf+=buf1;
   sprintf(buf1, " Texture Number: %i\n", ter->get_texture()); buf+=buf1;

   buf += "\n";
   ter=f->get_terd();
   buf += "3) Down Terrain Info\n";
   sprintf(buf1, " Name: %s\n", ter->get_name()); buf+=buf1;
   sprintf(buf1, " Texture Number: %i\n", ter->get_texture()); buf+=buf1;

   buf += "\n";
   buf += "4) Map Info";
   sprintf(buf1, " Name: %s\n", map->get_name()); buf+=buf1;
   sprintf(buf1, " Size: %ix%i\n", map->get_width(), map->get_height()); buf+=buf1;
   sprintf(buf1, " Author: %s\n", map->get_author()); buf+=buf1;
   sprintf(buf1, " Descr: %s\n", map->get_description()); buf+=buf1;
   sprintf(buf1, " Number of Players: %i\n", map->get_nrplayers()); buf+=buf1;
   sprintf(buf1, " TODO: more information (number of resources, number of terrains...)\n"); buf+=buf1;

   buf += "\n";
   buf += "5) World Info";
   sprintf(buf1, " Name: %s\n", map->get_world()->get_name()); buf+=buf1;
   sprintf(buf1, " Author: %s\n", map->get_world()->get_author()); buf+=buf1;
   sprintf(buf1, " Descr: %s\n", map->get_world()->get_descr()); buf+=buf1;
   sprintf(buf1, " TODO -- More information (Number of bobs/number of wares...)\n"); buf+=buf1;
   
   buf += "\n";
   buf += "\n";
   buf += "\n";
   buf += "\n";
   buf += "\n";
         
   multiline_textarea->set_text(buf.c_str()); 

   return 0;
}

/*
=============================

class Editor_Increase_Height_Tool_Options_Menu

this is the option menu for this tool

=============================
*/
class Editor_Increase_Height_Tool_Options_Menu : public Window {
   public:
      Editor_Increase_Height_Tool_Options_Menu(Editor_Interactive*, UniqueWindow*, int*);
      virtual ~Editor_Increase_Height_Tool_Options_Menu();

   private:
      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;
      int* m_changed_by;
      Textarea* m_textarea; 
      
      void button_clicked(int);
};

/*
===============
Editor_Increase_Height_Tool_Options_Menu::Editor_Increase_Height_Tool_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Increase_Height_Tool_Options_Menu::Editor_Increase_Height_Tool_Options_Menu(Editor_Interactive *parent, UniqueWindow *registry, int* changed_by)
	: Window(parent, (parent->get_w()-400)/2, (parent->get_h()-100)/2, 210, 70, "Option Menu")
{
   m_registry = registry;
	if (m_registry) {
		if (m_registry->window)
			delete m_registry->window;
		
		m_registry->window = this;
		if (m_registry->x >= 0)
			set_pos(m_registry->x, m_registry->y);
	}
   m_changed_by=changed_by;
   m_parent=parent;

   new Textarea(this, 5, 5, "Increase Height Tool Options", Align_Left);
   char buf[250];
   sprintf(buf, "Increase by: %i", *m_changed_by);
   m_textarea=new Textarea(this, 50, 25, buf);

   Button* b = new Button(this, 85, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Increase_Height_Tool_Options_Menu::button_clicked);
   b=new Button(this, 105, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Increase_Height_Tool_Options_Menu::button_clicked);
}

/*
===============
Editor_Increase_Height_Tool_Options_Menu::~Editor_Increase_Height_Tool_Options_Menu

Unregister from the registry pointer
===============
*/
Editor_Increase_Height_Tool_Options_Menu::~Editor_Increase_Height_Tool_Options_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}

/*
===========
Editor_Increase_Height_Tool_Options_Menu::button_clicked()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Increase_Height_Tool_Options_Menu::button_clicked(int n) {
   int val=*m_changed_by;
   if(n==0) {
      ++val;
      if(val>MAX_FIELD_HEIGHT_DIFF) val=MAX_FIELD_HEIGHT_DIFF;
   } else if(n==1) {
      --val;
      if(val<0) val=0;
   }
   *m_changed_by=val;
   
   char buf[250];
   sprintf(buf, "Increase by: %i", *m_changed_by);
   m_textarea->set_text(buf);
}

/*
=============================

class Editor_Increase_Height_Tool

=============================
*/

/*
===========
Editor_Increase_Height_Tool::handle_click()

increase the height of the current field by one,
this increases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Increase_Height_Tool::handle_click(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   Map_Region_Coords mrc(*coordinates, parent->get_fieldsel_radius(), map);
   int mx, my;
 
   int max, i;
   max=0;
   while(mrc.next(&mx, &my)) {
      i=map->change_field_height(Coords(mx,my), m_increase_by);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}

/*
===========
Editor_Increase_Height_Tool::tool_options_dialog()

Launch the option window
===========
*/
int Editor_Increase_Height_Tool::tool_options_dialog(Editor_Interactive* parent) {
   if (m_w.window)
		delete m_w.window;
	else
		new Editor_Increase_Height_Tool_Options_Menu(parent, &m_w, &m_increase_by);
   return 0;
}

/*
=============================

class Editor_Decrease_Height_Tool_Options_Menu

this is the option menu for this tool

=============================
*/
class Editor_Decrease_Height_Tool_Options_Menu : public Window {
   public:
      Editor_Decrease_Height_Tool_Options_Menu(Editor_Interactive*, UniqueWindow*, int*);
      virtual ~Editor_Decrease_Height_Tool_Options_Menu();

   private:
      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;
      int* m_changed_by;
      Textarea* m_textarea; 
      
      void button_clicked(int);
};

/*
===============
Editor_Decrease_Height_Tool_Options_Menu::Editor_Decrease_Height_Tool_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Decrease_Height_Tool_Options_Menu::Editor_Decrease_Height_Tool_Options_Menu(Editor_Interactive *parent, UniqueWindow *registry, int* changed_by)
	: Window(parent, (parent->get_w()-300)/2, (parent->get_h()-100)/2, 210, 70, "Option Menu")
{
   m_registry = registry;
	if (m_registry) {
		if (m_registry->window)
			delete m_registry->window;
		
		m_registry->window = this;
		if (m_registry->x >= 0)
			set_pos(m_registry->x, m_registry->y);
	}
   m_changed_by=changed_by;
   m_parent=parent;
   
   new Textarea(this, 5, 5, "Decrease Height Tool Options", Align_Left);
   char buf[250];
   sprintf(buf, "Decrease by: %i", *m_changed_by);
   m_textarea=new Textarea(this, 50, 25, buf);

   Button* b = new Button(this, 85, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Decrease_Height_Tool_Options_Menu::button_clicked);
   b=new Button(this, 105, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Decrease_Height_Tool_Options_Menu::button_clicked);
}

/*
===============
Editor_Decrease_Height_Tool_Options_Menu::~Editor_Decrease_Height_Tool_Options_Menu

Unregister from the registry pointer
===============
*/
Editor_Decrease_Height_Tool_Options_Menu::~Editor_Decrease_Height_Tool_Options_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}

/*
===========
Editor_Decrease_Height_Tool_Options_Menu::button_clicked()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Decrease_Height_Tool_Options_Menu::button_clicked(int n) {
   int val=*m_changed_by;
   if(n==0) {
      ++val;
      if(val>MAX_FIELD_HEIGHT_DIFF) val=MAX_FIELD_HEIGHT_DIFF;
   } else if(n==1) {
      --val;
      if(val<0) val=0;
   }
   *m_changed_by=val;
   
   char buf[250];
   sprintf(buf, "Decrease by: %i", *m_changed_by);
   m_textarea->set_text(buf);
}

/*
=============================

class Editor_Decrease_Height_Tool

=============================
*/

/*
===========
Editor_Decrease_Height_Tool::handle_click()

decrease the height of the current field by one,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Decrease_Height_Tool::handle_click(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   Map_Region_Coords mrc(*coordinates, parent->get_fieldsel_radius(), map);
   int mx, my;
  
   int max,i;
   max=0;
   while(mrc.next(&mx, &my)) {
      i=map->change_field_height(Coords(mx,my), -m_decrease_by);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}

/*
===========
Editor_Decrease_Height_Tool::tool_options_dialog()

Calls the Tool Option dialog
===========
*/
int Editor_Decrease_Height_Tool::tool_options_dialog(Editor_Interactive* parent) {
   if (m_w.window)
      delete m_w.window;
   else
      new Editor_Decrease_Height_Tool_Options_Menu(parent, &m_w, &m_decrease_by);
   return 0;
}

/*
=============================

class Editor_Set_Height_Tool_Options_Menu

this is the option menu for this tool

=============================
*/
class Editor_Set_Height_Tool_Options_Menu : public Window {
   public:
      Editor_Set_Height_Tool_Options_Menu(Editor_Interactive*, UniqueWindow*, int*);
      virtual ~Editor_Set_Height_Tool_Options_Menu();

   private:
      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;
      int* m_change_to;
      Textarea* m_textarea; 
      
      void button_clicked(int);
};

/*
===============
Editor_Set_Height_Tool_Options_Menu::Editor_Set_Height_Tool_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Set_Height_Tool_Options_Menu::Editor_Set_Height_Tool_Options_Menu(Editor_Interactive *parent, UniqueWindow *registry, int* change_to)
	: Window(parent, (parent->get_w()-400)/2, (parent->get_h()-100)/2, 190, 70, "Option Menu")
{
   m_registry = registry;
   if (m_registry) {
      if (m_registry->window)
         delete m_registry->window;

      m_registry->window = this;
      if (m_registry->x >= 0)
         set_pos(m_registry->x, m_registry->y);
   }
   m_change_to=change_to;
   m_parent=parent;

   new Textarea(this, 10, 5, "Set Height Tool Options", Align_Left);
   char buf[250];
   sprintf(buf, "Set to: %i", *m_change_to);
   m_textarea=new Textarea(this, 59, 25, buf);

   Button* b = new Button(this, 75, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Set_Height_Tool_Options_Menu::button_clicked);
   b=new Button(this, 95, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Set_Height_Tool_Options_Menu::button_clicked);
}

/*
===============
Editor_Set_Height_Tool_Options_Menu::~Editor_Set_Height_Tool_Options_Menu

Unregister from the registry pointer
===============
*/
Editor_Set_Height_Tool_Options_Menu::~Editor_Set_Height_Tool_Options_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}

/*
===========
Editor_Set_Height_Tool_Options_Menu::button_clicked()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Set_Height_Tool_Options_Menu::button_clicked(int n) {
   int val=*m_change_to;
   if(n==0) {
      ++val;
      if(val>MAX_FIELD_HEIGHT) val=MAX_FIELD_HEIGHT;
   } else if(n==1) {
      --val;
      if(val<0) val=0;
   }
   *m_change_to=val;

   char buf[250];
   sprintf(buf, "Set to: %i", *m_change_to);
   m_textarea->set_text(buf);
}

/*
=============================

class Editor_Set_Height_Tool

=============================
*/

/*
===========
Editor_Set_Height_Tool::handle_click()

sets the height of the current to a fixed value,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Set_Height_Tool::handle_click(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   

   Map_Region_Coords mrc(*coordinates, parent->get_fieldsel_radius(), map);
   int mx, my;
   int i, max;
   max=0;
   while(mrc.next(&mx, &my)) {
      i=map->set_field_height(mx, my, m_set_to);
      if(i>max) max=i;
   }

   return parent->get_fieldsel_radius()+max;
}

/*
===========
Editor_Set_Height_Tool::tool_options_dialog()

Calls the Tool Option dialog
===========
*/
int Editor_Set_Height_Tool::tool_options_dialog(Editor_Interactive* parent) {
   if (m_w.window)
		delete m_w.window;
	else
		new Editor_Set_Height_Tool_Options_Menu(parent, &m_w, &m_set_to);
   return 0;
}

/*
=============================

class Editor_Noise_Height_Tool_Options_Menu

this is the option menu for this tool

=============================
*/
class Editor_Noise_Height_Tool_Options_Menu : public Window {
   public:
      Editor_Noise_Height_Tool_Options_Menu(Editor_Interactive*, UniqueWindow*, int*, int*);
      virtual ~Editor_Noise_Height_Tool_Options_Menu();

   private:
      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;
      int* m_lower_value, *m_upper_value;
      Textarea* m_textarea_lower; 
      Textarea* m_textarea_upper; 
      
      void button_clicked(int);
};

/*
===============
Editor_Noise_Height_Tool_Options_Menu::Editor_Noise_Height_Tool_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Noise_Height_Tool_Options_Menu::Editor_Noise_Height_Tool_Options_Menu(Editor_Interactive *parent, UniqueWindow *registry, int* lower_value,
      int* higher_value)
	: Window(parent, (parent->get_w()-500)/2, (parent->get_h()-100)/2, 200, 70, "Option Menu")
{
   m_registry = registry;
   if (m_registry) {
      if (m_registry->window)
         delete m_registry->window;

      m_registry->window = this;
      if (m_registry->x >= 0)
         set_pos(m_registry->x, m_registry->y);
   }
   m_lower_value=lower_value;
   m_upper_value=higher_value;
   m_parent=parent;

   new Textarea(this, 3, 5, "Noise Height Tool Options", Align_Left);
   char buf[250];
   sprintf(buf, "Minimum: %i", *m_lower_value);
   m_textarea_lower=new Textarea(this, 10, 25, buf);
   sprintf(buf, "Maximum: %i", *m_upper_value);
   m_textarea_upper=new Textarea(this, 105, 25, buf);

   Button* b = new Button(this, 30, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Noise_Height_Tool_Options_Menu::button_clicked);
   b=new Button(this, 50, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Noise_Height_Tool_Options_Menu::button_clicked);
   b=new Button(this, 130, 40, 20, 20, 0, 2);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Noise_Height_Tool_Options_Menu::button_clicked);
   b=new Button(this, 150, 40, 20, 20, 0, 3);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Noise_Height_Tool_Options_Menu::button_clicked);
}

/*
===============
Editor_Noise_Height_Tool_Options_Menu::~Editor_Noise_Height_Tool_Options_Menu

Unregister from the registry pointer
===============
*/
Editor_Noise_Height_Tool_Options_Menu::~Editor_Noise_Height_Tool_Options_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}

/*
===========
Editor_Noise_Height_Tool_Options_Menu::button_clicked()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Noise_Height_Tool_Options_Menu::button_clicked(int n) {
   int upper=*m_upper_value;
   int lower=*m_lower_value;
   
   if(n==0) {
      ++lower;
      if(lower>MAX_FIELD_HEIGHT) lower=MAX_FIELD_HEIGHT;
   } else if(n==1) {
      --lower;
      if(lower<0) lower=0;
   } else if(n==2) {
      ++upper;
      if(upper>MAX_FIELD_HEIGHT) upper=MAX_FIELD_HEIGHT;
   } else if(n==3) {
      --upper;
      if(upper<0) upper=0;
   }
   *m_upper_value=upper;
   *m_lower_value=lower;

   char buf[250];
   sprintf(buf, "Minimum: %i", lower);
   m_textarea_lower->set_text(buf);
   sprintf(buf, "Maximum: %i", upper);
   m_textarea_upper->set_text(buf);
}

/*
=============================

class Editor_Noise_Height_Tool

=============================
*/

/*
===========
Editor_Noise_Height_Tool::handle_click()

sets the height of the current to a random value,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Noise_Height_Tool::handle_click(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   Map_Region_Coords mrc(*coordinates, parent->get_fieldsel_radius(), map);
   int mx, my;

   int i, max;
   max=0;
   while(mrc.next(&mx, &my)) { 
      int j=m_lower_value+(int) ((double)(m_upper_value-m_lower_value)*rand()/(RAND_MAX+1.0));
      i=map->set_field_height(Coords(mx,my), j);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}

/*
===========
Editor_Noise_Height_Tool::tool_options_dialog()

Calls the Tool Option dialog
===========
*/
int Editor_Noise_Height_Tool::tool_options_dialog(Editor_Interactive* parent) {
   if (m_w.window)
		delete m_w.window;
	else
		new Editor_Noise_Height_Tool_Options_Menu(parent, &m_w, &m_lower_value, &m_upper_value);
   return 0;
}


/*
=============================

class Editor_Set_Down_Terrain_Tool_Options_Menu

this is the option menu for this tool

=============================
*/
class Editor_Set_Down_Terrain_Tool_Options_Menu : public Window {
   public:
      Editor_Set_Down_Terrain_Tool_Options_Menu(Editor_Interactive*, UniqueWindow*, int*);
      virtual ~Editor_Set_Down_Terrain_Tool_Options_Menu();

   private:
      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;
      int* m_terrain;
      Textarea* m_textarea; 
      
      void button_clicked(int);
};

/*
===============
Editor_Set_Down_Terrain_Tool_Options_Menu::Editor_Set_Down_Terrain_Tool_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Set_Down_Terrain_Tool_Options_Menu::Editor_Set_Down_Terrain_Tool_Options_Menu(Editor_Interactive *parent, UniqueWindow *registry, int* terrain)
	: Window(parent, (parent->get_w()-300)/2, (parent->get_h()-100)/2, 210, 70, "Option Menu")
{
   m_registry = registry;
	if (m_registry) {
		if (m_registry->window)
			delete m_registry->window;
		
		m_registry->window = this;
		if (m_registry->x >= 0)
			set_pos(m_registry->x, m_registry->y);
	}
   m_terrain=terrain;
   m_parent=parent;
   
   new Textarea(this, 3, 5, "Set Down Terrain Tool Options", Align_Left);
   char buf[250];
   sprintf(buf, "Current: %i (%s)", *m_terrain, parent->get_map()->get_world()->get_terrain(*m_terrain)->get_name());
   m_textarea=new Textarea(this, 50, 25, buf);

   Button* b = new Button(this, 85, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Set_Down_Terrain_Tool_Options_Menu::button_clicked);
   b=new Button(this, 105, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Set_Down_Terrain_Tool_Options_Menu::button_clicked);
}

/*
===============
Editor_Set_Down_Terrain_Tool_Options_Menu::~Editor_Set_Down_Terrain_Tool_Options_Menu

Unregister from the registry pointer
===============
*/
Editor_Set_Down_Terrain_Tool_Options_Menu::~Editor_Set_Down_Terrain_Tool_Options_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}

/*
===========
Editor_Set_Down_Terrain_Tool_Options_Menu::button_clicked()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Set_Down_Terrain_Tool_Options_Menu::button_clicked(int n) {
   int val=*m_terrain;
   if(n==0) {
      ++val;
      if(val>=m_parent->get_map()->get_world()->get_nr_terrains()) val=m_parent->get_map()->get_world()->get_nr_terrains()-1;
   } else if(n==1) {
      --val;
      if(val<0) val=0;
   }
   *m_terrain=val;
   
   char buf[250];
   sprintf(buf, "Current: %i (%s)", *m_terrain, m_parent->get_map()->get_world()->get_terrain(*m_terrain)->get_name());
   m_textarea->set_text(buf);
}

/*
=============================

class Editor_Set_Down_Terrain_Tool

=============================
*/

/*
===========
Editor_Set_Down_Terrain_Tool::handle_click()

decrease the height of the current field by one,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Set_Down_Terrain_Tool::handle_click(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   Map_Region_Coords mrc(*coordinates, parent->get_fieldsel_radius(), map);
   int mx, my;
 
   int i, max;
   max=0;
   while(mrc.next(&mx, &my)) { 
      i=map->change_field_terrain(mx,my,m_terrain,true, false);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}

/*
===========
Editor_Set_Down_Terrain_Tool::tool_options_dialog()

Calls the Tool Option dialog
===========
*/
int Editor_Set_Down_Terrain_Tool::tool_options_dialog(Editor_Interactive* parent) {
   if (m_w.window)
      delete m_w.window;
   else
      new Editor_Set_Down_Terrain_Tool_Options_Menu(parent, &m_w, &m_terrain);
   return 0;
}


/*
=============================

class Editor_Set_Right_Terrain_Tool_Options_Menu

this is the option menu for this tool

=============================
*/
class Editor_Set_Right_Terrain_Tool_Options_Menu : public Window {
   public:
      Editor_Set_Right_Terrain_Tool_Options_Menu(Editor_Interactive*, UniqueWindow*, int*);
      virtual ~Editor_Set_Right_Terrain_Tool_Options_Menu();

   private:
      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;
      int* m_terrain;
      Textarea* m_textarea;

      void button_clicked(int);
};

/*
===============
Editor_Set_Right_Terrain_Tool_Options_Menu::Editor_Set_Right_Terrain_Tool_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Set_Right_Terrain_Tool_Options_Menu::Editor_Set_Right_Terrain_Tool_Options_Menu(Editor_Interactive *parent, UniqueWindow *registry, int* terrain)
	: Window(parent, (parent->get_w()-300)/2, (parent->get_h()-100)/2, 210, 70, "Option Menu")
{
   m_registry = registry;
	if (m_registry) {
		if (m_registry->window)
			delete m_registry->window;

		m_registry->window = this;
		if (m_registry->x >= 0)
			set_pos(m_registry->x, m_registry->y);
	}
   m_terrain=terrain;
   m_parent=parent;
   
   new Textarea(this, 2, 5, "Set Right Terrain Tool Options", Align_Left);
   char buf[250];
   sprintf(buf, "Current: %i (%s)", *m_terrain, parent->get_map()->get_world()->get_terrain(*m_terrain)->get_name());
   m_textarea=new Textarea(this, 50, 25, buf);

   Button* b = new Button(this, 85, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Set_Right_Terrain_Tool_Options_Menu::button_clicked);
   b=new Button(this, 105, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Set_Right_Terrain_Tool_Options_Menu::button_clicked);
}

/*
===============
Editor_Set_Right_Terrain_Tool_Options_Menu::~Editor_Set_Right_Terrain_Tool_Options_Menu

Unregister from the registry pointer
===============
*/
Editor_Set_Right_Terrain_Tool_Options_Menu::~Editor_Set_Right_Terrain_Tool_Options_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}

/*
===========
Editor_Set_Right_Terrain_Tool_Options_Menu::button_clicked()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Set_Right_Terrain_Tool_Options_Menu::button_clicked(int n) {
   int val=*m_terrain;
   if(n==0) {
      ++val;
      if(val>=m_parent->get_map()->get_world()->get_nr_terrains()) val=m_parent->get_map()->get_world()->get_nr_terrains()-1;
   } else if(n==1) {
      --val;
      if(val<0) val=0;
   }
   *m_terrain=val;
   
   char buf[250];
   sprintf(buf, "Current: %i (%s)", *m_terrain, m_parent->get_map()->get_world()->get_terrain(*m_terrain)->get_name());
   m_textarea->set_text(buf);
}

/*
=============================

class Editor_Set_Right_Terrain_Tool

=============================
*/

/*
===========
Editor_Set_Right_Terrain_Tool::handle_click()

decrease the height of the current field by one,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Set_Right_Terrain_Tool::handle_click(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   Map_Region_Coords mrc(*coordinates, parent->get_fieldsel_radius(), map);
   int mx, my;
 
   int i, max;
   max=0;
   while(mrc.next(&mx, &my)) {
      i=map->change_field_terrain(mx,my,m_terrain,false,true);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}

/*
===========
Editor_Set_Right_Terrain_Tool::tool_options_dialog()

Calls the Tool Option dialog
===========
*/
int Editor_Set_Right_Terrain_Tool::tool_options_dialog(Editor_Interactive* parent) {
   if (m_w.window)
      delete m_w.window;
   else
      new Editor_Set_Right_Terrain_Tool_Options_Menu(parent, &m_w, &m_terrain);
   return 0;
}

/*
=============================

class Editor_Set_Both_Terrain_Tool_Options_Menu

this is the option menu for this tool

=============================
*/
class Editor_Set_Both_Terrain_Tool_Options_Menu : public Window {
   public:
      Editor_Set_Both_Terrain_Tool_Options_Menu(Editor_Interactive*, UniqueWindow*, int*);
      virtual ~Editor_Set_Both_Terrain_Tool_Options_Menu();

   private:
      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;
      int* m_terrain;
      Textarea* m_textarea;
      Radiogroup* m_radiogroup;

      void selected(int);
};

/*
===============
Editor_Set_Both_Terrain_Tool_Options_Menu::Editor_Set_Both_Terrain_Tool_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Set_Both_Terrain_Tool_Options_Menu::Editor_Set_Both_Terrain_Tool_Options_Menu(Editor_Interactive *parent, UniqueWindow *registry, int* terrain)
	//: Window(parent, (parent->get_w()-300)/2, (parent->get_h()-100)/2, 210, 70, "Option Menu")
	: Window(parent, (parent->get_w()-300)/2, (parent->get_h()-100)/2, 210, 200, "Option Menu")
{
   m_registry = registry;
	if (m_registry) {
		if (m_registry->window)
			delete m_registry->window;

		m_registry->window = this;
		if (m_registry->x >= 0)
			set_pos(m_registry->x, m_registry->y);
	}
   m_terrain=terrain;
   m_parent=parent;

   const int space=5;
   const int xstart=5;
   const int ystart=15;
   const int yend=15;
   int nr_textures=m_parent->get_map()->get_world()->get_nr_terrains();
   int textures_in_row=(int)(sqrt(nr_textures));
   if(textures_in_row*textures_in_row<nr_textures) { textures_in_row++; }
   int i=1;

   m_radiogroup = new Radiogroup();
   set_inner_size((textures_in_row)*(TEXTURE_W+1+space)+xstart, (textures_in_row)*(TEXTURE_H+1+space)+ystart+yend);

   int ypos=ystart;
   int xpos=xstart;
   int cur_x=0;
   while(i<=nr_textures) {
      if(cur_x==textures_in_row) { cur_x=0; ypos+=TEXTURE_H+1+space; xpos=xstart; }

      m_radiogroup->add_button(this, xpos , ypos, get_graphicimpl()->get_maptexture_data(i)->get_texture_picture());

      xpos+=TEXTURE_W+1+space;
      ++cur_x;
      ++i;
   }
   ypos+=TEXTURE_H+1+space+5;

   Textarea* ta=new Textarea(this, 0, 5, "Choose Terrain Menu", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);

   char buf[250];
   sprintf(buf, "Current: %s", parent->get_map()->get_world()->get_terrain(*m_terrain)->get_name());
   m_textarea=new Textarea(this, 5, ypos, buf);
   m_textarea->set_pos((get_inner_w()-m_textarea->get_w())/2, ypos);

   m_radiogroup->changedto.set(this, &Editor_Set_Both_Terrain_Tool_Options_Menu::selected);

   if (m_radiogroup->get_state() < 0)
      m_radiogroup->set_state(*m_terrain);

}

/*
===============
Editor_Set_Both_Terrain_Tool_Options_Menu::~Editor_Set_Both_Terrain_Tool_Options_Menu

Unregister from the registry pointer
===============
*/
Editor_Set_Both_Terrain_Tool_Options_Menu::~Editor_Set_Both_Terrain_Tool_Options_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
   delete m_radiogroup;
}

/*
===========
Editor_Set_Both_Terrain_Tool_Options_Menu::selected()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Set_Both_Terrain_Tool_Options_Menu::selected(int n) {
   *m_terrain=n;

   char buf[250];
   sprintf(buf, "Current: %s", m_parent->get_map()->get_world()->get_terrain(*m_terrain)->get_name());
   m_textarea->set_text(buf);
}

/*
=============================

class Editor_Set_Both_Terrain_Tool

=============================
*/

/*
===========
Editor_Set_Both_Terrain_Tool::handle_click()

decrease the height of the current field by one,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Set_Both_Terrain_Tool::handle_click(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   Map_Region_Coords mrc(*coordinates, parent->get_fieldsel_radius(), map);
   int mx, my;
 
   int i, max;
   max=0;
   while(mrc.next(&mx, &my)) { 
      i=map->change_field_terrain(mx,my,m_terrain,true,true);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}

/*
===========
Editor_Set_Both_Terrain_Tool::tool_options_dialog()

Calls the Tool Option dialog
===========
*/
int Editor_Set_Both_Terrain_Tool::tool_options_dialog(Editor_Interactive* parent) {
   if (m_w.window)
      delete m_w.window;
   else
      new Editor_Set_Both_Terrain_Tool_Options_Menu(parent, &m_w, &m_terrain);
   return 0;
}

