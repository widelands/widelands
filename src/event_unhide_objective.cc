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

#include "editor_game_base.h"
#include "error.h"
#include "event_unhide_objective.h"
#include "filesystem.h"
#include "game.h"
#include "interactive_base.h"
#include "map.h"
#include "map_objective_manager.h"

static const int EVENT_VERSION = 1;

/*
 * Init and cleanup
 */
Event_Unhide_Objective::Event_Unhide_Objective(void) {
   set_name(_("Move View"));
   set_dounhide(true);
   set_objective(0);
}

Event_Unhide_Objective::~Event_Unhide_Objective(void) {
}

/*
 * reinitialize
 */
void Event_Unhide_Objective::reinitialize(Game* g) {
}

/*
 * File Read, File Write
 */
void Event_Unhide_Objective::Read(Section* s, Editor_Game_Base* egbase) {
   int version=s->get_safe_int("version"); 
   if(version == EVENT_VERSION) {
      std::string name = s->get_safe_string("objective");
      MapObjective* obj = egbase->get_map()->get_mom()->get_objective( name.c_str() );
      if( !obj ) {
         throw wexception("Unhide Objective event with unknown objecive %s in map!\n", name.c_str());
      }
      set_dounhide(s->get_bool("dounhide"));
      return;
   }
   throw wexception("Unhide Objective Event with unknown/unhandled version %i in map!\n", version);
}

void Event_Unhide_Objective::Write(Section* s, Editor_Game_Base *egbase) {
   assert( m_objective );

   // the version
   s->set_int("version", EVENT_VERSION);

   // Point
   s->set_string("objective", m_objective->get_name());
   s->set_bool("setto", get_dounhide());
   // done
}

/*
 * check if trigger conditions are done
 */
Event::State Event_Unhide_Objective::run(Game* game) {
   assert( m_objective );

   m_objective->set_is_visible( get_dounhide());

   m_state = DONE;
   return m_state;
}


