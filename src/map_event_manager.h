/*
 * Copyright (C) 2002-5 by the Widelands Development Team
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

#ifndef __S__EVENT_MANAGER_H
#define __S__EVENT_MANAGER_H

#include "error.h"
#include "event_chain.h"
#include <string>

class Event;

/*
 * The Event Manager keeps all events 
 * in order
 *
 */
/*
 * The manager himself. 
 * This is mainly a wrapper, the function
 * could have also been implemented directly in the map.
 *
 * But it is better this way.
 */
class MapEventManager {
   public:
      MapEventManager( void );
      ~MapEventManager( void );

      /*
       * Register a new event
       */
      bool register_new_event( Event* );

      /*
       * Get a event
       */
      Event* get_event( const char* name );
      void delete_event( const char* name );

      /*
       * Delete all unreferenced events
       */
      void delete_unreferenced_events( void );

      inline int get_nr_events( void ) { return m_events.size(); }
      inline Event* get_event_by_nr( int i ) { assert(i < (int)m_events.size()); return m_events[i]; }

   private:
      std::vector<Event*>      m_events; 
};

#endif

