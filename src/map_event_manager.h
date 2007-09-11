/*
 * Copyright (C) 2002-2007 by the Widelands Development Team
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

#include <cassert>
#include <string>
#include <vector>

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
struct MapEventManager {
	MapEventManager();
	~MapEventManager();

      /*
       * Register a new event
       */
      bool register_new_event(Event*);

      /*
       * Get a event
       */
      Event* get_event(const char* name);
      void delete_event(const char* name);

      /*
       * Delete all unreferenced events
       */
      void delete_unreferenced_events();

	typedef std::vector<Event *> event_vector;
	typedef event_vector::size_type Index;
	Index get_nr_events() const {return m_events.size();}
	Event & get_event_by_nr(const Index i) const
	{assert(i < m_events.size()); return *m_events[i];}

private:
	event_vector m_events;
};

#endif
