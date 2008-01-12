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

#ifndef __S__TRIGGER_MANAGER_H
#define __S__TRIGGER_MANAGER_H

#include <cassert>
#include <vector>

namespace Widelands {

class Trigger;

/*
 * The Trigger Manager keeps all triggers
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
struct MapTriggerManager {
      MapTriggerManager();
      ~MapTriggerManager();

      /*
       * Register a new trigger
       */
      bool register_new_trigger(Trigger*);

      /*
       * Delete unused trigger
       */
      void delete_unreferenced_triggers();

      /*
       * Get a trigger
       */
      Trigger* get_trigger(const char* name);
      void delete_trigger(const char* name);

	typedef std::vector<Trigger *> trigger_vector;
	typedef trigger_vector::size_type Index;
	Index get_nr_triggers() const {return m_triggers.size();}
	Trigger & get_trigger_by_nr(const Index i) const
	{assert(i < m_triggers.size()); return *m_triggers[i];}

private:
	trigger_vector      m_triggers;
};

};

#endif
