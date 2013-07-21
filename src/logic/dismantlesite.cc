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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
#include "worker.h"

#include "dismantlesite.h"

namespace Widelands {

DismantleSite_Descr::DismantleSite_Descr
	(char const * const _name, char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 const Tribe_Descr & _tribe)
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


DismantleSite::DismantleSite(const DismantleSite_Descr & gdescr) :
Partially_Finished_Building(gdescr)
{}

DismantleSite::DismantleSite
	(const DismantleSite_Descr & gdescr, Editor_Game_Base & egbase, Coords const c,
	 Player & plr, const Building_Descr & bdscr, bool loading)
:
Partially_Finished_Building(gdescr)
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

	std::map<Ware_Index, uint8_t> wares;
	count_returned_wares(*m_building, wares);

	std::map<Ware_Index, uint8_t>::const_iterator it = wares.begin();
	m_wares.resize(wares.size());

	for (size_t i = 0; i < wares.size(); ++i, ++it) {
		WaresQueue & wq =
			*(m_wares[i] = new WaresQueue(*this, it->first, it->second));

		wq.set_filled(it->second);
		m_work_steps += it->second;
	}
}

/*
===============
Count wich wares you get back if you dismantle the given building
===============
*/
void DismantleSite::count_returned_wares
	(const Widelands::Building_Descr & building,
	 std::map<Ware_Index, uint8_t>   & res)
{
	const Tribe_Descr & t = building.tribe();
	Building_Descr const * bd = &building;
	Building_Index bd_idx = t.building_index(bd->name());

	bool done = false;
	while (not done) {
		const std::map<Ware_Index, uint8_t> & buildcost = bd->buildcost();
		for (std::map<Ware_Index, uint8_t>::const_iterator i = buildcost.begin(); i != buildcost.end(); ++i)
			res[i->first] += i->second;

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

	for (std::map<Ware_Index, uint8_t>::iterator it = res.begin(); it != res.end(); ++it) {
		it->second = (it->second + RATIO_RETURNED_WARES - 1) / RATIO_RETURNED_WARES;
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

			const Item_Ware_Descr & wd = *tribe().get_ware_descr(wq.get_ware());
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
	(const Editor_Game_Base& game, RenderTarget& dst, const FCoords& coords, const Point& pos)
{
	assert(0 <= game.get_gametime());
	const uint32_t gametime = game.get_gametime();
	uint32_t tanim = gametime - m_animstart;

	if (coords != m_position)
		return; // draw big buildings only once

	// Draw the construction site marker
	dst.drawanim(pos, m_anim, tanim, get_owner());

	// Draw the partially dismantled building
	static_assert(0 <= DISMANTLESITE_STEP_TIME, "assert(0 <= DISMANTLESITE_STEP_TIME) failed.");
	uint32_t total_time = DISMANTLESITE_STEP_TIME * m_work_steps;
	uint32_t completed_time = DISMANTLESITE_STEP_TIME * m_work_completed;

	if (m_working)
		completed_time += DISMANTLESITE_STEP_TIME + gametime - m_work_steptime;

	uint32_t anim_idx;
	try {
		anim_idx = m_building->get_animation("unoccupied");
	} catch (Map_Object_Descr::Animation_Nonexistent &) {
		anim_idx = m_building->get_animation("idle");
	}
	const Animation& anim = g_gr->animations().get_animation(anim_idx);
	const uint16_t w = anim.width();
	const uint16_t h = anim.height();

	uint32_t lines = h * completed_time / total_time;

	dst.drawanimrect(pos, anim_idx, tanim, get_owner(), Rect(Point(0, lines), w, h - lines));

	// Draw help strings
	draw_help(game, dst, coords, pos);
}

}
