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

#ifndef __S__EVENT_CHAIN_H
#define __S__EVENT_CHAIN_H

#include "event_referencer.h"
#include "trigger/trigger_referencer.h"

#include <cassert>
#include <string>
#include <vector>

namespace Widelands {

class Event;
class Game;
class TriggerConditional;

/*
 * The Chain defined
 *
 * A eventchain is a succession of events, which are run
 * in order as soon as the eventchains trigger conditional is
 * true.
 */
struct EventChain : public EventReferencer, public TriggerReferencer {
	friend struct Map_EventChain_Data_Packet;

	enum State {
         INIT,
         RUNNING,
         DONE,
	};

      EventChain() {
        m_repeating = false;
        m_trigconditional = 0;
        m_state = INIT;
        m_curevent = 0;
		}
      virtual ~EventChain() {}

	const std::string & name() const throw () {return m_name;}
	const char * get_type() const {return "EventChain";}
	void set_name(const std::string & new_name) {m_name = new_name;}
	bool get_repeating() const {return m_repeating;}
	TriggerConditional * get_trigcond() {return m_trigconditional;}
	void set_trigcond(TriggerConditional * t) {m_trigconditional = t;}

	State get_state() const {return m_state;}

      // Below is only good idea in editor
      void set_repeating(bool t) {m_repeating = t;}
      uint32_t get_nr_events() {return m_events.size();}
      Event* get_event(uint32_t i) {assert(i < m_events.size()); return m_events[i];}
      void clear_events();
      void add_event(Event* ev);


      // Run this event queue
      State run(Game* g);

private:
      std::string        m_name;
      bool                m_repeating;
      TriggerConditional* m_trigconditional;
	typedef std::vector<Event*> event_vector;
	event_vector m_events;

      // For running
      uint32_t                m_curevent;
      State               m_state;

};

};

#endif
