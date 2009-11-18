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

#include "trigger_defeated.h"

#include "editor/editorinteractive.h"
#include "logic/game.h"
#include "game_data_error.h"
#include "map.h"
#include "logic/player.h"
#include "profile/profile.h"

#define PACKET_VERSION 1

namespace Widelands {

Trigger_Defeated::Trigger_Defeated(char const * const Name, bool const set)
	: Trigger(Name, set), m_player(0)
{}


void Trigger_Defeated::Read(Section & s, Editor_Game_Base & egbase) {
	try {
		uint32_t const packet_version = s.get_safe_positive("version");
		if (packet_version <= PACKET_VERSION) {
			m_player = s.get_safe_int("player");
			Map const & map = egbase.map();
			if (map.get_nrplayers() < m_player)
				throw game_data_error
					(_("player is %u, but map has only %u players"),
					 m_player, map.get_nrplayers());
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("(defeated): %s"), e.what());
	}
}

void Trigger_Defeated::Write(Section & s, Editor_Game_Base const &) const
{
	s.set_string       ("type",     "defeated");
	s.set_int          ("version",  PACKET_VERSION);
	s.set_int          ("player",   m_player);
}

/**
 * Check if trigger conditions are done
 */
void Trigger_Defeated::check_set_conditions(Game const & game) {
	const std::vector<uint32_t> & nr_workers =
		game.get_general_statistics()[m_player - 1].nr_workers;

	if (nr_workers.empty())
		return set_trigger(false);
	if (*nr_workers.rbegin() == 0)
		return set_trigger(true);
	return set_trigger(false);
}

}
