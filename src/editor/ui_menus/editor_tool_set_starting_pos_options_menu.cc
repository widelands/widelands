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

#include "editor_tool_set_starting_pos_options_menu.h"
#include "editor_set_starting_pos_tool.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_editbox.h"
#include "graphic.h"
#include "editorinteractive.h"
#include "error.h"
#include "map.h"
#include "overlay_manager.h"
#include "tribe.h"

/*
=================================================

class Editor_Tool_Set_Starting_Pos_Options_Menu

=================================================
*/


/*
===============
Editor_Tool_Set_Starting_Pos_Options_Menu::Editor_Tool_Set_Starting_Pos_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Tool_Set_Starting_Pos_Options_Menu::Editor_Tool_Set_Starting_Pos_Options_Menu(Editor_Interactive *parent,
				Editor_Set_Starting_Pos_Tool* spt, UIUniqueWindowRegistry* registry)
   : Editor_Tool_Options_Menu(parent, registry, "Player Options")
{
   // Initialisation
   m_spt=spt;
   m_parent=parent;

   // User Interface
   int offsx=5;
   int offsy=30;
   int spacing=5;
   int height=20;
   int width=20;
   int posx=offsx;
   int posy=offsy;

   Tribe_Descr::get_all_tribes(&m_tribes);

   set_inner_size(320, 135);
   UITextarea* ta=new UITextarea(this, 0, 0, "Player Options", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);

   ta=new UITextarea(this, 0, 0, "Number of Players", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, posy+5);
   posy+=spacing+width;
   UIButton* b=new UIButton(this, posx, posy, width, height, 1, 1000);
   b->clickedid.set(this, &Editor_Tool_Set_Starting_Pos_Options_Menu::button_clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b=new UIButton(this, get_inner_w()-spacing-width, posy, width, height, 1, 1001);
   b->clickedid.set(this, &Editor_Tool_Set_Starting_Pos_Options_Menu::button_clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   m_nr_of_players_ta=new UITextarea(this, 0, 0, "5", Align_Left);
   m_nr_of_players_ta->set_pos((get_inner_w()-m_nr_of_players_ta->get_w())/2, posy+5);
   posy+=width+spacing+spacing;

   m_posy=posy;

   int i=0; 
   for(i=0; i<MAX_PLAYERS; i++) {
      m_plr_names[i]=0;
      m_plr_set_pos_buts[i]=0;
      m_plr_set_tribes_buts[i]=0;
   }
   update();
}

/*
===============
Editor_Tool_Set_Starting_Pos_Options_Menu::update()

Update all
===============
*/
void Editor_Tool_Set_Starting_Pos_Options_Menu::update(void) {
   int nr_players=m_parent->get_map()->get_nrplayers();
   std::string text="";
   if(nr_players/10) text+=static_cast<char>(nr_players/10 + 0x30);
   text+=static_cast<char>((nr_players%10) + 0x30);
  
   m_nr_of_players_ta->set_text(text.c_str());

   // Now remove all the unneeded stuff 
   int i=0; 
   for(i=nr_players; i<MAX_PLAYERS; i++) {
      if(m_plr_names[i]) {
         delete m_plr_names[i];
         m_plr_names[i]=0;
      }
      if(m_plr_set_pos_buts[i]) {
         delete m_plr_set_pos_buts[i];
         m_plr_set_pos_buts[i]=0;
      }
      if(m_plr_set_tribes_buts[i]) {
         delete m_plr_set_tribes_buts[i];
         m_plr_set_tribes_buts[i]=0;
      }
   }
   int posy=m_posy;
   int spacing=5;
   int size=20;
   int posx=spacing;

   // And recreate the needed
   for(i=0; i<nr_players; i++) {
      if(!m_plr_names[i]) {
          m_plr_names[i]=new UIEdit_Box(this, posx, posy, 140, size, 0, 750+i);
          m_plr_names[i]->changedid.set(this, &Editor_Tool_Set_Starting_Pos_Options_Menu::button_clicked);
          posx+=140+spacing;
      } 
      m_plr_names[i]->set_text(m_parent->get_map()->get_scenario_player_name(i+1).c_str());
      if(!m_plr_set_tribes_buts[i]) {
         m_plr_set_tribes_buts[i]=new UIButton(this, posx, posy, 140, size, 0, i+500);
         posx+=140+spacing;
         m_plr_set_tribes_buts[i]->clickedid.set(this, &Editor_Tool_Set_Starting_Pos_Options_Menu::button_clicked);
      }
      m_plr_set_tribes_buts[i]->set_title(m_parent->get_map()->get_scenario_player_tribe(i+1).c_str());
      if(!m_plr_set_pos_buts[i]) {
          m_plr_set_pos_buts[i]=new UIButton(this, get_inner_w()-size-spacing, posy, size, size, 0, i+1);
          m_plr_set_pos_buts[i]->clickedid.set(this, &Editor_Tool_Set_Starting_Pos_Options_Menu::button_clicked);
      } 
      text="pics/fsel_editor_set_player_";
      text+=static_cast<char>(((i+1)/10) + 0x30);
      text+=static_cast<char>(((i+1)%10) + 0x30);
      text+="_pos.png"; 
      m_plr_set_pos_buts[i]->set_pic(g_gr->get_picture(PicMod_Game, text.c_str(), RGBColor(0,0,255)));
      posy+=size+spacing;
      posx=spacing;
   } 
   set_inner_size(get_inner_w(),posy+spacing);
}

