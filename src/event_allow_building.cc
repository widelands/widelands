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

#include "editorinteractive.h"
#include "editor_game_base.h"
#include "event_allow_building.h"
#include "error.h"
#include "filesystem.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "tribe.h"

static const int EVENT_VERSION = 1;

/*
 * Init and cleanup
 */
Event_Allow_Building::Event_Allow_Building(void) {
   set_name("Allow Building");
   set_is_one_time_event(true);
   set_player(0);
   set_building("<undefined>");
   set_allow(true);
}

Event_Allow_Building::~Event_Allow_Building(void) {
}

/*
 * cleanup()
 */
void Event_Allow_Building::cleanup(Editor_Game_Base* g) {
   // Nothing todo
   Event::cleanup(g);
}

/*
 * reinitialize
 */
void Event_Allow_Building::reinitialize(Game* g) {
   if(is_one_time_event()) {
      cleanup(g); // Also calls event cleanup
   } else {
      Event::reinitialize(g);
   }
}

/*
 * File Read, File Write
 */
void Event_Allow_Building::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip) {
   int version=fr->Unsigned16();
   if(version <= EVENT_VERSION) {
      set_name(fr->CString());
      set_is_one_time_event(fr->Unsigned8());
      int player=fr->Signed8();
      set_player(player);
      set_building(fr->CString());
      set_allow(fr->Unsigned8());
      read_triggers(fr,egbase, skip);
      if(player<=0 || player>egbase->get_map()->get_nrplayers() || m_building=="<undefined>") {
         // we're not configured and can't jump. delete us
         // but give a warning
         log("Conquer Area Event with illegal player orbuilding name: (Player: %i, Building: %s) deleted!\n", m_player, m_building.c_str());
         set_is_one_time_event(true);
         cleanup(egbase);
      }
      if(!egbase->is_game()) 
         static_cast<Editor_Interactive*>(egbase->get_iabase())->reference_player_tribe(player, this);
      return;
   }
   throw wexception("Allow Building Event with unknown/unhandled version %i in map!\n", version);
}

void Event_Allow_Building::Write(FileWrite* fw, Editor_Game_Base *egbase) {
   // First of all the id
   fw->Unsigned16(get_id());

   // Now the version
   fw->Unsigned16(EVENT_VERSION);

   // Name
   fw->Data(get_name(), strlen(get_name()));
   fw->Unsigned8('\0');

   // triggers only once?
   fw->Unsigned8(is_one_time_event());

   // Player
   fw->Signed8(get_player());
   
   // Building name
   fw->Data(m_building.c_str(), m_building.size());
   fw->Unsigned8('\0');
   
   // Allow or disallow 
   fw->Unsigned8(m_allow);

   // Write all trigger ids
   write_triggers(fw, egbase);
   // done
}

/*
 * run the event
 */
void Event_Allow_Building::run(Game* game) {
   assert(m_player>0 && m_player<=game->get_map()->get_nrplayers());

   Player* plr=game->get_player(m_player);
   
   assert(plr);
   
   Tribe_Descr* tribe=plr->get_tribe();
   int index=tribe->get_building_index(m_building.c_str());
   if(index==-1) 
      throw wexception("Event Allow Building asks for building: %s, which doesn't exist in tribe %s\n", m_building.c_str(), tribe->get_name());

   plr->allow_building(index, m_allow);

   // If this is a one timer, release our triggers
   // and forget about us
   reinitialize(game);
}


