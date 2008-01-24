/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef __S__EVENT_SET_NULL_TRIGGER_H
#define __S__EVENT_SET_NULL_TRIGGER_H

#include "event.h"
#include "trigger/trigger_null.h"

namespace Widelands {

class Editor_Game_Base;

/*
 * This event is able to set a null trigger to a certain state
 */
struct Event_Set_Null_Trigger : public Event, public Referencer<Trigger> {
	Event_Set_Null_Trigger(char const * name, State);
	~Event_Set_Null_Trigger();

	std::string identifier() const {
		return "Event (set null trigger): " + name();
	}

	int32_t option_menu(Editor_Interactive &);

      State run(Game*);

	void Read (Section &, Editor_Game_Base &);
	void Write(Section &) const;

	void set_trigger(Trigger_Null * const new_trigger) {
		if (new_trigger != m_trigger) {
			if   (m_trigger) Referencer<Trigger>::unreference  (*m_trigger);
			if (new_trigger) Referencer<Trigger>::  reference(*new_trigger);
			m_trigger = new_trigger;
		}
	}
	Trigger_Null * get_trigger() const {return m_trigger;}
	void set_setto(bool t) {m_setto = t;}
	bool get_setto() const {return m_setto;}

private:
      Trigger_Null* m_trigger;
      bool          m_setto;
};

};

#endif
