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

#include "editor.h"
#include "editorinteractive.h"
#include "editor_make_infrastructure_tool.h"
#include "editor_player_menu.h"
#include "editor_player_menu_allowed_buildings_menu.h"
#include "editor_set_starting_pos_tool.h"
#include "error.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"
#include "overlay_manager.h"
#include "player.h"
#include "tribe.h"
#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_modal_messagebox.h"
#include "ui_textarea.h"
#include "wexception.h"

/*
=================================================

class Editor_Tool_Set_Starting_Pos_Options_Menu

=================================================
*/


/*
===============
Editor_Player_Menu::Editor_Player_Menu

Create all the buttons etc...
===============
*/
Editor_Player_Menu::Editor_Player_Menu(Editor_Interactive *parent,
      Editor_Interactive::Editor_Tools* tools, int spt_tool_index, int make_infrs_tindex, UIUniqueWindowRegistry* registry)
   : UIUniqueWindow(parent, registry, 340, 400, _("Player Options"))
{
   // Initialisation
   m_parent=parent;
   m_tools=tools;
   m_spt_index=spt_tool_index;
   m_mis_index=make_infrs_tindex;

   // User Interface
   int offsx=5;
   int offsy=30;
   int spacing=5;
   int height=20;
   int width=20;
   int posx=offsx;
   int posy=offsy;

   Tribe_Descr::get_all_tribes(&m_tribes);

   set_inner_size(375, 135);
   UITextarea* ta=new UITextarea(this, 0, 0, _("Player Options"), Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);

   ta=new UITextarea(this, 0, 0, _("Number of Players"), Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, posy+5);
   posy+=spacing+width;
   UIButton* b=new UIButton(this, posx, posy, width, height, 1, 0);
   b->clickedid.set(this, &Editor_Player_Menu::button_clicked);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_up.png" ));
   b=new UIButton(this, get_inner_w()-spacing-width, posy, width, height, 1, 1);
   b->clickedid.set(this, &Editor_Player_Menu::button_clicked);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_down.png" ));
   m_nr_of_players_ta=new UITextarea(this, 0, 0, "5", Align_Left);
   m_nr_of_players_ta->set_pos((get_inner_w()-m_nr_of_players_ta->get_w())/2, posy+5);
   posy+=width+spacing+spacing;

   m_posy=posy;

   int i=0;
   for(i=0; i<MAX_PLAYERS; i++) {
      m_plr_names[i]=0;
      m_plr_set_pos_buts[i]=0;
      m_plr_set_tribes_buts[i]=0;
      m_plr_make_infrastructure_buts[i]=0;
      m_plr_allowed_buildings[i]=0;
   }
   update();

   set_think(true);
}

/*
 * Think function. Some things may change while this window
 * is open
 */
void Editor_Player_Menu::think(void) {
   update();
}

/*
===============
Editor_Player_Menu::update()

Update all
===============
*/
void Editor_Player_Menu::update(void) {
   if(is_minimized()) return;

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
      if(m_plr_set_tribes_buts[i]) {
         delete m_plr_make_infrastructure_buts[i];
         m_plr_make_infrastructure_buts[i]=0;
      }
      if(m_plr_allowed_buildings[i]) {
         delete m_plr_allowed_buildings[i];
         m_plr_allowed_buildings[i]=0;
      }
   }
   int posy=m_posy;
   int spacing=5;
   int size=20;
   int posx=spacing;


   // And recreate the needed
   for(i=0; i<nr_players; i++) {
      // Check if starting position is valid
      bool start_pos_valid=true;
      Coords start_pos=m_parent->get_map()->get_starting_pos(i+1);
      if(start_pos.x==-1 && start_pos.y==-1) start_pos_valid=false;

      if(!m_plr_names[i]) {
          m_plr_names[i]=new UIEdit_Box(this, posx, posy, 140, size, 0, i);
          m_plr_names[i]->changedid.set(this, &Editor_Player_Menu::name_changed);
          posx+=140+spacing;
          m_plr_names[i]->set_text(m_parent->get_map()->get_scenario_player_name(i+1).c_str());
      }

      if(!m_plr_set_tribes_buts[i]) {
         m_plr_set_tribes_buts[i]=new UIButton(this, posx, posy, 140, size, 0, i);
         posx+=140+spacing;
         m_plr_set_tribes_buts[i]->clickedid.set(this, &Editor_Player_Menu::player_tribe_clicked);
      }
      if(m_parent->get_map()->get_scenario_player_tribe(i+1)!="<undefined>")
         m_plr_set_tribes_buts[i]->set_title(m_parent->get_map()->get_scenario_player_tribe(i+1).c_str());
      else {
         m_plr_set_tribes_buts[i]->set_title(m_tribes[0].c_str());
         m_parent->get_map()->set_scenario_player_tribe(i+1,m_tribes[0]);
      }

      // Set Starting pos button
      if(!m_plr_set_pos_buts[i]) {
          m_plr_set_pos_buts[i]=new UIButton(this, posx, posy, size, size, 0, i+1);
          m_plr_set_pos_buts[i]->clickedid.set(this, &Editor_Player_Menu::set_starting_pos_clicked);
          posx+=size+spacing;
      }
      text="pics/fsel_editor_set_player_";
      text+=static_cast<char>(((i+1)/10) + 0x30);
      text+=static_cast<char>(((i+1)%10) + 0x30);
      text+="_pos.png";
      m_plr_set_pos_buts[i]->set_pic(g_gr->get_picture( PicMod_Game,  text.c_str() ));
      // Build infrastructure but
      if(!m_plr_make_infrastructure_buts[i]) {
         m_plr_make_infrastructure_buts[i]=new UIButton(this, posx, posy, size, size, 0, i+1);
         m_plr_make_infrastructure_buts[i]->clickedid.set(this, &Editor_Player_Menu::make_infrastructure_clicked);
         posx+=size+spacing;
      }
      m_plr_make_infrastructure_buts[i]->set_enabled(0); // TODO enable this again: ->set_enabled(start_pos_valid)
      m_plr_make_infrastructure_buts[i]->set_title("I"); // TODO: come up with a picture for this
      // Allowed buildings
      if(!m_plr_allowed_buildings[i]) {
         m_plr_allowed_buildings[i]=new UIButton(this, posx, posy, size, size, 0, i+1);
         m_plr_allowed_buildings[i]->clickedid.set(this, &Editor_Player_Menu::allowed_buildings_clicked);
         posx+=size+spacing;
      }
      m_plr_allowed_buildings[i]->set_enabled(0); // TODO enable this again: ->set_enabled(start_pos_valid)
      m_plr_allowed_buildings[i]->set_title("B"); // TODO: come up with a picture for this

      posx=spacing;
      posy+=size+spacing;
   }
   set_inner_size(get_inner_w(),posy+spacing);
}

