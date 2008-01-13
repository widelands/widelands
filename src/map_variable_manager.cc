/*
 * Copyright (C) 2002-2008 by the Widelands Development Team
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

#include "map_variable_manager.h"

#include "i18n.h"

#include <vector>

namespace Widelands {

MapVariableManager::MapVariableManager() {
   // Create a default variable
   String_MapVariable* smv = new String_MapVariable(1);
   smv->set_name(_("Next scenario").c_str());
   smv->set_value(_("<undefined>").c_str());
   register_new_variable(smv);
}


MapVariableManager::~MapVariableManager() {
	for (uint32_t i = 0; i < m_variables.size(); ++i)
      delete m_variables[i];
   m_variables.resize(0);
}


bool MapVariableManager::register_new_variable(MapVariable* mv) {
   // check if this variable is already known
	if (get_variable(mv->get_name().c_str()))
		return false;

   m_variables.push_back(mv);
   return true;
}

/**
 * Get variables
 */
Int_MapVariable * MapVariableManager::get_int_variable
(const char * const name) const
{
	return dynamic_cast<Widelands::Int_MapVariable    *>(get_variable(name));
}
String_MapVariable * MapVariableManager::get_string_variable
(const char * const name) const
{
	return dynamic_cast<Widelands::String_MapVariable *>(get_variable(name));
}

MapVariable* MapVariableManager::get_variable
(const char * const name) const
{
	for (uint32_t i = 0; i < m_variables.size(); ++i)
		if (!strcmp(m_variables[i]->get_name().c_str(), name))
         return m_variables[i];
	return 0;
}

/**
 * Remove a variable
 */
void MapVariableManager::delete_variable(MapVariable const & var) {
	for (uint32_t i = 0; i < m_variables.size(); ++i) {
		if (m_variables[i] == &var) {
         delete m_variables[i];
         m_variables[i] = m_variables[m_variables.size() - 1];
         m_variables.resize(m_variables.size() - 1);
         break;
		}
	}
}

};
