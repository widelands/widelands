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

#include "event.h"
#include "map.h"
#include "editor_game_base.h"
#include "game.h"
#include "filesystem.h"

/*
 * does the trigger already exist?
 *
 *  [private function]
 */
bool Event::trigger_exists(Trigger* trig) {
   std::vector<Trigger_Info>::iterator i;
   for(i=m_triggers.begin(); i!=m_triggers.end(); i++)
      if((*i).t==trig) {
         return true;
      }
   return false;
}

/*
 * register and unregister triggers
 * with this event.
 */
void Event::register_trigger(Trigger* trig, Map* m, bool up) {
   assert(!trigger_exists(trig));
   Trigger_Info t= { trig, up };
   m_triggers.push_back(t);
   m->reference_trigger(trig);
}

void Event::unregister_trigger(Trigger* t, Map* m) {
   assert(trigger_exists(t));
   std::vector<Trigger_Info>::iterator i;
   for(i=m_triggers.begin(); i!=m_triggers.end(); i++) {
      if((*i).t==t) {
         m_triggers.erase(i);
         m->release_trigger(t);
         return;
      }
   }
   assert(0);
}

/*
 * Read and write common data
 */
void Event::write_triggers(FileWrite* fw, Editor_Game_Base* egbase) {
   fw->Unsigned16(get_nr_triggers());
   int i=0;
   for(i=0; i<get_nr_triggers(); i++) {
      fw->Unsigned16(egbase->get_map()->get_trigger_index(m_triggers[i].t));
      fw->Unsigned8(m_triggers[i].up);
   }

}
void Event::read_triggers(FileRead* fr, Editor_Game_Base* egbase, bool skip) {
   int nr_triggers=fr->Unsigned16();
   int i=0;
   for(i=0; i<nr_triggers; i++) {
      int id=fr->Unsigned16();
      bool up=fr->Unsigned8();
      if(!skip) {
         Trigger* t=egbase->get_map()->get_trigger(id);
         register_trigger(t,egbase->get_map(),up);
      }
   }
}

/*
 * check if all triggers are in the requested state, if they are
 * return true
 */
bool Event::check_triggers(void) {
   for(int i=0; i<get_nr_triggers(); i++)
      if(m_triggers[i].t->is_set()!=m_triggers[i].up)
         return false;
   return true;
}

/*
 * Release all triggers and unregister this event if it is a one timer.
 */
void Event::reinitialize(Game* game) {
   if(is_one_time_event()) {
     Event::cleanup(game);
   } else {
      // toggle all those triggers back
      int i=0;
      for(i=0; i<get_nr_triggers(); i++)
         m_triggers[i].t->reset_trigger(game);
   }
}


/*
 * Cleanup
 */
void Event::cleanup(Editor_Game_Base* g) {
   Map* map=g->get_map();
   while(get_nr_triggers())
      unregister_trigger(m_triggers[0].t, map);
   log("Unregistering event: %s. %p\n", get_name(), this);
   g->get_map()->unregister_event(this);
}

/*
 * Returns true when the event should react when this trigger is set
 */
bool Event::reacts_when_trigger_is_set(Trigger* t) {
   assert(trigger_exists(t));
   std::vector<Trigger_Info>::iterator i;
   for(i=m_triggers.begin(); i!=m_triggers.end(); i++) {
      if((*i).t==t) {
         return i->up==true;
      }
   }
   assert(0);
   throw;
}

bool Event::set_reacts_when_trigger_is_set(Trigger* t, bool up) {
   assert(trigger_exists(t));
   std::vector<Trigger_Info>::iterator i;
   for(i=m_triggers.begin(); i!=m_triggers.end(); i++) {
      if((*i).t==t) {
         return i->up=up;
      }
   }
   assert(0);
   throw;
}

