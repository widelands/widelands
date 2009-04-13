/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "trigger_military_influence.h"

#include "game.h"
#include "player.h"
#include "profile.h"
#include "wexception.h"

#define PACKET_VERSION 3

namespace Widelands {

Trigger_Military_Influence::Trigger_Military_Influence
	(char const * const Name, bool const set)
: Trigger_Player_Area(Name, set)
{}


void Trigger_Military_Influence::Read(Section & s, Editor_Game_Base & egbase) {
	try {
		uint32_t const packet_version = s.get_safe_positive("version");
		if (packet_version <= PACKET_VERSION) {
			Trigger_Player_Area::Read(s, egbase);
			require_highest = s.get_bool("require_highest", false);
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (std::exception const & e) {
		throw wexception("(military influence): %s", e.what());
	}
}

void Trigger_Military_Influence::Write
	(Section & s, Editor_Game_Base const & egbase) const
{
	s.set_string ("type",            "military_influence");
	s.set_int    ("version",         PACKET_VERSION);
	Trigger_Player_Area::Write(s, egbase);
	if (require_highest)
		s.set_bool("require_highest", true);
}

void Trigger_Military_Influence::check_set_conditions(Game const & game) {
	Map const & map = game.map();

	X_Coordinate  const mapwidth   = map.get_width();
	Player_Number const nr_players = map.get_nrplayers();
	Player const & player = game.player(m_player_area.player_number);
	uint32_t count = 0;
	MapRegion<Area<FCoords> > mr (map, m_player_area);
	if (require_highest)
		do {
			Map_Index const index = Map::get_index(mr.location(), mapwidth);
			if
				(Military_Influence const p_military_influence =
				 player.military_influence(index))
			{
				iterate_players_existing(other_p, nr_players, game, other)
					if (p_military_influence < other->military_influence(index))
						goto next_location;
				if (m_count <= ++count)
					return set_trigger(true);
			} next_location:;
		} while (mr.advance(map));
	else
		do {
			Map_Index const index = Map::get_index(mr.location(), mapwidth);
			if (player.military_influence(index))
				if (m_count <= ++count)
					return set_trigger(true);
		} while (mr.advance(map));
}

};
