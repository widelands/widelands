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
#include "event.h"
#include "event_ids.h"
#include "event_factory.h"
#include "error.h"
#include "wexception.h"
#include "event_message_box.h"
#include "event_message_box_option_menu.h"
#include "event_move_view.h"
#include "event_move_view_option_menu.h"
#include "event_unhide_area.h"
#include "event_unhide_area_option_menu.h"
#include "event_conquer_area.h"
#include "event_conquer_area_option_menu.h"

static const int nr_of_events=4;

Event_Descr EVENT_DESCRIPTIONS[nr_of_events] = {
   { EVENT_MESSAGE_BOX, "Message Box", "This Event shows a messagebox. The user can choose to make it modal/non-modal and to add a picture. Events can be assigned"
                                       " to each button to use this as a Choose Dialog for the user" },
   { EVENT_MOVE_VIEW, "Move View", "This Event centers the Players View on a certain field" },
   { EVENT_UNHIDE_AREA, "Unhide Area", "This Event makes a user definable part of the map visible for a selectable user" },
   { EVENT_CONQUER_AREA, "Conquer Area", "This Event conquers a user definable part of the map for one player if there isn't a player already there" },
};

/*
 * return the correct event for this id
 */
Event* Event_Factory::get_correct_event(uint id) {
   switch(id) {
      case EVENT_MESSAGE_BOX: return new Event_Message_Box(); break;
      case EVENT_MOVE_VIEW: return new Event_Move_View(); break;
      case EVENT_UNHIDE_AREA: return new Event_Unhide_Area(); break;
      case EVENT_CONQUER_AREA: return new Event_Conquer_Area(); break;
      default: break;
   }
   throw wexception("Event_Factory::get_correct_event: Unknown event id found: %i\n", id);
   // never here
   return 0;
}

/*
 * create the correct option dialog and initialize it with the given
 * event. if the given event is zero, create a new event 
 * and let it be initalised through it.
 * if it fails, return zero/unmodified given event, elso return the created/modified event
 */
Event* Event_Factory::make_event_with_option_dialog(uint id, Editor_Interactive* m_parent, Event* gevent) {
   Event* event=gevent;
   if(!event) 
      event=get_correct_event(id);

   int retval=-100;
   switch(id) {
      case EVENT_MESSAGE_BOX: { Event_Message_Box_Option_Menu* t=new Event_Message_Box_Option_Menu(m_parent, static_cast<Event_Message_Box*>(event)); retval=t->run(); delete t; } break;
      case EVENT_MOVE_VIEW: { Event_Move_View_Option_Menu* t=new Event_Move_View_Option_Menu(m_parent, static_cast<Event_Move_View*>(event)); retval=t->run(); delete t; } break;
      case EVENT_UNHIDE_AREA: { Event_Unhide_Area_Option_Menu* t=new Event_Unhide_Area_Option_Menu(m_parent, static_cast<Event_Unhide_Area*>(event)); retval=t->run(); delete t; } break;
      case EVENT_CONQUER_AREA: { Event_Conquer_Area_Option_Menu* t=new Event_Conquer_Area_Option_Menu(m_parent, static_cast<Event_Conquer_Area*>(event)); retval=t->run(); delete t; } break;
      default: break;
   }
   if(retval==-100) 
      throw wexception("Event_Factory::make_event_with_option_dialog: Unknown event id found: %i\n", id);
   if(retval) 
      return event;
   if(!gevent) {
      delete event;
      return 0;
   } else return gevent;
   // never here
}

/*
 * Get the correct event descriptions and names from the
 * id header
 */   
Event_Descr* Event_Factory::get_correct_event_descr(uint id) {
   assert(id<Event_Factory::get_nr_of_available_events());
   
   return &EVENT_DESCRIPTIONS[id];
}

/*
 * return the nummer of available events
 */
const uint Event_Factory::get_nr_of_available_events(void) {
   return nr_of_events;
}

