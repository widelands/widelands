/*
 * Copyright (C) 2009-2010 by the Widelands Development Team
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

#ifndef EVENT_SET_PLAYER_FRONTIER_STYLE_H
#define EVENT_SET_PLAYER_FRONTIER_STYLE_H

#include "event_set_player_style.h"

namespace Widelands {

struct Event_Set_Player_Frontier_Style : public Event_Set_Player_Style {
	Event_Set_Player_Frontier_Style(char const * const Name, State const S) :
		Event_Set_Player_Style(Name, S)
	{}
	Event_Set_Player_Frontier_Style(Section &, Editor_Game_Base &);

	int32_t option_menu(Editor_Interactive &);

	void Write
		(Section &, Editor_Game_Base const &, Map_Map_Object_Saver const &)
		const;

	State run(Game &);
};

}

#endif
