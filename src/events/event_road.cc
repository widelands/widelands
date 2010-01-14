/*
 * Copyright (C) 2008, 2010 by the Widelands Development Team
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

#include "event_road.h"

#include "logic/checkstep.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "profile/profile.h"

#define EVENT_VERSION 1

namespace Widelands {

Event_Road::Event_Road(Section & s, Editor_Game_Base & egbase) : Event(s) {
	try {
		uint32_t const packet_version = s.get_safe_positive("version");
		if (packet_version == EVENT_VERSION) {
			Map & map = egbase.map();
			m_path = s.get_safe_Coords("point", map.extent());
			char const * const steps = s.get_safe_string("steps");
			if (not steps[0])
				throw game_data_error(_("there are no steps"));
			if (not steps[1])
				throw game_data_error
					(_("there is only 1 step, must be at least 2"));
			CheckStepLimited cstep;
			for (char const * p = steps; *p; ++p) {
				if (*p < '1' or '6' < *p)
					throw game_data_error
						(_
						 	("step %li has direction '%c', must be one of {1 "
						 	 "(northeast), 2 (east), 3 (southeast), 4 (southwest), 5 "
						 	 "(west), 6 (northwest)}"),
						 static_cast<long int>(p - steps), *p);
				m_path.append(map, *p - '0');
				cstep.add_allowed_location(m_path.get_end());
			}
			Path optimal_path;
			map.findpath
				(m_path.get_start(), m_path.get_end(),
				 0,
				 optimal_path,
				 cstep,
				 Map::fpBidiCost);
			Path::Step_Vector::size_type const nr_steps =
				optimal_path.get_nsteps();
			char optimal_steps[nr_steps + 1];
			for (Path::Step_Vector::size_type i = 0; i < nr_steps; ++i)
				optimal_steps[i] = '0' + optimal_path[i];
			optimal_steps[nr_steps] = '\0';
			if (strcmp(steps, optimal_steps))
				throw game_data_error
					(_
					 	("the steps \"%s\" do not form the optimal path from the "
					 	 "start to the end through only the used locations, should "
					 	 "be \"%s\""),
					 steps, optimal_steps);
			m_player = s.get_Player_Number("player", map.get_nrplayers());
			m_fill   = s.get_bool         ("fill",                        true);
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error("(road): %s", e.what());
	}
}

void Event_Road::Write
	(Section & s, Editor_Game_Base const &, Map_Map_Object_Saver const &) const
{
	s.set_string ("type",    "road");
	s.set_int    ("version", EVENT_VERSION);
	s.set_Coords ("point",   m_path.get_start());
	Path::Step_Vector::size_type const nr_steps = m_path.get_nsteps();
	char steps[nr_steps + 1];
	for (Path::Step_Vector::size_type i = 0; i < nr_steps; ++i)
		steps[i] = '0' + m_path[i];
	steps[nr_steps] = '\0';
	s.set_string ("steps",   steps);
	if (m_player != 1)
		s.set_int ("player",  m_player);
	if (not m_fill)
		s.set_bool("fill",    false);
}


Event::State Event_Road::run(Game & game) {
	game.player(m_player).force_road(m_path, m_fill);
	return m_state = DONE;
}

}
