/*
 * Copyright (C) 2002-2004, 2006-2009, 2011 by the Widelands Development Team
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

#include <cstdio>

#include "upcast.h"
#include "wexception.h"

#include "economy/wares_queue.h"
#include "editor_game_base.h"
#include "game.h"
#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "i18n.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "ui_basic/object.h" //only needed for i18n function _()
#include "worker.h"

#include "dismantlesite.h"

namespace Widelands {

DismantleSite_Descr::DismantleSite_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const & _tribe)
: Building_Descr(_name, _descname, directory, prof, global_s, _tribe)
{
	add_attribute(Map_Object::CONSTRUCTIONSITE); // Yep, this is correct.
}

Building & DismantleSite_Descr::create_object() const {
	return *new DismantleSite(*this);
}

/*
==============================

IMPLEMENTATION

==============================
*/


DismantleSite::DismantleSite(const DismantleSite_Descr & descr) :
Partially_Finished_Building(descr)
{}

DismantleSite::DismantleSite
	(const DismantleSite_Descr & descr, Editor_Game_Base & egbase, Coords const c,
	 Player & plr, const Building_Descr & bdscr, bool loading)
:
Partially_Finished_Building(descr)
{
	set_building(bdscr);

	m_position = c;
	set_owner(&plr);

	if (loading) {
		Building::init(egbase);
	} else {
		init(egbase);
	}
}

/*
===============
Print completion percentage.
===============
*/
std::string DismantleSite::get_statistics_string()
{
	char buffer[40];
	snprintf
		(buffer, sizeof(buffer),
		 _("%u%% dismantled"), (get_built_per64k() * 100) >> 16);
	return buffer;
}

/*
===============
Initialize the construction site by starting orders
===============
*/
void DismantleSite::init(Editor_Game_Base & egbase)
{
	Partially_Finished_Building::init(egbase);

	Tribe_Descr const & t = tribe();
	Building_Descr const * bd = m_building;
	Building_Index bd_idx = t.building_index(bd->name());

	std::map<Ware_Index, uint8_t> all_costs;
	bool done = false;
	while (not done) {
		std::map<Ware_Index, uint8_t> const & buildcost = bd->buildcost();
		for (std::map<Ware_Index, uint8_t>::const_iterator i = buildcost.begin(); i != buildcost.end(); ++i)
			all_costs[i->first] += i->second;

		// Find the (first) predecessor of this building
		for (Building_Index i = Building_Index::First(); i < t.get_nrbuildings(); ++i) {
			Building_Descr const * ob = t.get_building_descr(i);
			if (ob->enhancements().count(bd_idx)) {
				done = false;
				bd = ob;
				bd_idx = i;
				break;
			} else
				done = true;
		}
	}

	std::map<Ware_Index, uint8_t>::const_iterator it = all_costs.begin();

	m_wares.resize(all_costs.size());

	for (size_t i = 0; i < all_costs.size(); ++i, ++it) {
		uint8_t nwares = (it->second + RATIO_RETURNED_WARES - 1) / RATIO_RETURNED_WARES;
		WaresQueue & wq =
			*(m_wares[i] = new WaresQueue(*this, it->first, nwares));

		wq.set_filled(nwares);
		m_work_steps += nwares;
	}
}


/*
===============
Construction sites only burn if some of the work has been completed.
===============
*/
bool DismantleSite::burn_on_destroy()
{
	if (m_work_completed >= m_work_steps)
		return false; // completed, so don't burn

	return true;
}

/*
===============
Called by our builder to get instructions.
===============
*/
bool DismantleSite::get_building_work(Game & game, Worker & worker, bool) {
	if (&worker != m_builder.get(game)) {
		// Not our construction worker; e.g. a miner leaving a mine
		// that is supposed to be enhanced. Make him return to a warehouse
		worker.pop_task(game);
		worker.start_task_leavebuilding(game, true);
		return true;
	}

	if (not m_work_steps) //  Happens for building without buildcost.
		schedule_destroy(game); //  Complete the building immediately.

	// Check if one step has completed
	if (static_cast<int32_t>(game.get_gametime() - m_work_steptime) >= 0 and m_working) {
		++m_work_completed;

		for (uint32_t i = 0; i < m_wares.size(); ++i) {
			WaresQueue & wq = *m_wares[i];

			if (!wq.get_filled())
				continue;

			wq.set_filled(wq.get_filled() - 1);
			wq.set_max_size(wq.get_max_size() - 1);

			//update statistics
			owner().ware_produced(wq.get_ware());

			Item_Ware_Descr const & wd = *tribe().get_ware_descr(wq.get_ware());
			WareInstance & item = *new WareInstance(wq.get_ware(), &wd);
			item.init(game);
			worker.start_task_dropoff(game, item);

			m_working = false;
			return true;
		}
	}

	if (m_work_completed >= m_work_steps) {
		schedule_destroy(game);

		worker.pop_task(game);
		worker.start_task_leavebuilding(game, true);
	} else if (not m_working) {
		m_work_steptime = game.get_gametime() + DISMANTLESITE_STEP_TIME;
		worker.start_task_idle
			(game, worker.get_animation("work"), DISMANTLESITE_STEP_TIME);

		m_working = true;
	}
	return true;
}

/*
===============
Draw it.
===============
*/
void DismantleSite::draw
	(Editor_Game_Base const & game,
	 RenderTarget           & dst,
	 FCoords          const   coords,
	 Point            const   pos)
{
	assert(0 <= game.get_gametime());
	const uint32_t gametime = game.get_gametime();
	uint32_t tanim = gametime - m_animstart;

	if (coords != m_position)
		return; // draw big buildings only once

	// Draw the construction site marker
	dst.drawanim(pos, m_anim, tanim, get_owner());

	// Draw the partially dismantled building
	compile_assert(0 <= DISMANTLESITE_STEP_TIME);
	uint32_t total_time = DISMANTLESITE_STEP_TIME * m_work_steps;
	uint32_t completed_time = DISMANTLESITE_STEP_TIME * m_work_completed;

	if (m_working)
		completed_time += DISMANTLESITE_STEP_TIME + gametime - m_work_steptime;

	uint32_t anim;
	try {
		anim = m_building->get_animation("unoccupied");
	} catch (Map_Object_Descr::Animation_Nonexistent) {
		anim = m_building->get_animation("idle");
	}
	uint32_t w, h;
	g_gr->get_animation_size(anim, tanim, w, h);

	uint32_t lines = h * completed_time / total_time;

	dst.drawanimrect(pos, anim, tanim, get_owner(), Rect(Point(0, lines), w, h - lines));

	// Draw help strings
	draw_help(game, dst, coords, pos);
}

}
