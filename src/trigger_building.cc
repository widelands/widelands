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

#include "building.h"
#include "editor_game_base.h"
#include "editorinteractive.h"
#include "error.h"
#include "filesystem.h"
#include "game.h"
#include "map.h"
#include "trigger_building.h"

static const int TRIGGER_VERSION = 1;

/*
 * Init and cleanup
 */
Trigger_Building::Trigger_Building(void) {
   set_name("Building Trigger");
   set_trigger(false);
   set_is_one_time_trigger(true);

   m_count=-1;
   m_area=-1;
   m_pt.x=0;
   m_pt.y=0;
   m_player=-1;
   m_building="<unset>";
}

Trigger_Building::~Trigger_Building(void) {
}

/*
 * File Read, File Write
 */
void Trigger_Building::Read(FileRead* fr, Editor_Game_Base* egbase) {
   int version=fr->Unsigned16();
   if(version <= TRIGGER_VERSION) {
      set_name(fr->CString());
      int x=fr->Signed16();
      int y=fr->Signed16();
      m_pt.x=x;
      m_pt.y=y;
      set_area(fr->Signed16());
      int player=fr->Signed8();
      set_player(player);
      if(!egbase->is_game()) 
         static_cast<Editor_Interactive*>(egbase->get_iabase())->reference_player_tribe(player, this);
      set_building_count(fr->Signed8());
      set_building(fr->CString());
      return;
   }
   throw wexception("Building Trigger with unknown/unhandled version %i in map!\n", version);
}

void Trigger_Building::Write(FileWrite* fw) {
   // First of all the id
   fw->Unsigned16(get_id());

   // Now the version
   fw->Unsigned16(TRIGGER_VERSION);

   // Name
   fw->Data(get_name(), strlen(get_name()));
   fw->Unsigned8('\0');

   // Point
   fw->Signed16(m_pt.x);
   fw->Signed16(m_pt.y);

   // Area
   fw->Signed16(get_area());

   // Player
   fw->Signed8(get_player());

   // Count
   fw->Signed8(get_building_count());

   // Building
   fw->Data(m_building.c_str(), m_building.size());
   fw->Unsigned8('\0');
   // done
}

/*
 * check if trigger conditions are done
 */
void Trigger_Building::check_set_conditions(Game* game) {
   if(m_pt.x<0 || 
         m_pt.y<0 || 
         m_pt.x>=static_cast<int>(game->get_map()->get_width()) || 
         m_pt.y>=static_cast<int>(game->get_map()->get_height()))
      return;
   if(m_player<=0 || m_player>MAX_PLAYERS) return;
  
   MapRegion mrc(game->get_map(), m_pt, m_area);

   int count=0;
   FCoords f;
   while(mrc.next(&f)) {
      BaseImmovable* imm=f.field->get_immovable();
      if(!imm) continue;
      if(imm->get_type()!=Map_Object::BUILDING) continue;
     
      Building* b=static_cast<Building*>(imm);
      if(b->get_owner()!=game->get_player(m_player)) continue;
      std::string name=b->get_name();
      if(name!=m_building) continue;
      ++count; 
   }

   if(count>=m_count) set_trigger(true);

   return;
}

/*
 * Reset this trigger. This is only valid for non one timers
 */
void Trigger_Building::reset_trigger(Game* game) {
   // This shouldn't be called, this is a one time trigger always
   assert(0);
}
