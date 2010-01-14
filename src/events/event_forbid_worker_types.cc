/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "event_forbid_worker_types.h"

#include "economy/economy.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "profile/profile.h"

#include "i18n.h"
#include "log.h"

namespace Widelands {

void Event_Forbid_Worker_Types::Write
	(Section                    & s,
	 Editor_Game_Base     const & egbase,
	 Map_Map_Object_Saver const & mos)
	const
{
	s.set_string("type",  "forbid_worker_types");
	Event_Player_Worker_Types::Write(s, egbase, mos);
}


char const * Event_Forbid_Worker_Types::action_name() const {
	return _("Forbid");
}


Event::State Event_Forbid_Worker_Types::run(Game & game) {
	Player                        & player = game  .player(m_player);
	Tribe_Descr             const & tribe  = player.tribe ();
	std::vector<Ware_Index> const & worker_types_without_cost =
		tribe.worker_types_without_cost();
	container_iterate_const(Worker_Types, m_worker_types, i)
		if (player.is_worker_type_allowed(*i.current)) {
			player.allow_worker_type(*i.current, false);

			Worker_Descr const & worker_descr =
				*tribe.get_worker_descr(*i.current);
			assert(worker_descr.is_buildable());
			if (worker_descr.buildcost().empty()) {
				//  Workers of this type can be spawned in warehouses. Stop it.
				uint8_t worker_types_without_cost_index = 0;
				for (;; ++worker_types_without_cost_index) {
					assert
						(worker_types_without_cost_index
						 <
						 worker_types_without_cost.size());
					if
						(worker_types_without_cost.at
						 	(worker_types_without_cost_index)
						 ==
						 *i.current)
						break;
				}
				for (uint32_t j = player.get_nr_economies(); j;) {
					Economy & economy = *player.get_economy_by_number(--j);
					container_iterate_const
						(std::vector<Warehouse *>, economy.warehouses(), k)
						(*k.current)->disable_spawn(worker_types_without_cost_index);
				}
			}
		}
	return m_state = DONE;
}

}
