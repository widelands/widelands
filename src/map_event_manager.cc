/*
 * Copyright (C) 2002-2005, 2007 by the Widelands Development Team
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

#include "map_event_manager.h"

#include "events/event.h"

#include <vector>

namespace Widelands {

MapEventManager::MapEventManager() {}

MapEventManager::~MapEventManager() {
	for (uint32_t i = 0; i < m_events.size(); ++i)
      delete m_events[i];
   m_events.resize(0);
}


bool MapEventManager::register_new_event(Event* mv) {
   // check if this event is already known
	if (get_event(mv->name().c_str()))
		return false;

   m_events.push_back(mv);
   return true;
}

/*
 * Get events
 */
Event* MapEventManager::get_event(const char* name) {
	event_vector::const_iterator const events_end = m_events.end();
	for
		(event_vector::const_iterator it = m_events.begin();
		 it != events_end;
		 ++it)
		if (!strcmp((*it)->name().c_str(), name))
			return *it;
	return 0;
}

/*
 * Remove a event
 */
void MapEventManager::delete_event(const char* name) {
	event_vector::const_iterator const events_end = m_events.end();
	for
		(event_vector::iterator it = m_events.begin();
		 it != events_end;
		 ++it)
		if (!strcmp((*it)->name().c_str(), name)) {
			delete *it;
			*it = *(events_end - 1);
			m_events.pop_back();
         break;
		}
}

/*
 * Delete all unreferenced events
 */
void MapEventManager::delete_unreferenced_events() {
   uint32_t i = 0;
	while (i < m_events.size()) {
      Event* tr = m_events[i];
		if (tr->get_referencers().empty()) {
         delete_event(tr->name().c_str());
         i = 0;
         continue;
		}
      ++i;
	}
}

};
