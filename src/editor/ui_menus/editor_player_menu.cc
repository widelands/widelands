/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "editor_player_menu.h"

#include "editorinteractive.h"
#include "editor_set_starting_pos_tool.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"
#include "overlay_manager.h"
#include "player.h"
#include <stdint.h>
#include "tribe.h"
#include "warehouse.h"
#include "wexception.h"

#include "ui_editbox.h"
#include "ui_modal_messagebox.h"
#include "ui_textarea.h"


Editor_Player_Menu::Editor_Player_Menu
(Editor_Interactive & parent, UI::UniqueWindow::Registry * registry)
:
UI::UniqueWindow(&parent, registry, 340, 400, _("Player Options")),

m_add_player
(this,
 5, 5, 20, 20,
 1,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
 &Editor_Player_Menu::clicked_add_player, this,
 _("Add player"),
 parent.egbase().map().get_nrplayers() < MAX_PLAYERS),

m_remove_last_player
(this,
 get_inner_w() - 5 - 20, 5, 20, 20,
 1,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
 &Editor_Player_Menu::clicked_remove_last_player, this,
 _("Remove last player"),
 1 < parent.egbase().map().get_nrplayers())

{
   // User Interface
   int32_t spacing=5;
   int32_t width=20;
   int32_t posy= 0;

	Tribe_Descr::get_all_tribenames(m_tribes);

   set_inner_size(375, 135);

	UI::Textarea * ta =
		new UI::Textarea(this, 0, 0, _("Number of Players"), Align_Left);
	ta->set_pos(Point((get_inner_w() - ta->get_w()) / 2, posy + 5));
   posy+=spacing+width;

   m_nr_of_players_ta=new UI::Textarea(this, 0, 0, "5", Align_Left);
   m_nr_of_players_ta->set_pos
		(Point((get_inner_w() - m_nr_of_players_ta->get_w()) / 2, posy + 5));
   posy+=width+spacing+spacing;

   m_posy=posy;

   int32_t i=0;
   for (i=0; i<MAX_PLAYERS; i++) {
      m_plr_names[i]=0;
      m_plr_set_pos_buts[i]=0;
      m_plr_set_tribes_buts[i]=0;
//       m_plr_make_infrastructure_buts[i]=0;
//       m_plr_allowed_buildings[i]=0;
	}
   update();

   set_think(true);
}

/*
 * Think function. Some things may change while this window
 * is open
 */
void Editor_Player_Menu::think() {
   update();
}

