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

#include "event_allow_building.h"

#include "editor_game_base.h"
#include "interactive_base.h"
#include "filesystem.h"
#include "game.h"
#include "i18n.h"
#include "map.h"
#include "player.h"
#include "profile.h"
#include "tribe.h"
#include "wexception.h"

#include "log.h"

#define EVENT_VERSION 2

namespace Widelands {

Event_Allow_Building::Event_Allow_Building
	(char const * const Name, State const S)
	:
	Event     (Name, S),
	m_player  (1),
	m_building(Building_Index::Null()),
	m_allow   (true)
{}


void Event_Allow_Building::Read(Section & s, Editor_Game_Base & egbase) {
	try {
		uint32_t const packet_version = s.get_safe_positive("version");
		if (packet_version <= EVENT_VERSION) {
			m_player   =
				s.get_Player_Number("player", egbase.map().get_nrplayers(), 1);
			egbase.get_iabase()->reference_player_tribe(m_player, this);
			m_building = s.get_safe_Building_Index("building", egbase, m_player);
			m_allow    = s.get_bool("allow", true);
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (std::exception const & e) {
		throw wexception("(allow building): %s", e.what());
	}
}

void Event_Allow_Building::Write(Section & s, Editor_Game_Base & egbase) const
{
	s.set_string        ("type",    "allow_building");
	s.set_int           ("version",  EVENT_VERSION);
	if (m_player != 1)
		s.set_int        ("player",   m_player);
	s.set_Building_Index("building", m_building, egbase, m_player);
	if (not m_allow)
		s.set_bool       ("allow",    false);
}


Event::State Event_Allow_Building::run(Game* game) {
	game->player(m_player).allow_building(m_building, m_allow);
	return m_state = DONE;
}

};
