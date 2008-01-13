/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "event_allow_building.h"

#include "editor_game_base.h"
#include "editorinteractive.h"
#include "filesystem.h"
#include "game.h"
#include "i18n.h"
#include "map.h"
#include "player.h"
#include "profile.h"
#include "tribe.h"
#include "wexception.h"

#include "log.h"

namespace Widelands {

static const int32_t EVENT_VERSION = 1;

/*
 * Init and cleanup
 */
Event_Allow_Building::Event_Allow_Building() : Event(_("Allow Building")) {
   set_player(0);
   set_building("<undefined>");
   set_allow(true);
}

Event_Allow_Building::~Event_Allow_Building() {
}

/*
 * reinitialize
 */
void Event_Allow_Building::reinitialize(Game *) {}

/*
 * File Read, File Write
 */
void Event_Allow_Building::Read(Section* s, Editor_Game_Base* egbase) {
	const int32_t packet_version = s->get_safe_int("version");
	if (packet_version == EVENT_VERSION) {
      int32_t player= s->get_safe_int("player");
      set_player(player);
      set_building(s->get_safe_string("building"));
      set_allow(s->get_safe_bool("allow"));

		if
			(player <= 0
			 ||
			 player>egbase->map().get_nrplayers()
			 ||
			 m_building == "<undefined>")
		{
         // give a warning
         log("Conquer Area Event with illegal player orbuilding name: (Player: %i, Building: %s) deleted!\n", m_player, m_building.c_str());
		}
		egbase->get_iabase()->reference_player_tribe(player, this);
	} else
		throw wexception
			("Allow Building Event with unknown/unhandled version %i in map!",
			 packet_version);
}

void Event_Allow_Building::Write
(Section & s, const Editor_Game_Base &) const
{
	s.set_int   ("version",  EVENT_VERSION);
	s.set_int   ("player",   get_player());
	s.set_string("building", m_building.c_str());
	s.set_bool  ("allow",    m_allow);
}

/*
 * run the event
 */
Event::State Event_Allow_Building::run(Game* game) {
   assert(m_player>0 && m_player<=game->get_map()->get_nrplayers());

   Player* plr=game->get_player(m_player);

   assert(plr);

	const Tribe_Descr & tribe = plr->tribe();
   int32_t index=tribe.get_building_index(m_building.c_str());
	if (index==-1)
		throw wexception
			("Event Allow Building asks for building: %s, which doesn't exist in "
			 "tribe %s",
			 m_building.c_str(), tribe.name().c_str());

   plr->allow_building(index, m_allow);

   m_state = DONE;
   return m_state;
}

};
