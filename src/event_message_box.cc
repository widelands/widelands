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
#include "ui_window.h"
#include "editor_game_base.h"
#include "interactive_base.h"
#include "ui_multilinetextarea.h"

//
// This is kept in this file, since only this file will
// ever need to create one of those
// 
class Message_Box_Event_Message_Box : public UIWindow {
   public:
      Message_Box_Event_Message_Box(Editor_Game_Base*, Event_Message_Box*);
};

static const int EVENT_VERSION = 1;

/* 
 * Init and cleanup
 */
Event_Message_Box::Event_Message_Box(void) {
   set_name("Message Box");
   set_is_one_time_event(true);
}

Event_Message_Box::~Event_Message_Box(void) {
}

/*
 * File Read, File Write
 */
void Event_Message_Box::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip) {
   int version=fr->Unsigned16();
   if(version <= EVENT_VERSION) {
      set_name(fr->CString());
      set_is_one_time_event(fr->Unsigned8());
      m_string=fr->CString();
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
   fw->Data(m_string.c_str(), m_string.size());
   fw->Unsigned8('\0');

   // Write all trigger ids
   write_triggers(fw, egbase);
   // done
}
      
/*
 * check if trigger conditions are done
 */
void Event_Message_Box::run(Game* game) {
   new Message_Box_Event_Message_Box(game, this);

   // If this is a one timer, release our triggers 
   // and forget about us
   reinitialize(game);
}

/*
 * The message box himself
 */
Message_Box_Event_Message_Box::Message_Box_Event_Message_Box(Editor_Game_Base* egbase, Event_Message_Box* m_event) :
  UIWindow(egbase->get_iabase(), 0, 0, 400, 300, "Message Box") {
   
     new UIMultiline_Textarea(this, 0,0,get_w(),get_h(), m_event->get_string(), Align_Left);

     center_to_parent();
}

