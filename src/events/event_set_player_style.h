/*
 * Copyright (C) 2009 by the Widelands Development Team
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

#ifndef EVENT_SET_PLAYER_STYLE_H
#define EVENT_SET_PLAYER_STYLE_H

#include "event.h"

namespace Widelands {

/// Abstract base for events setting some style for a player.
struct Event_Set_Player_Style : public Event {
	Event_Set_Player_Style
		(char const * const Name, State const S)
		: Event(Name, S), m_player_number(1), m_style_index(0)
	{}
	Event_Set_Player_Style(Section &, Editor_Game_Base &);

	void Write(Section &, Editor_Game_Base &) const;

	Player_Number player_number() const {return m_player_number;}
	void set_player(Player_Number);
	uint8_t style_index() const {return m_style_index;}
	void set_style_index(uint8_t);
protected:
	Player_Number m_player_number;
	uint8_t       m_style_index;
};

}

#endif
