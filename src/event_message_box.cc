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

#include "event_message_box.h"
#include "error.h"
#include "filesystem.h"
#include "game.h"
#include "editor_game_base.h"
#include "event_message_box_message_box.h"
#include "map.h"
#include "graphic.h"
#include "trigger_null.h"

static const int EVENT_VERSION = 1;

/*
 * Init and cleanup
 */
Event_Message_Box::Event_Message_Box(void) {
   set_name("Message Box");
   set_is_one_time_event(true);
   set_text("No text defined");
   set_caption("Caption");
   set_window_title("Window Title");
   set_is_modal(false);
   set_pic_id(-1);
   set_pic_position(Right);

   set_nr_buttons(1);
   m_buttons[0].name="Continue";
   m_buttons[0].trigger=0;
}

Event_Message_Box::~Event_Message_Box(void) {
}

/*
 * cleanup()
 */
void Event_Message_Box::cleanup(Editor_Game_Base* g) {
   uint i=0;
   for(i=0; i<m_buttons.size(); i++)
      if(m_buttons[i].trigger) {
         set_button_trigger(i, 0, g->get_map());
      }
   m_buttons.resize(0);

     Event::cleanup(g);
}

/*
 * reinitialize
 */
void Event_Message_Box::reinitialize(Game* g) {
   if(is_one_time_event()) {
      cleanup(g); // Also calls event cleanup
   } else {
      Event::reinitialize(g);
   }
}

/*
 * functions for button handling
 */
void Event_Message_Box::set_nr_buttons(int i) {
   int oldsize=m_buttons.size();
   m_buttons.resize(i);
   for(uint i=oldsize; i<m_buttons.size(); i++)
      m_buttons[i].trigger=0;
}
int Event_Message_Box::get_nr_buttons(void) {
   return m_buttons.size();
}
void Event_Message_Box::set_button_trigger(int i, Trigger_Null* t, Map* map) {
   assert(i<get_nr_buttons());
   if(m_buttons[i].trigger==t) return;

   if(m_buttons[i].trigger)
      map->release_trigger(m_buttons[i].trigger);

   if(t)
      map->reference_trigger(t);
   m_buttons[i].trigger=t;
}
Trigger_Null* Event_Message_Box::get_button_trigger(int i) {
   assert(i<get_nr_buttons());
   return m_buttons[i].trigger;
}
void Event_Message_Box::set_button_name(int i, std::string str) {
   assert(i<get_nr_buttons());
   m_buttons[i].name=str;
}
const char* Event_Message_Box::get_button_name(int i) {
   assert(i<get_nr_buttons());
   return m_buttons[i].name.c_str();
}

/*
 * File Read, File Write
 */
void Event_Message_Box::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip) {
   int version=fr->Unsigned16();
   if(version <= EVENT_VERSION) {
      set_name(fr->CString());
      set_is_one_time_event(fr->Unsigned8());
      set_text(fr->CString());
      set_caption(fr->CString());
      set_window_title(fr->CString());
      set_is_modal(fr->Unsigned8());
      bool has_pic=fr->Unsigned8();
      if(has_pic) {
         set_pic_position(fr->Unsigned8());
         set_pic_id(g_gr->load_pic_from_file(fr, PicMod_Game));
         if(skip)
            g_gr->flush_picture(get_pic_id());
      }
      set_nr_buttons(fr->Unsigned8());
      int i=0;
      for(i=0; i<get_nr_buttons(); i++) {
         set_button_name(i,fr->CString());
         int id=fr->Signed16();
         if(id!=-1 && !skip) {
            Trigger* trig=egbase->get_map()->get_trigger(id);
            if(trig->get_id()!=TRIGGER_NULL)
               throw wexception("Trigger of message box %s is not a Null Trigger!\n", get_name());
            set_button_trigger(i, static_cast<Trigger_Null*>(trig), egbase->get_map());
         } else
            set_button_trigger(i,0,egbase->get_map());
      }

      read_triggers(fr,egbase, skip);
      return;
   }
   throw wexception("Message Box Event with unknown/unhandled version %i in map!\n", version);
}

void Event_Message_Box::Write(FileWrite* fw, Editor_Game_Base *egbase) {
   // First of all the id
   fw->Unsigned16(get_id());

   // Now the version
   fw->Unsigned16(EVENT_VERSION);

   // Name
   fw->Data(get_name(), strlen(get_name()));
   fw->Unsigned8('\0');

   // triggers only once?
   fw->Unsigned8(is_one_time_event());


   // Description string
   fw->Data(m_text.c_str(), m_text.size());
   fw->Unsigned8('\0');

   // Caption
   fw->Data(m_caption.c_str(), m_caption.size());
   fw->Unsigned8('\0');

   // Window Title
   fw->Data(m_window_title.c_str(), m_window_title.size());
   fw->Unsigned8('\0');

   // is modal
   fw->Unsigned8(get_is_modal());

   // has picture
   fw->Unsigned8(static_cast<int>(get_pic_id())!=-1);
   if(static_cast<int>(get_pic_id())!=-1) {
      // Write picture
      // Pic position
      fw->Unsigned8(get_pic_position());
      g_gr->save_pic_to_file(get_pic_id(), fw);
   }

   // Number of buttons
   fw->Unsigned8(get_nr_buttons());
   int i=0;
   for(i=0; i<get_nr_buttons(); i++) {
      fw->Data(m_buttons[i].name.c_str(), m_buttons[i].name.size());
      fw->Unsigned8('\0');
      if(m_buttons[i].trigger)
         fw->Signed16(egbase->get_map()->get_trigger_index(m_buttons[i].trigger));
      else
         fw->Signed16(-1);
   }

   // Write all trigger ids
   write_triggers(fw, egbase);
   // done
}

/*
 * check if trigger conditions are done
 */
void Event_Message_Box::run(Game* game) {

   Message_Box_Event_Message_Box* mb=new Message_Box_Event_Message_Box(game, this);
   if(get_is_modal()) {
      mb->run();
      delete mb;
   }

   // If this is a one timer, release our triggers
   // and forget about us
   reinitialize(game);
}


