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

#ifndef __S__OBJECTIVE_MANAGER_H
#define __S__OBJECTIVE_MANAGER_H

#include "i18n.h"
#include "trigger/trigger_null.h"
#include "trigger/trigger_referencer.h"

#include <cassert>
#include <string>

/*
 * The Map Objective manager keeps all objectives
 * in Order.
 *
 * A Map (or scenario) objective is a objectives that has to be
 * fullfilled to end a scenario successfull.
 * Optional objectives are available, which usually do not need to be fullfilled.
 * Each objectives has a NULL trigger assigned to it, which is used to check
 * the objectives condition.
 * But note, the objectives itself doesn't check it's conditions,
 * the map designer is responsible of checking it and setting it's trigger up.
 *
 * Usually, the win trigger is only set, when all of the objectives triggers are
 * going up.
 *
 * Also note, that each game has objectives. These might be in a network game
 * something like "Crush you enemy" or "Have the most points at the end of the game".
 * Depending on choosen game type.
 */

struct MapObjective : public TriggerReferencer {
	MapObjective()
		:
		m_descr      (_("no descr")),
		m_trigger    (0),
		m_is_visible (true),
		m_is_optional(false)
	{}
      virtual ~MapObjective() {
         if (m_trigger)
            unreference_trigger(m_trigger);
		}


	const char * get_name() const {return m_name.c_str();}
      inline void set_name(const char* name) {m_name = name;}
	const char * get_descr() const {return m_descr.c_str();}
      inline void set_descr(const char* descr) {m_descr = descr;}
	bool get_is_visible() const {return m_is_visible;}
      inline void set_is_visible(bool t) {m_is_visible = t;}
	bool get_is_optional() const {return m_is_optional;}

      // For trigger referncer
	const char * get_type() const {return "Map Objective";}

      // Get the trigger that is attached to this
      // Trigger is created by Editor or on load
	Trigger_Null * get_trigger() const {return m_trigger;}

      // Setting the values below is only a good idea in editor
      inline void set_trigger(Trigger_Null* tr) {
         assert(!m_trigger);
         if (m_trigger)
            unreference_trigger(m_trigger);
         if (tr)
            reference_trigger(tr);
         m_trigger = tr;
		}
      inline void set_is_optional(bool t) {m_is_optional = t;}

private:
      std::string   m_name;
      std::string   m_descr;
      Trigger_Null*  m_trigger;
      bool           m_is_visible;
      bool           m_is_optional;
};

/*
 * The manager himself.
 * This is mainly a wrapper, the function
 * could have also been implemented directly in the map.
 *
 * But it is better this way.
 */
struct MapObjectiveManager {
      MapObjectiveManager();
      ~MapObjectiveManager();

      /*
       * Register a new objective
       */
      bool register_new_objective(MapObjective*);

      /*
       * Get a objective
       */
	MapObjective * get_objective(const char * const name) const;
      void delete_objective(const char* name);

	typedef std::vector<MapObjective *> objective_vector;
	typedef objective_vector::size_type Index;
	Index get_nr_objectives() const {return m_objectives.size();}
	MapObjective & get_objective_by_nr(const Index i) const
	{assert(i < m_objectives.size()); return *m_objectives[i];}

private:
	objective_vector m_objectives;
};

#endif