/*
==============
Editor_Player_Menu::button_clicked()

called when a button is clicked
==============
*/
void Editor_Player_Menu::button_clicked(int n) {
   int nr_players=m_parent->get_map()->get_nrplayers();
   // Up down button
   if(n==0) ++nr_players;
   if(n==1) --nr_players;
   if(nr_players<1) nr_players=1;
   if(nr_players>MAX_PLAYERS) nr_players=MAX_PLAYERS;
   if(nr_players>m_parent->get_map()->get_nrplayers()) {
      // register new default name for this players
      char c1=  (nr_players/10) ? (nr_players/10) + 0x30 : 0;
      char c2= (nr_players%10) + 0x30;
      std::string name=_("Player ");
      if(c1) name.append(1,c1);
      name.append(1,c2);
      m_parent->get_map()->set_nrplayers(nr_players);
      m_parent->get_map()->set_scenario_player_name(nr_players, name);
      m_parent->get_map()->set_scenario_player_tribe(nr_players, m_tribes[0]);
      m_parent->set_need_save(true);
   } else {
      if(!m_parent->is_player_tribe_referenced(nr_players)) {
         std::string name= m_parent->get_map()->get_scenario_player_name(nr_players);
         std::string tribe=  m_parent->get_map()->get_scenario_player_tribe(nr_players);
         m_parent->get_map()->set_nrplayers(nr_players);
         m_parent->get_map()->set_scenario_player_name(nr_players, name);
         m_parent->get_map()->set_scenario_player_tribe(nr_players, tribe);
         m_parent->set_need_save(true);
      } else {
         UIModal_Message_Box* mmb=new UIModal_Message_Box(m_parent, _("Error!"), _("Can't remove player. It is referenced in some place. Remove all buildings, bobs, triggers and events that depend of this player and try again"), UIModal_Message_Box::OK);
         mmb->run();
         delete mmb;
      }
   }
   update();
}


/*
 * Player Tribe Button clicked
 */
void Editor_Player_Menu::player_tribe_clicked(int n) {
   // Tribe button has been clicked
   if(!m_parent->is_player_tribe_referenced(n+1)) {
      UIButton* but=m_plr_set_tribes_buts[n];
      std::string t= but->get_title();
      if(!Tribe_Descr::exists_tribe(t))
         throw wexception("Map defines tribe %s, but it doesn't exist!\n", t.c_str());
      uint i;
      for(i=0; i<m_tribes.size(); i++)
         if(m_tribes[i]==t) break;
      if(i==m_tribes.size()-1) t=m_tribes[0];
      else t=m_tribes[++i];
      m_parent->get_map()->set_scenario_player_tribe(n+1,t);
      m_parent->set_need_save(true);
   } else {
      UIModal_Message_Box* mmb=new UIModal_Message_Box(m_parent, _("Error!"), _("Can't change player tribe. It is referenced in some place. Remove all buildings, bobs, triggers and events that depend on this tribe and try again"), UIModal_Message_Box::OK);
      mmb->run();
      delete mmb;
   }
   update();
}


/*
 * Set Current Start Position button selected
 */
