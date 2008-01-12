/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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
#include "trigger/trigger_referencer.h"

namespace Widelands {

class Editor_Game_Base;

/*
 * This event is able to set a null trigger to a certain state
 */
struct Event_Set_Null_Trigger : public Event, public TriggerReferencer {
     Event_Set_Null_Trigger();
      ~Event_Set_Null_Trigger();

      // one liner functions
	const char * get_id() const {return "set_null_trigger";}

      State run(Game*);
      virtual void reinitialize(Game*);

      // File Functions
	void Write(Section &, const Editor_Game_Base &) const;
	void Read (Section *,       Editor_Game_Base *);

	void set_trigger(Trigger_Null * trig) {
		 if (m_trigger)
			 unreference_trigger(m_trigger);
		 if (trig)
			 reference_trigger(trig);
		 m_trigger = trig;
	}
	Trigger_Null * get_trigger() const {return m_trigger;}
	void set_setto(bool t) {m_setto = t;}
	bool get_setto() const {return m_setto;}

      // For Trigger referencer
	const std::string & name() const {return Event::name();}
	const char* get_type() const {return "Event Set Null Trigger";}

private:
      Trigger_Null* m_trigger;
      bool          m_setto;
};

};

#endif