/*
==============
Editor_Tool_Set_Starting_Pos_Options_Menu::button_clicked()

called when a button is clicked
==============
*/
void Editor_Tool_Set_Starting_Pos_Options_Menu::button_clicked(int n) {
   if(n<500) {
      // player selected
      m_spt->set_current_player(n);
      // jump to the current field
      Coords c=m_parent->get_map()->get_starting_pos(n);
      if(c.x!= -1 && c.y!= -1) 
         m_parent->move_view_to(c.x,c.y);
   
      // Reselect tool, so everything is in a defined state
      m_parent->select_tool(m_parent->get_selected_tool(),0);
     
      // Register callback function to make sure that only valid fields are selected.
      m_parent->get_map()->get_overlay_manager()->register_overlay_callback_function(&Editor_Tool_Set_Starting_Pos_Callback, m_parent->get_map());
      m_parent->get_map()->recalc_whole_map();

   } else if(n<750) {
      // Tribe button has been clicked
      int m=n-500;
      UIButton* but=m_plr_set_tribes_buts[m];
      std::string t= but->get_title();
      if(t=="<undefined>") {
         t=m_tribes[0];
      } else {
         if(!Tribe_Descr::exists_tribe(t)) 
            throw wexception("Map defines tribe %s, but it doesn't exist!\n", t.c_str());
         uint i;
         for(i=0; i<m_tribes.size(); i++) 
            if(m_tribes[i]==t) break;
         if(i==m_tribes.size()-1) t="<undefined>";
         else t=m_tribes[++i];
      }
      m_parent->get_map()->set_scenario_player_tribe(m+1,t); 
   } else if(n<1000) {
      // Player name has been changed
      int m=n-750;   
      std::string text=m_plr_names[m]->get_text();
      if(text=="") {
         text=m_parent->get_map()->get_scenario_player_name(m+1);
         m_plr_names[m]->set_text(text.c_str());
      }
      m_parent->get_map()->set_scenario_player_name(m+1, text);
   } else {
      int nr_players=m_parent->get_map()->get_nrplayers();
      // Up down button
      if(n==1000) ++nr_players; 
      if(n==1001) --nr_players;
      if(nr_players<1) nr_players=1;
      if(nr_players>MAX_PLAYERS) nr_players=MAX_PLAYERS;
      if(nr_players>m_parent->get_map()->get_nrplayers()) {
         // register new default name for this players
         char c1=  (nr_players/10) ? (nr_players/10) + 0x30 : 0;
         char c2= (nr_players%10) + 0x30;
         std::string name="Player ";
         if(c1) name.append(1,c1);
         name.append(1,c2);
         m_parent->get_map()->set_nrplayers(nr_players);
         m_parent->get_map()->set_scenario_player_name(nr_players, name);
         m_parent->get_map()->set_scenario_player_tribe(nr_players, "<undefined>");
      } else {
         std::string name= m_parent->get_map()->get_scenario_player_name(nr_players);
         std::string tribe=  m_parent->get_map()->get_scenario_player_tribe(nr_players);
         m_parent->get_map()->set_nrplayers(nr_players);
         m_parent->get_map()->set_scenario_player_name(nr_players, name); 
         m_parent->get_map()->set_scenario_player_tribe(nr_players, tribe);
      }
   }
   update();
}


