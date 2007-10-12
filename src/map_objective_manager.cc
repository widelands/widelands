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

#include "map_objective_manager.h"

#include <stdint.h>
#include <vector>


MapObjectiveManager::MapObjectiveManager() {}

MapObjectiveManager::~MapObjectiveManager() {
   for (uint32_t i = 0; i < m_objectives.size(); ++i)
      delete m_objectives[i];
   m_objectives.resize(0);
}


bool MapObjectiveManager::register_new_objective(MapObjective* mv) {
   // check if this objective is already known
	if (get_objective(mv->name().c_str()))
         return 0;

   m_objectives.push_back(mv);
   return true;
}


MapObjective* MapObjectiveManager::get_objective(const char * const name) const
{
   uint32_t i;
   MapObjective* retval = 0;
	for (i = 0; i < m_objectives.size(); ++i) {
      if (!strcmp(m_objectives[i]->name().c_str(), name)) {
         retval = m_objectives[i];
         break;
		}
	}

   return retval;
}


void MapObjectiveManager::delete_objective(const std::string & name) {
	for (uint32_t i = 0; i < m_objectives.size(); ++i) {
      if (m_objectives[i]->name() == name) {
         delete m_objectives[i];
         m_objectives[i] = m_objectives[m_objectives.size() - 1];
         m_objectives.resize(m_objectives.size() - 1);
         break;
		}
	}
}
