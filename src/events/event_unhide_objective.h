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

#ifndef __S__EVENT_UNHIDE_OBJECTIVE_H
#define __S__EVENT_UNHIDE_OBJECTIVE_H

#include "event.h"

class Editor_Game_Base;
class MapObjective;

/*
 * This event unhides or hides an objective.
 */
struct Event_Unhide_Objective : public Event {
     Event_Unhide_Objective();
      ~Event_Unhide_Objective();

      // one liner functions
	const char * get_id() const {return "unhide_objective";}

      State run(Game*);
      virtual void reinitialize(Game*);

      // File Functions
	void Write(Section &, const Editor_Game_Base &) const;
	void Read (Section *,       Editor_Game_Base *);

	void set_objective(MapObjective * objective) {m_objective = objective;}
	MapObjective * get_objective() {return m_objective;}
	void set_dounhide(bool t) {m_dounhide = t;}
	bool get_dounhide() const {return m_dounhide;}

private:
	MapObjective * m_objective;
	bool           m_dounhide;
};



#endif