void Editor_Player_Menu::set_starting_pos_clicked(int n) {
   // jump to the current field
   Coords c=m_parent->get_map()->get_starting_pos(n);
   if(c.x!= -1 && c.y!= -1)
      m_parent->move_view_to(c.x,c.y);

   // If the player is already created in the editor, this means
   // that there might be already a hq placed somewhere. This needs to be
   // deleted before a starting position change can occure
   if(m_parent->get_editor()->get_player(n)) {
      if(m_parent->get_map()->get_starting_pos(n) != Coords(-1,-1)) {
         BaseImmovable* imm = m_parent->get_map()->get_field(m_parent->get_map()->get_starting_pos(n))->get_immovable();
         if(imm && imm->get_type() == Map_Object::BUILDING) return;
      }
   }

   // Select tool set mplayer
   m_parent->select_tool(m_spt_index,0);
   static_cast<Editor_Set_Starting_Pos_Tool*>(m_tools->tools[m_spt_index])->set_current_player(n);

   // Reselect tool, so everything is in a defined state
   m_parent->select_tool(m_parent->get_selected_tool(),0);

   // Register callback function to make sure that only valid fields are selected.
   m_parent->get_map()->get_overlay_manager()->register_overlay_callback_function(&Editor_Tool_Set_Starting_Pos_Callback, m_parent->get_map());
   m_parent->get_map()->recalc_whole_map();
   update();
}

/*
 * Player name has changed
 */
void Editor_Player_Menu::name_changed(int m) {
   // Player name has been changed
   std::string text=m_plr_names[m]->get_text();
   if(text=="") {
      text=m_parent->get_map()->get_scenario_player_name(m+1);
      m_plr_names[m]->set_text(text.c_str());
   }
   m_parent->get_map()->set_scenario_player_name(m+1, text);
   m_plr_names[m]->set_text(m_parent->get_map()->get_scenario_player_name(m+1).c_str());
   m_parent->set_need_save(true);
}

/*
 * Make infrastructure button clicked
 */
void Editor_Player_Menu::make_infrastructure_clicked(int n) {
   // Check if starting position is valid (was checked before
   // so must be true)
   Coords start_pos=m_parent->get_map()->get_starting_pos(n);
   assert(start_pos.x!=-1 && start_pos.y!=-1);

   Editor* editor=m_parent->get_editor();

   Player* p=editor->get_player(n);
   if(!p) {
      // This player is unknown, register it, place a hq and reference the tribe
      // so that this tribe can not be changed
      editor->add_player(n, Player::playerLocal, m_plr_set_tribes_buts[n-1]->get_title(), m_plr_names[n-1]->get_text());

      p=editor->get_player(n);
      p->init(m_parent->get_egbase(),false);
   }

   // If the player is already created in the editor, this means
   // that there might be already a hq placed somewhere. This needs to be
   // deleted before a starting position change can occure
   BaseImmovable* imm = m_parent->get_map()->get_field(m_parent->get_map()->get_starting_pos(p->get_player_number()))->get_immovable();
   if(!imm) {
      // place HQ
      const Coords &c = m_parent->get_map()->get_starting_pos(p->get_player_number());
      int idx = p->get_tribe()->get_building_index("headquarters");
      if (idx < 0)
         throw wexception("Tribe %s lacks headquarters", p->get_tribe()->get_name());
      m_parent->get_egbase()->warp_building(c, p->get_player_number(), idx);

      m_parent->reference_player_tribe(n, p->get_tribe());

      // Remove the player overlay from this starting pos.
      // A HQ is overlay enough
      std::string picsname="pics/editor_player_";
      picsname+=static_cast<char>((n/10) + 0x30);
      picsname+=static_cast<char>((n%10) + 0x30);
      picsname+="_starting_pos.png";
      int picid=g_gr->get_picture( PicMod_Game,  picsname.c_str() );
      // Remove old overlay if any
      m_parent->get_editor()->get_map()->get_overlay_manager()->remove_overlay(start_pos,picid);
   }

   m_parent->select_tool(m_mis_index,0);
   static_cast<Editor_Make_Infrastructure_Tool*>(m_tools->tools[m_mis_index])->set_player(n);
   m_parent->get_editor()->get_map()->get_overlay_manager()->register_overlay_callback_function(&Editor_Make_Infrastructure_Tool_Callback, static_cast<void*>(m_parent->get_editor()),n);
   m_parent->get_editor()->get_map()->recalc_whole_map();
}

/*
 * Allowed building button clicked
 */
void Editor_Player_Menu::allowed_buildings_clicked(int n) {

   Editor* editor=m_parent->get_editor();

   if(!editor->get_player(n)) {
      // The player is not yet really on the map, call make infrastructure button first
      make_infrastructure_clicked(n);
   }

   // Create the menu
   if (m_allow_buildings_menu.window) {
      delete m_allow_buildings_menu.window;
   }
   else {
      new Editor_Player_Menu_Allowed_Buildings_Menu(m_parent, editor->get_player(n), &m_allow_buildings_menu);
   }
}