/*
===============
Editor_Player_Menu::update()

Update all
===============
*/
void Editor_Player_Menu::update() {
	if (is_minimal()) return;

	Map & map =
		dynamic_cast<const Editor_Interactive &>(*get_parent()).egbase().map();
	const Player_Number nr_players = map.get_nrplayers();
   std::string text="";
   if (nr_players/10) text+=static_cast<char>(nr_players/10 + 0x30);
   text+=static_cast<char>((nr_players%10) + 0x30);

   m_nr_of_players_ta->set_text(text.c_str());

   // Now remove all the unneeded stuff
   int32_t i=0;
   for (i=nr_players; i<MAX_PLAYERS; i++) {
         delete m_plr_names[i];
         m_plr_names[i]=0;
         delete m_plr_set_pos_buts[i];
         m_plr_set_pos_buts[i]=0;
         delete m_plr_set_tribes_buts[i];
         m_plr_set_tribes_buts[i]=0;
#if 0
         delete m_plr_make_infrastructure_buts[i];
         m_plr_make_infrastructure_buts[i]=0;
         delete m_plr_allowed_buildings[i];
         m_plr_allowed_buildings[i]=0;
#endif
	}
   int32_t posy=m_posy;
   int32_t spacing=5;
   int32_t size=20;


   // And recreate the needed
   for (i=0; i<nr_players; i++) {
		int32_t posx = spacing;
      if (!m_plr_names[i]) {
          m_plr_names[i]=new UI::Edit_Box(this, posx, posy, 140, size, 0, i);
          m_plr_names[i]->changedid.set(this, &Editor_Player_Menu::name_changed);
          posx+=140+spacing;
			m_plr_names[i]->set_text(map.get_scenario_player_name(i + 1).c_str());
		}

      if (!m_plr_set_tribes_buts[i]) {
			m_plr_set_tribes_buts[i] =
				new UI::IDButton<Editor_Player_Menu, const Player_Number>
				(this,
				 posx, posy, 140, size,
				 0,
				 &Editor_Player_Menu::player_tribe_clicked, this, i,
				 std::string());
         posx+=140+spacing;
		}
		if (map.get_scenario_player_tribe(i + 1) != "<undefined>")
			m_plr_set_tribes_buts[i]->set_title
				(map.get_scenario_player_tribe(i + 1).c_str());
	   else {
         m_plr_set_tribes_buts[i]->set_title(m_tribes[0].c_str());
			map.set_scenario_player_tribe(i + 1, m_tribes[0]);
		}

      // Set Starting pos button
      if (!m_plr_set_pos_buts[i]) {
			m_plr_set_pos_buts[i] =
				new UI::IDButton<Editor_Player_Menu, const Player_Number>
				(this,
				 posx, posy, size, size,
				 0,
				 0, //  set below
				 &Editor_Player_Menu::set_starting_pos_clicked, this, i + 1,
				 std::string());
          posx+=size+spacing;
		}
      text="pics/fsel_editor_set_player_";
      text+=static_cast<char>(((i+1)/10) + 0x30);
      text+=static_cast<char>(((i+1)%10) + 0x30);
      text+="_pos.png";
      m_plr_set_pos_buts[i]->set_pic(g_gr->get_picture(PicMod_Game,  text.c_str()));
      posy+=size+spacing;
	}
   set_inner_size(get_inner_w(), posy+spacing);
}

void Editor_Player_Menu::clicked_add_player() {
	Editor_Interactive & parent =
		dynamic_cast<Editor_Interactive &>(*get_parent());
	Map & map = parent.egbase().map();
	const Player_Number nr_players = map.get_nrplayers() + 1;
	assert(nr_players <= MAX_PLAYERS);
      // register new default name for this players
      char c1=  (nr_players/10) ? (nr_players/10) + 0x30 : 0;
      char c2= (nr_players%10) + 0x30;
      std::string name=_("Player ");
      if (c1) name.append(1, c1);
      name.append(1, c2);
		map.set_nrplayers(nr_players);
		map.set_scenario_player_name(nr_players, name);
		map.set_scenario_player_tribe(nr_players, m_tribes[0]);
		parent.set_need_save(true);
	m_add_player        .set_enabled(nr_players < MAX_PLAYERS);
	m_remove_last_player.set_enabled(true);
	update();
}


void Editor_Player_Menu::clicked_remove_last_player() {
	Editor_Interactive & parent =
		dynamic_cast<Editor_Interactive &>(*get_parent());
	Map & map = parent.egbase().map();
	const Player_Number old_nr_players = map.get_nrplayers();
	const Player_Number nr_players = old_nr_players - 1;
	assert(1 <= nr_players);
	if (not parent.is_player_tribe_referenced(nr_players)) {
		if (const Coords sp = map.get_starting_pos(old_nr_players)) {
			//  Remove starting position marker.
			char picsname[] = "pics/editor_player_??_starting_pos.png";
			picsname[19] = static_cast<char>(old_nr_players / 10 + 0x30);
			picsname[20] = static_cast<char>(old_nr_players % 10 + 0x30);
			map.overlay_manager().remove_overlay
				(sp, g_gr->get_picture(PicMod_Game, picsname));
		}
			std::string name  = map.get_scenario_player_name (nr_players);
			std::string tribe = map.get_scenario_player_tribe(nr_players);
			map.set_nrplayers(nr_players);
			map.set_scenario_player_name(nr_players, name);
			map.set_scenario_player_tribe(nr_players, tribe);
			parent.set_need_save(true);
		m_add_player        .set_enabled(true);
		m_remove_last_player.set_enabled(1 < nr_players);
		if
			(&parent.tools.current() == &parent.tools.set_starting_pos
			 and
			 parent.tools.set_starting_pos.get_current_player() == old_nr_players)
			//  The starting position tool is the currently active editor tool and
			//  the sel picture is the one with the color of
			//  the player that is being removed. Make sure that it is fixed in
			//  that case by by switching the tool to the previous player and
			//  reselecting the tool.
			set_starting_pos_clicked(nr_players); //  This calls update().
		else update();
		} else {
			UI::Modal_Message_Box mmb
				(&parent,
				 _("Error!"),
				 _("Can't remove player. It is referenced in some place. Remove "
				   "all buildings, bobs, triggers and events that depend on this "
				   "player and try again"),
				 UI::Modal_Message_Box::OK);
			mmb.run();
		}
}


