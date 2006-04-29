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
#include "i18n.h"
#include "map.h"
#include "map_variable_manager.h"
#include "profile.h"
#include "trigger_building.h"
#include "util.h"

static const int TRIGGER_VERSION = 1;

/*
 * Init and cleanup
 */
Trigger_Building::Trigger_Building(void) {
   set_name(_("Building Trigger"));
   set_trigger(false);
   m_count=-1;
   m_area=-1;
   m_pt.x=0;
   m_pt.y=0;
   m_player=-1;
   m_building=_("<unset>");
}

Trigger_Building::~Trigger_Building(void) {
}

/*
 * File Read, File Write
 */
void Trigger_Building::Read(Section* s, Editor_Game_Base* egbase) {
   int version= s->get_safe_int( "version" );

   if(version == TRIGGER_VERSION) {
      m_pt.x = s->get_safe_int( "point_x" );
      m_pt.y = s->get_safe_int( "point_y" );
      set_area( s->get_safe_int( "area" ));
      int player = s->get_safe_int( "player" );
      set_player(player);
      if(!egbase->is_game())
         static_cast<Editor_Interactive*>(egbase->get_iabase())->reference_player_tribe(player, this);
      set_building_count( s->get_int( "count" ));
      set_building( s->get_safe_string( "building" ));
      return;
   }
   throw wexception("Building Trigger with unknown/unhandled version %i in map!\n", version);
}

void Trigger_Building::Write(Section* s) {
   // the version
   s->set_int("version", TRIGGER_VERSION );

   // Point
   s->set_int("point_x", m_pt.x);
   s->set_int("point_y", m_pt.y);

   // Area
   s->set_int("area", get_area());

   // Player
   s->set_int("player", get_player());

   // Count
   s->set_int("count", get_building_count());

   // Building
   s->set_string( "building", m_building.c_str() );
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
      if(name != m_building) continue;
      ++count;
   }

   if(count>=m_count) set_trigger(true);

   // Set MapVariable inttemp
   Int_MapVariable* inttemp = game->get_map()->get_mvm()->get_int_variable( "inttemp" );
   if( !inttemp ) {
      inttemp = new Int_MapVariable( false );
      inttemp->set_name( "inttemp" );
      game->get_map()->get_mvm()->register_new_variable( inttemp );
   }
   inttemp->set_value( count );

   return;
}

/*
 * Reset this trigger. This is only valid for non one timers
 */
void Trigger_Building::reset_trigger(Game* game) {
}
