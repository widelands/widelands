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

#include "events/event_chain.h"
#include "map_eventchain_manager.h"
#include "trigger/trigger_conditional.h"

#include <vector>

namespace Widelands {

MapEventChainManager::MapEventChainManager() {}


MapEventChainManager::~MapEventChainManager() {
	for (uint32_t i = 0; i < m_eventchains.size(); ++i)
      delete m_eventchains[i];
   m_eventchains.resize(0);
}


bool MapEventChainManager::register_new_eventchain(EventChain* mv) {
   // check if this eventchain is already known
	if (get_eventchain(mv->name().c_str()))
         return 0;

   m_eventchains.push_back(mv);
   return true;
}

/*
 * Get eventchains
 */
EventChain* MapEventChainManager::get_eventchain(const char * const name) const
{
   EventChain* retval = 0;
	for (uint32_t i = 0; i < m_eventchains.size(); ++i) {
      if (!strcmp(m_eventchains[i]->name().c_str(), name)) {
         retval = m_eventchains[i];
         break;
		}
	}

   return retval;
}

/*
 * Remove a eventchain
 */
void MapEventChainManager::delete_eventchain(const std::string & name) {
	for (uint32_t i = 0; i < m_eventchains.size(); ++i) {
      if (m_eventchains[i]->name() == name) {
         assert(m_eventchains[i]->get_trigcond());
         m_eventchains[i]->get_trigcond()->unreference_triggers(m_eventchains[i]);
         m_eventchains[i]->clear_events();
         delete m_eventchains[i];
         m_eventchains[i] = m_eventchains[m_eventchains.size() - 1];
         m_eventchains.resize(m_eventchains.size() - 1);
         break;
		}
	}
}

};
