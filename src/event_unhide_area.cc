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

#include "event_unhide_area.h"
#include "error.h"
#include "filesystem.h"
#include "game.h"
#include "editor_game_base.h"
#include "map.h"
#include "player.h"

static const int EVENT_VERSION = 1;

/* 
 * Init and cleanup
 */
Event_Unhide_Area::Event_Unhide_Area(void) {
   set_name("Unhide Area");
   set_is_one_time_event(true);
   set_coords(Coords(-1,-1));
   set_player(-1);
   set_area(5);
}

Event_Unhide_Area::~Event_Unhide_Area(void) {
}

/*
 * cleanup()
 */
void Event_Unhide_Area::cleanup(Editor_Game_Base* g) {  
   // Nothing todo
   Event::cleanup(g);
}

/*
 * reinitialize
 */
void Event_Unhide_Area::reinitialize(Game* g) {
   if(is_one_time_event()) {
      cleanup(g); // Also calls event cleanup
   } else {
      Event::reinitialize(g);
   }
}

/*
 * File Read, File Write
 */
void Event_Unhide_Area::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip) {
   int version=fr->Unsigned16();
   if(version <= EVENT_VERSION) {
      set_name(fr->CString());
      set_is_one_time_event(fr->Unsigned8());
      int x=fr->Signed16();
      int y=fr->Signed16();
      m_pt.x=x;
      m_pt.y=y;
      set_area(fr->Signed16());
      int player=fr->Signed8();
      set_player(player);
      read_triggers(fr,egbase, skip);
      if(x<0 || y<0 || x>=((int)egbase->get_map()->get_width()) || y>=((int)egbase->get_map()->get_height()) || player<=0 || player>egbase->get_map()->get_nrplayers()) {
         // we're not configured and can't jump. delete us
         // but give a warning
         log("Unhide Area Event with illegal coordinates or player: (%i,%i) (Player: %i) deleted!\n", x,y, player);
         set_is_one_time_event(true);
         cleanup(egbase);
      }
      return;
   }
   throw wexception("Unhide Area Event with unknown/unhandled version %i in map!\n", version);
}

void Event_Unhide_Area::Write(FileWrite* fw, Editor_Game_Base *egbase) {
   // First of all the id
   fw->Unsigned16(get_id());

   // Now the version
   fw->Unsigned16(EVENT_VERSION);

   // Name
   fw->Data(get_name(), strlen(get_name()));
   fw->Unsigned8('\0');
   
   // triggers only once?
   fw->Unsigned8(is_one_time_event());

   // Point
   fw->Signed16(m_pt.x);
   fw->Signed16(m_pt.y);

   // Area
   fw->Signed16(get_area());

   // Player
   fw->Signed8(get_player());

   // Write all trigger ids
   write_triggers(fw, egbase);
   // done
}
      
/*
 * run the event 
 */
void Event_Unhide_Area::run(Game* game) {
   assert(m_pt.x!=-1 && m_pt.y!=-1);
   assert(m_player>0 && m_player<game->get_map()->get_nrplayers());

   Player* player=game->get_player(m_player);
   player->set_area_seen(Coords(m_pt.x,m_pt.y),get_area(), true);
   
   // If this is a one timer, release our triggers 
   // and forget about us
   reinitialize(game);
}


