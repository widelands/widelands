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

#include "event_chain.h"

#include "event.h"
#include "logic/game.h"
#include "logic/map.h"
#include "trigger/trigger_conditional.h"
#include "wexception.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"

namespace Widelands {


EventChain::~EventChain() {
	clear_events();
	if (m_trigconditional)
		m_trigconditional->unreference_triggers(*this);
}


EventChain::State EventChain::run(Game & game) {
	m_state = RUNNING;

	while (m_curevent < m_events.size()) {
		if (m_events[m_curevent]->run(game) == Event::DONE)
			++m_curevent;
		else
			break;
	}

	if (m_curevent == m_events.size()) {
		//  Last event has been run. This is finished.
		if (get_repeating()) {
			//  This eventchain will repeat in due time.
			m_curevent = 0;
			m_trigconditional->reset_triggers(game);
			m_state = INIT;
		} else {
			//  This eventchain is completely done.
			m_state = DONE;
		}
	} else {
		assert(m_events[m_curevent]->state() == Event::RUNNING);
	}

	return m_state;
}

/**
 * Clear all events, events are not delted.
 */
void EventChain::clear_events() {
	event_vector & evs = m_events;
	container_iterate_const(event_vector, evs, i)
		(*i.current)->unreference(*this);

	evs.clear();
}

/**
 * Add an event
 */
void EventChain::add_event(Event * const ev) {
	m_events.push_back(ev);
	ev->reference(*this);
}

}
