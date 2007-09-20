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

#ifndef __S__EVENTCHAIN_MANAGER_H
#define __S__EVENTCHAIN_MANAGER_H

#include <cassert>
#include <string>

class Event;
class EventChain;

/*
 * The Eventchain Manager keeps all event chains
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
struct MapEventChainManager {
      MapEventChainManager();
      ~MapEventChainManager();

      /*
       * Register a new eventchain
       */
      bool register_new_eventchain(EventChain*);

      /*
       * Get a eventchain
       */
	EventChain * get_eventchain(const char * const name) const;
	void delete_eventchain(const std::string & name);

	typedef std::vector<EventChain *> eventchain_vector;
	typedef eventchain_vector::size_type Index;
	Index get_nr_eventchains() const {return m_eventchains.size();}
	EventChain & get_eventchain_by_nr(const Index i) const
	{assert(i < m_eventchains.size()); return *m_eventchains[i];}

private:
	eventchain_vector m_eventchains;
};

#endif
