/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#ifndef EVENT_CHAIN_H
#define EVENT_CHAIN_H

#include "named.h"
#include "referencer.h"

#include <cassert>
#include <stdint.h>
#include <cstring>
#include <string>
#include <vector>


namespace Widelands {

struct Event;
struct Game;
struct Trigger;
struct TriggerConditional;

/**
 * The Chain defined
 *
 * A eventchain is a succession of events, which are run
 * in order as soon as the eventchains trigger conditional is
 * true.
 */
struct EventChain
	: public Named, public Referencer<Event>, public Referencer<Trigger>
{
	friend struct Map_EventChain_Data_Packet;

	enum State {
		INIT,
		RUNNING,
		DONE,
	};

	EventChain(char const * const Name)
		:
		Named            (Name),
		m_repeating      (false),
		m_trigconditional(0),
		m_curevent       (0),
		m_state          (INIT)
	{}
	virtual ~EventChain();

	std::string identifier() const {return "EventChain: " + name();}

	bool get_repeating() const {return m_repeating;}
	TriggerConditional * get_trigcond() {return m_trigconditional;}
	void set_trigcond(TriggerConditional & t) {m_trigconditional = &t;}

	State get_state() const {return m_state;}

	//  Below is only good idea in editor.
	void set_repeating(bool const t) {m_repeating = t;}
	uint32_t get_nr_events() {return m_events.size();}
	Event * get_event(uint32_t const i) {
		assert(i < m_events.size()); return m_events[i];
	}
	void clear_events();
	void add_event(Event *);


	State run(Game &);

private:
	bool                         m_repeating;
	TriggerConditional         * m_trigconditional;
	typedef std::vector<Event *> event_vector;
	event_vector m_events;

	// For running
	uint32_t                     m_curevent;
	State                        m_state;

};

}

#endif
