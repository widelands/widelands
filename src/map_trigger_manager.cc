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

#include "map_trigger_manager.h"

#include "trigger/trigger.h"

#include <vector>


MapTriggerManager::MapTriggerManager() {}


MapTriggerManager::~MapTriggerManager() {
   for (uint i = 0; i < m_triggers.size(); i++)
      delete m_triggers[i];
   m_triggers.resize(0);
}


bool MapTriggerManager::register_new_trigger(Trigger* mv) {
   // check if this trigger is already known
   if (get_trigger(mv->get_name()))
         return 0;

   m_triggers.push_back(mv);
   return true;
}


Trigger* MapTriggerManager::get_trigger(const char* name) {
   uint i;
   Trigger* retval = 0;
   for (i = 0; i < m_triggers.size(); i++) {
      if (!strcmp(m_triggers[i]->get_name(), name)) {
         retval = m_triggers[i];
         break;
		}
	}

   return retval;
}


void MapTriggerManager::delete_trigger(const char* name) {
   for (uint i = 0; i < m_triggers.size(); i++) {
      if (!strcmp(m_triggers[i]->get_name(), name)) {
         delete m_triggers[i];
         m_triggers[i] = m_triggers[m_triggers.size() - 1];
         m_triggers.resize(m_triggers.size() - 1);
         break;
		}
	}
}


void MapTriggerManager::delete_unreferenced_triggers() {
   uint i = 0;
   while (i < m_triggers.size()) {
      Trigger* tr = m_triggers[i];
      if (tr->get_referencers().empty()) {
         delete_trigger(tr->get_name());
         i = 0;
         continue;
		}
      ++i;
	}
}
