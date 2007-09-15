/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#ifndef __S__EVENT_SET_VISIBLITY_H
#define __S__EVENT_SET_VISIBLITY_H

#include "event.h"

#include <string>

/**
 * Sets a campaign/campaignmap in Campaign UI visible/invisible
 */
struct Event_Set_Visiblity : public Event {
	Event_Set_Visiblity();
	~Event_Set_Visiblity();

	// one liner functions
	const char * get_id() const {return "set_visiblity";}

	State run(Game*);
	virtual void reinitialize(Game*);

	// fonctions for reading/writing the content of events section
	void Write(Section &, const Editor_Game_Base &) const;
	void Read(Section*, Editor_Game_Base*);

private:
	static const int EVENT_VERSION = 1;

	std::string v_entry;
	int v_case;
};

#endif //__S__EVENT_SET_VISIBLITY_H
