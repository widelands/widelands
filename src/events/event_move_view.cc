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

#include "event_move_view.h"

#include "editor_game_base.h"
#include "filesystem.h"
#include "game.h"
#include "i18n.h"
#include "interactive_base.h"
#include "map.h"
#include "profile.h"
#include "wexception.h"

static const int32_t EVENT_VERSION = 2;

/*
 * Init and cleanup
 */
Event_Move_View::Event_Move_View() {
	set_name(_("Move View").c_str());
   set_coords(Coords(0, 0));
}

Event_Move_View::~Event_Move_View() {
}

/*
 * reinitialize
 */
void Event_Move_View::reinitialize(Game *) {}

/*
 * File Read, File Write
 */
void Event_Move_View::Read(Section * s, Editor_Game_Base *) {
	const int32_t packet_version = s->get_safe_int("version");
	if (1 <= packet_version and packet_version <= EVENT_VERSION) {
		m_pt =
			packet_version == 1 ?
			Coords(s->get_safe_int("point_x"), s->get_safe_int("point_y"))
			:
			s->get_safe_Coords("point");
	} else
		throw wexception
			("Move View Event with unknown/unhandled version %i in map!",
			 packet_version);
}

void Event_Move_View::Write(Section & s, const Editor_Game_Base &) const {
	s.set_int   ("version", EVENT_VERSION);
	s.set_Coords("point",   m_pt);
}

/*
 * check if trigger conditions are done
 */
Event::State Event_Move_View::run(Game* game) {
	assert(m_pt);

   Interactive_Base* iab=game->get_iabase();
   iab->move_view_to(m_pt);

   m_state = DONE;
   return m_state;
}
