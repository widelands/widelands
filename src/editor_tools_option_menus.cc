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
#include "ui_basic.h"
#include "editor.h"
#include "editorinteractive.h"
#include "editor_tools.h"
#include "editor_tools_option_menus.h"
#include "graphic.h"

/*
=================================================

class Editor_Tool_Options_Menu

=================================================
*/

/*
===========
Editor_Tool_Options_Menu::Editor_Tool_Options_Menu()

constructor
===========
*/
Editor_Tool_Options_Menu::Editor_Tool_Options_Menu(Editor_Interactive* parent,
													UIUniqueWindowRegistry* registry, char* title) :
   UIUniqueWindow(parent, registry, 100, 100, title)
{
   m_parent=parent;

	if (get_usedefaultpos())
		move_to_mouse();
}

/*
===========
Editor_Tool_Options_Menu::~Editor_Tool_Options_Menu()

destructor
===========
*/
Editor_Tool_Options_Menu::~Editor_Tool_Options_Menu()
{
}


/*
=================================================

class Editor_Tool_Change_Height_Options_Menu

=================================================
*/

/*
===========
Editor_Tool_Change_Height_Options_Menu::Editor_Tool_Change_Height_Options_Menu()

constructor
===========
*/
Editor_Tool_Change_Height_Options_Menu::Editor_Tool_Change_Height_Options_Menu(Editor_Interactive* parent, 
      Editor_Increase_Height_Tool* iht, UIUniqueWindowRegistry* registry) :
   Editor_Tool_Options_Menu(parent, registry, "Height Tools Options") {

   m_iht=iht;
   m_dht=iht->get_dht();
   m_sht=iht->get_sht();

   int offsx=5; 
   int offsy=30;
   int spacing=5;
   int height=20;
   int width=20;
   int posx=offsx;
   int posy=offsy;
   
   set_inner_size(135, 135);
   UITextarea* ta=new UITextarea(this, 0, 0, "Height Tool Options", Align_Left); 
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);

   ta=new UITextarea(this, 0, 0, "In/Decrease Value", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, posy+5);
   posy+=spacing+width;
   UIButton* b=new UIButton(this, posx, posy, width, height, 1, 0);
   b->clickedid.set(this, &Editor_Tool_Change_Height_Options_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b=new UIButton(this, get_inner_w()-spacing-width, posy, width, height, 1, 1); 
   b->clickedid.set(this, &Editor_Tool_Change_Height_Options_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   m_increase=new UITextarea(this, 0, 0, "5", Align_Left);
   m_increase->set_pos((get_inner_w()-m_increase->get_w())/2, posy+5);
   posy+=width+spacing+spacing; 
   
   ta=new UITextarea(this, 0, 0, "Set Value", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, posy+5);
   posy+=width+spacing;
   b=new UIButton(this, posx, posy, width, height, 1, 2);
   b->clickedid.set(this, &Editor_Tool_Change_Height_Options_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b=new UIButton(this, get_inner_w()-spacing-width, posy, width, height, 1, 3); 
   b->clickedid.set(this, &Editor_Tool_Change_Height_Options_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   m_set=new UITextarea(this, 0, 0, "5", Align_Left);
   m_set->set_pos((get_inner_w()-m_set->get_w())/2, posy+5);
   posy+=width+spacing; 
   
   update();
}

/*
===========
Editor_Tool_Change_Height_Options_Menu::clicked()

called when one button is clicked
===========
*/
void Editor_Tool_Change_Height_Options_Menu::clicked(int n) {
   int increase=m_iht->get_changed_by();
   int set=m_sht->get_set_to();

   assert(m_iht->get_changed_by()==m_dht->get_changed_by());

   if(n==0) {
      ++increase;
      if(increase>MAX_FIELD_HEIGHT_DIFF) increase=MAX_FIELD_HEIGHT_DIFF;
   } else if(n==1) {
      --increase;
      if(increase<0) increase=0;
   }
   m_iht->set_changed_by(increase);
   m_dht->set_changed_by(increase);

   if(n==2) {
      ++set;
      if(set>MAX_FIELD_HEIGHT) set=MAX_FIELD_HEIGHT;
   } else if(n==3) {
      --set;
      if(set<0) set=0;
   }
   m_sht->set_set_to(set);

   update();
}

/*
===========
Editor_Tool_Change_Height_Options_Menu::update()

Update all the textareas, so that they represent the correct values
===========
*/
void Editor_Tool_Change_Height_Options_Menu::update(void) {
   char buf[250];
   sprintf(buf, "%i", m_iht->get_changed_by());
   m_increase->set_text(buf);
   sprintf(buf, "%i", m_sht->get_set_to());
   m_set->set_text(buf);
}

/*
=================================================

class Editor_Tool_Noise_Height_Options_Menu

=================================================
*/

/*
===============
Editor_Tool_Noise_Height_Options_Menu::Editor_Tool_Noise_Height_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Tool_Noise_Height_Options_Menu::Editor_Tool_Noise_Height_Options_Menu(Editor_Interactive *parent,
				Editor_Noise_Height_Tool* nht, UIUniqueWindowRegistry* registry)
   : Editor_Tool_Options_Menu(parent, registry, "Noise Height Options")
{
   char buf[250];
   sprintf(buf, "Minimum: %i", 10);
   m_textarea_lower=new UITextarea(this, 10, 25, buf);
   sprintf(buf, "Maximum: %i", 10);
   m_textarea_upper=new UITextarea(this, 105, 25, buf);

   UIButton* b = new UIButton(this, 30, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);
   b=new UIButton(this, 50, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);
   b=new UIButton(this, 130, 40, 20, 20, 0, 2);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);
   b=new UIButton(this, 150, 40, 20, 20, 0, 3);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);

   set_inner_size(200, 115);
   
   UITextarea* ta=new UITextarea(this, 3, 5, "Noise Height Tool Options", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);

   int posy=70;
   int width=20;
   int spacing=5;
   int height=20;
   ta=new UITextarea(this, 0, 0, "Set Value", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, posy+5);
   posy+=20;
   
   m_set=new UITextarea(this, 0, 0, "99", Align_Left);
   m_set->set_pos((get_inner_w()-m_set->get_w())/2, posy+5);
   b=new UIButton(this, m_set->get_x()-width-spacing, posy, width, height, 1, 4);
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b=new UIButton(this, m_set->get_x()+m_set->get_w()+spacing, posy, width, height, 1, 5);
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", RGBColor(0,0,255)));
          
   m_nht=nht;
   
   update();
}

/*
===============
Editor_Tool_Noise_Height_Options_Menu::update()

Update all textareas
===============
*/
void Editor_Tool_Noise_Height_Options_Menu::update(void) {
   char buf[200];
   int up, low;
   m_nht->get_values(&low, &up);
   sprintf(buf, "Minimum: %i", low);
   m_textarea_lower->set_text(buf);
   sprintf(buf, "Maximum: %i", up);
   m_textarea_upper->set_text(buf);

   sprintf(buf, "%i", m_nht->get_sht()->get_set_to());
   m_set->set_text(buf);

}

/*
==============
Editor_Tool_Noise_Height_Options_Menu::button_clicked()

called when a button is clicked
==============
*/
void Editor_Tool_Noise_Height_Options_Menu::button_clicked(int n) {
   int up, low, set;
   set=m_nht->get_sht()->get_set_to();
   m_nht->get_values(&low, &up);
   switch(n) {
      case 0: ++low; break;
      case 1: --low; break;
      case 2: ++up; break;
      case 3: --up; break;
      case 4: ++set; break;
      case 5: --set; break;
   } 
   if(low>MAX_FIELD_HEIGHT) low=MAX_FIELD_HEIGHT;
   if(low<0) low=0;
   if(up>MAX_FIELD_HEIGHT) up=MAX_FIELD_HEIGHT;
   if(up<0) up=0;
   if(set>MAX_FIELD_HEIGHT) set=MAX_FIELD_HEIGHT;
   if(set<0) set=0;

   m_nht->set_values(low, up);
   m_nht->get_sht()->set_set_to(set);

   update();
}

/*
=================================================

class Editor_Tool_Set_Terrain_Tool_Options_Menu

=================================================
*/

/*
===============
Editor_Tool_Set_Terrain_Tool_Options_Menu::Editor_Tool_Set_Terrain_Tool_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Tool_Set_Terrain_Tool_Options_Menu::Editor_Tool_Set_Terrain_Tool_Options_Menu(Editor_Interactive *parent,
						Editor_Set_Both_Terrain_Tool* sbt, UIUniqueWindowRegistry *registry)
	: Editor_Tool_Options_Menu(parent, registry, "Terrain Select")
{
   m_sbt=sbt;

   const int space=5;
   const int xstart=5;
   const int ystart=15;
   const int yend=15;
   int nr_textures=get_parent()->get_map()->get_world()->get_nr_terrains();
   int textures_in_row=(int)(sqrt(nr_textures));
   if(textures_in_row*textures_in_row<nr_textures) { textures_in_row++; }
   int i=1;

   set_inner_size((textures_in_row)*(TEXTURE_W+1+space)+xstart, (textures_in_row)*(TEXTURE_H+1+space)+ystart+yend);

   int ypos=ystart;
   int xpos=xstart;
   int cur_x=0;
   while(i<=nr_textures) {
      if(cur_x==textures_in_row) { cur_x=0; ypos+=TEXTURE_H+1+space; xpos=xstart; }

      UICheckbox* cb=new UICheckbox(this, xpos , ypos, g_gr->get_picture(PicMod_Game, g_gr->get_maptexture_picture(i)));

      cb->set_size(TEXTURE_W+1, TEXTURE_H+1);
      cb->set_id(i-1);
      cb->set_state(m_sbt->is_enabled(i-1));
      cb->changedtoid.set(this, &Editor_Tool_Set_Terrain_Tool_Options_Menu::selected);

      xpos+=TEXTURE_W+1+space;
      ++cur_x;
      ++i;
   }
   ypos+=TEXTURE_H+1+space+5;

   UITextarea* ta=new UITextarea(this, 0, 5, "Choose Terrain Menu", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);


   std::string buf="Current: ";
   int j=m_sbt->get_nr_enabled();
   for(int i=0; j; i++) {
      if(m_sbt->is_enabled(i)) {
         buf+=get_parent()->get_map()->get_world()->get_terrain(i)->get_name();
         buf+=" ";
         --j;
      }
   }
   
   m_textarea=new UITextarea(this, 5, ypos, buf);
   m_textarea->set_pos((get_inner_w()-m_textarea->get_w())/2, ypos);
}

/*
===========
Editor_Tool_Set_Terrain_Tool_Options_Menu::selected()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Tool_Set_Terrain_Tool_Options_Menu::selected(int n, bool t) {
   m_sbt->enable(n,t);

   std::string buf="Current: ";
   int j=m_sbt->get_nr_enabled();
   for(int i=0; j; i++) {
      if(m_sbt->is_enabled(i)) {
         buf+=get_parent()->get_map()->get_world()->get_terrain(i)->get_name();
         buf+=" ";
         --j;
      }
   }
   
   m_textarea->set_text(buf.c_str());
   m_textarea->set_pos((get_inner_w()-m_textarea->get_w())/2, m_textarea->get_y());
}

/*
=================================================

class Editor_Tool_Place_Immovable_Options_Menu

=================================================
*/

/*
===========
Editor_Tool_Place_Immovable_Options_Menu::Editor_Tool_Place_Immovable_Options_Menu

constructor
===========
*/
Editor_Tool_Place_Immovable_Options_Menu::Editor_Tool_Place_Immovable_Options_Menu(Editor_Interactive* parent,
		Editor_Place_Immovable_Tool* pit, UIUniqueWindowRegistry* registry) :
   Editor_Tool_Options_Menu(parent, registry, "Immovable Bobs Menu") {
   const int max_items_in_tab=6;
   
   m_pit=pit;
   
   const int space=5;
   const int xstart=5;
   const int ystart=15;
   const int yend=15;
   int nr_immovables=get_parent()->get_map()->get_world()->get_nr_immovables();
   int immovables_in_row=(int)(sqrt(nr_immovables));
   if(immovables_in_row*immovables_in_row<nr_immovables) { immovables_in_row++; }
   if(immovables_in_row>max_items_in_tab) immovables_in_row=max_items_in_tab;


   UITab_Panel* m_tabpanel=new UITab_Panel(this, 0, 0, 1);
   m_tabpanel->set_snapparent(true);
   UIBox* box=new UIBox(m_tabpanel, 0, 0, UIBox::Horizontal);
   m_tabpanel->add(g_gr->get_picture(PicMod_Game, "pics/menu_tab_buildbig.png" , RGBColor(0,0,255)), box);


   int width=0;
   int height=0;
   for(int j=0; j<nr_immovables; j++) {
      int w,h;
		Immovable_Descr* descr = get_parent()->get_map()->get_world()->get_immovable_descr(j);
      g_gr->get_picture_size(
            g_gr->get_picture(PicMod_Game, descr->get_picture(),
                              descr->get_default_encodedata().clrkey), &w, &h);
      if(w>width) width=w;
      if(h>height) height=h;
   }

   box->set_inner_size((immovables_in_row)*(width+1+space)+xstart, (immovables_in_row)*(height+1+space)+ystart+yend);

   int ypos=ystart;
   int xpos=xstart;
   int cur_x=0;
   int i=0;
   while(i<nr_immovables) {
      if(cur_x==immovables_in_row) {
         cur_x=0;
         ypos=ystart;
         xpos=xstart;
         box->resize();
         box=new UIBox(m_tabpanel, 0, 0, UIBox::Horizontal);
         m_tabpanel->add(g_gr->get_picture(PicMod_Game, "pics/menu_tab_buildbig.png" , RGBColor(0,0,255)), box);
      }

		Immovable_Descr* descr = get_parent()->get_map()->get_world()->get_immovable_descr(i);
      UICheckbox* cb= new UICheckbox(box, xpos, ypos,
            g_gr->get_picture(PicMod_Game, descr->get_picture(), descr->get_default_encodedata().clrkey));

      cb->set_size(width, height);
      cb->set_id(i);
      cb->set_state(m_pit->is_enabled(i));
      cb->changedtoid.set(this, &Editor_Tool_Place_Immovable_Options_Menu::clicked);
      box->add(cb, Align_Left);
      box->add_space(space);
      xpos+=width+1+space;
      ++cur_x;
      ++i;
   }
   ypos+=height+1+space+5;

   m_tabpanel->activate(0);
   box->resize();
   m_tabpanel->resize();
}

/*
   ===========
   void Editor_Tool_Place_Immovable_Options_Menu::clicked()

this is called when one of the state boxes is toggled
===========
*/
void Editor_Tool_Place_Immovable_Options_Menu::clicked(int n, bool t) {
   m_pit->enable(n,t);
}