/*
 * Player Tribe Button clicked
 */
void Editor_Player_Menu::player_tribe_clicked(const Uint8 n) {
	Editor_Interactive & parent =
		dynamic_cast<Editor_Interactive &>(*get_parent());
   // Tribe button has been clicked
	if (not parent.is_player_tribe_referenced(n + 1)) {
      std::string t = m_plr_set_tribes_buts[n]->get_title();
      if (!Tribe_Descr::exists_tribe(t))
         throw wexception("Map defines tribe %s, but it doesn't exist!\n", t.c_str());
      uint32_t i;
      for (i=0; i<m_tribes.size(); i++)
         if (m_tribes[i]==t) break;
      if (i==m_tribes.size()-1) t=m_tribes[0];
      else t=m_tribes[++i];
		parent.egbase().map().set_scenario_player_tribe(n+1, t);
		parent.set_need_save(true);
	} else {
		UI::Modal_Message_Box mmb
			(&parent,
			 _("Error!"),
			 _("Can't change player tribe. It is referenced in some place. Remove "
			   "all buildings, bobs, triggers and events that depend on this "
			   "tribe and try again"),
			 UI::Modal_Message_Box::OK);
		mmb.run();
	}
   update();
}


/*
 * Set Current Start Position button selected
 */
void Editor_Player_Menu::set_starting_pos_clicked(const Uint8 n) {
	Editor_Interactive & parent =
		dynamic_cast<Editor_Interactive &>(*get_parent());
   // jump to the current field
	Map & map =parent.egbase().map();
   if (const Coords sp = map.get_starting_pos(n)) parent.move_view_to(sp);

   // If the player is already created in the editor, this means
   // that there might be already a hq placed somewhere. This needs to be
   // deleted before a starting position change can occure
	if (parent.egbase().get_player(n))
		if (const Coords sp = map.get_starting_pos(n))
			if (dynamic_cast<const Building *> (map[sp].get_immovable())) return;

   // Select tool set mplayer
	parent.select_tool(parent.tools.set_starting_pos, Editor_Tool::First);
	parent.tools.set_starting_pos.set_current_player(n);

   // Reselect tool, so everything is in a defined state
	parent.select_tool(parent.tools.current(), Editor_Tool::First);

   // Register callback function to make sure that only valid fields are selected.
	map.overlay_manager().register_overlay_callback_function
		(&Editor_Tool_Set_Starting_Pos_Callback, &map);
	map.recalc_whole_map();
   update();
}

/*
 * Player name has changed
 */
void Editor_Player_Menu::name_changed(int32_t m) {
   // Player name has been changed
   std::string text=m_plr_names[m]->get_text();
	Editor_Interactive & parent =
		dynamic_cast<Editor_Interactive &>(*get_parent());
	Map & map = parent.egbase().map();
   if (text=="") {
		text = map.get_scenario_player_name(m + 1);
      m_plr_names[m]->set_text(text.c_str());
	}
	map.set_scenario_player_name(m + 1, text);
	m_plr_names[m]->set_text(map.get_scenario_player_name(m + 1).c_str());
	parent.set_need_save(true);
}
