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

#include "logic/dismantlesite.h"

#include <cstdio>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/wares_queue.h"
#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/tribes/tribe_descr.h"
#include "logic/worker.h"
#include "sound/sound_handler.h"

namespace Widelands {

DismantleSiteDescr::DismantleSiteDescr(const std::string& init_descname,
													const LuaTable& table, const EditorGameBase& egbase)
	: BuildingDescr(init_descname, MapObjectType::DISMANTLESITE, table, egbase)
{
	add_attribute(MapObject::Attribute::CONSTRUCTIONSITE); // Yep, this is correct.
}

Building & DismantleSiteDescr::create_object() const {
	return *new DismantleSite(*this);
}

/*
==============================

IMPLEMENTATION

==============================
*/


DismantleSite::DismantleSite(const DismantleSiteDescr & gdescr) :
PartiallyFinishedBuilding(gdescr)
{}

DismantleSite::DismantleSite
	(const DismantleSiteDescr & gdescr, EditorGameBase & egbase, Coords const c,
	 Player & plr, bool loading, Building::FormerBuildings & former_buildings)
:
PartiallyFinishedBuilding(gdescr)
{
	m_position = c;
	set_owner(&plr);

	assert(!former_buildings.empty());
	for (BuildingIndex former_idx : former_buildings) {
		m_old_buildings.push_back(former_idx);
	}
	const BuildingDescr* cur_descr = owner().tribe().get_building_descr(m_old_buildings.back());
	set_building(*cur_descr);

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
void DismantleSite::update_statistics_string(std::string* s)
{
	unsigned int percent = (get_built_per64k() * 100) >> 16;
	*s = (boost::format(_("%u%% dismantled")) % percent).str();
}

/*
===============
Initialize the construction site by starting orders
===============
*/
void DismantleSite::init(EditorGameBase & egbase)
{
	PartiallyFinishedBuilding::init(egbase);

	std::map<DescriptionIndex, uint8_t> wares;
	count_returned_wares(this, wares);

	std::map<DescriptionIndex, uint8_t>::const_iterator it = wares.begin();
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
	(Building* building,
	 std::map<DescriptionIndex, uint8_t>   & res)
{
	for (BuildingIndex former_idx : building->get_former_buildings()) {
		const std::map<DescriptionIndex, uint8_t> * return_wares;
		const BuildingDescr* former_descr = building->owner().tribe().get_building_descr(former_idx);
		if (former_idx != building->get_former_buildings().front()) {
			return_wares = & former_descr->returned_wares_enhanced();
		} else {
			return_wares = & former_descr->returned_wares();
		}
		assert(return_wares != nullptr);

		std::map<DescriptionIndex, uint8_t>::const_iterator i;
		for (i = return_wares->begin(); i != return_wares->end(); ++i) {
			res[i->first] += i->second;
		}
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

	if (!m_work_steps) //  Happens for building without buildcost.
		schedule_destroy(game); //  Complete the building immediately.

	// Check if one step has completed
	if (static_cast<int32_t>(game.get_gametime() - m_work_steptime) >= 0 && m_working) {
		++m_work_completed;

		for (uint32_t i = 0; i < m_wares.size(); ++i) {
			WaresQueue & wq = *m_wares[i];

			if (!wq.get_filled())
				continue;

			wq.set_filled(wq.get_filled() - 1);
			wq.set_max_size(wq.get_max_size() - 1);

			//update statistics
			owner().ware_produced(wq.get_ware());

			const WareDescr & wd = *owner().tribe().get_ware_descr(wq.get_ware());
			WareInstance & ware = *new WareInstance(wq.get_ware(), &wd);
			ware.init(game);
			worker.start_task_dropoff(game, ware);

			m_working = false;
			return true;
		}
	}

	if (m_work_completed >= m_work_steps) {
		schedule_destroy(game);

		worker.pop_task(game);
		// No more building, so move to the flag
		worker.start_task_move
				(game,
				 WALK_SE,
				 worker.descr().get_right_walk_anims(false),
				 true);
		worker.set_location(nullptr);
	} else if (!m_working) {
		m_work_steptime = game.get_gametime() + DISMANTLESITE_STEP_TIME;
		worker.start_task_idle
			(game, worker.descr().get_animation("work"), DISMANTLESITE_STEP_TIME);

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
	(const EditorGameBase& game, RenderTarget& dst, const FCoords& coords, const Point& pos)
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
	} catch (MapObjectDescr::AnimationNonexistent &) {
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
