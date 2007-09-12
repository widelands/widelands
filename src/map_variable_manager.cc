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

#include "map_variable_manager.h"

#include "i18n.h"
#include "types.h"
#include <vector>


MapVariableManager::MapVariableManager() {
   // Create a default variable
   String_MapVariable* smv = new String_MapVariable(1);
   smv->set_name(_("Next scenario").c_str());
   smv->set_value(_("<undefined>").c_str());
   register_new_variable(smv);
}


MapVariableManager::~MapVariableManager() {
   for (uint i = 0; i < m_variables.size(); i++)
      delete m_variables[i];
   m_variables.resize(0);
}


bool MapVariableManager::register_new_variable(MapVariable* mv) {
   // check if this variable is already known
   if (get_variable(mv->get_name()))
         return 0;

   m_variables.push_back(mv);
   return true;
}

/*
 * Get variables
 */
Int_MapVariable * MapVariableManager::get_int_variable
(const char * const name) const
{
   MapVariable* v = get_variable(name);
   if (v && v->get_type() != MapVariable::MVT_INT)
      return 0;

   return static_cast<Int_MapVariable*>(v);
}
String_MapVariable * MapVariableManager::get_string_variable
(const char * const name) const
{
   MapVariable* v = get_variable(name);
   if (v && v->get_type() != MapVariable::MVT_STRING)
      return 0;
   return static_cast<String_MapVariable*>(v);
}

MapVariable* MapVariableManager::get_variable
(const char * const name) const
{
   uint i;
   MapVariable* retval = 0;
   for (i = 0; i < m_variables.size(); i++) {
      if (!strcmp(m_variables[i]->get_name(), name)) {
         retval = m_variables[i];
         break;
		}
	}

   return retval;
}

/*
 * Remove a variable
 */
void MapVariableManager::delete_variable(const char* name) {
   for (uint i = 0; i < m_variables.size(); i++) {
      if (!strcmp(m_variables[i]->get_name(), name)) {
         delete m_variables[i];
         m_variables[i] = m_variables[m_variables.size() - 1];
         m_variables.resize(m_variables.size() - 1);
         break;
		}
	}
}
