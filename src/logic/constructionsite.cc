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

#include "editor_game_base.h"
#include "i18n.h"
#include "upcast.h"
#include "wexception.h"

#include "ui_basic/object.h" //only needed for i18n function _()
#include "graphic/animation.h"
#include "economy/wares_queue.h"
#include "game.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "worker.h"

#include "constructionsite.h"

namespace Widelands {

ConstructionSite_Descr::ConstructionSite_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const & _tribe)
: Building_Descr(_name, _descname, directory, prof, global_s, _tribe)
{
	add_attribute(Map_Object::CONSTRUCTIONSITE);

	{ // animation when a worker entered the site
		Section & sec = prof.get_safe_section("idle_with_worker");
		if (!is_animation_known("idle_with_worker"))
			add_animation
				("idle_with_worker",
				 g_anim.get(directory.c_str(), sec, 0));
	}
}


Building & ConstructionSite_Descr::create_object() const {
	return *new ConstructionSite(*this);
}


/*
==============================

IMPLEMENTATION

==============================
*/


ConstructionSite::ConstructionSite(const ConstructionSite_Descr & cs_descr) :
Partially_Finished_Building (cs_descr),
m_prev_building  (0),
m_fetchfromflag  (0),
m_builder_idle   (false),
m_info           (new Player::Constructionsite_Information)
{}


/*
===============
Print completion percentage.
===============
*/
std::string ConstructionSite::get_statistics_string()
{
	char buffer[40];
	snprintf
		(buffer, sizeof(buffer),
		 _("%u%% built"), (get_built_per64k() * 100) >> 16);
	return buffer;
}

/*
=======
Access to the wares queues by id
=======
*/
WaresQueue & ConstructionSite::waresqueue(Ware_Index const wi) {
	container_iterate_const(Wares, m_wares, i)
		if ((*i.current)->get_ware() == wi)
			return **i.current;
	throw wexception
		("%s (%u) (building %s) has no WaresQueue for %u",
		 name().c_str(), serial(), m_building->name().c_str(), wi.value());
}


/*
===============
Set the type of building we're going to build
===============
*/
void ConstructionSite::set_building(const Building_Descr & building_descr) {
	Partially_Finished_Building::set_building(building_descr);

	m_info->becomes = &building_descr;
}

/*
 * Set previous building
 * That is the building that was here before, we're
 * an enhancement
 */
void ConstructionSite::set_previous_building
	(Building_Descr const * const previous_building_descr)
{
	assert(!m_prev_building);

	m_prev_building = previous_building_descr;
	m_info->was = previous_building_descr;
}

/*
===============
Initialize the construction site by starting orders
===============
*/
void ConstructionSite::init(Editor_Game_Base & egbase)
{
	Partially_Finished_Building::init(egbase);

	//  TODO figure out whether planing is necessary

	//  initialize the wares queues
	std::map<Ware_Index, uint8_t> const & buildcost = m_building->buildcost();
	size_t const buildcost_size = buildcost.size();
	m_wares.resize(buildcost_size);
	std::map<Ware_Index, uint8_t>::const_iterator it = buildcost.begin();

	for (size_t i = 0; i < buildcost_size; ++i, ++it) {
		WaresQueue & wq =
			*(m_wares[i] = new WaresQueue(*this, it->first, it->second));

		wq.set_callback(ConstructionSite::wares_queue_callback, this);
		wq.set_consume_interval(CONSTRUCTIONSITE_STEP_TIME);

		m_work_steps += it->second;
	}
}


/*
===============
Release worker and material (if any is left).
If construction was finished successfully, place the building at our position.
===============
*/
void ConstructionSite::cleanup(Editor_Game_Base & egbase)
{
	Partially_Finished_Building::cleanup(egbase);

	if (m_work_steps <= m_work_completed) {
		// Put the real building in place
		Building & building =
			m_building->create(egbase, owner(), m_position, false);
		if (Worker * const builder = m_builder.get(egbase)) {
			builder->reset_tasks(ref_cast<Game, Editor_Game_Base>(egbase));
			builder->set_location(&building);
		}
	}
}


/*
===============
Construction sites only burn if some of the work has been completed.
===============
*/
bool ConstructionSite::burn_on_destroy()
{
	if (m_work_completed >= m_work_steps)
		return false; // completed, so don't burn

	return m_work_completed or m_prev_building;
}

/*
===============
Remember the item on the flag. The worker will be sent from get_building_work().
===============
*/
bool ConstructionSite::fetch_from_flag(Game & game)
{
	++m_fetchfromflag;

	if (Worker * const builder = m_builder.get(game))
		builder->update_task_buildingwork(game);

	return true;
}


/*
===============
Called by our builder to get instructions.
===============
*/
bool ConstructionSite::get_building_work(Game & game, Worker & worker, bool) {
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
	if (m_working) {
		if (static_cast<int32_t>(game.get_gametime() - m_work_steptime) < 0) {
			worker.start_task_idle
				(game,
				 worker.get_animation("work"),
				 m_work_steptime - game.get_gametime());
			m_builder_idle = false;
			return true;
		} else {
			//TODO(fweber): cause "construction sounds" to be played -
			//perhaps dependent on kind of construction?

			++m_work_completed;
			if (m_work_completed >= m_work_steps)
				schedule_destroy(game);

			m_working = false;
		}
	}

	// Fetch items from flag
	if (m_fetchfromflag) {
		--m_fetchfromflag;
		m_builder_idle = false;
		worker.start_task_fetchfromflag(game);
		return true;
	}

	// Drop all the wares that are too much out to the flag.
	container_iterate(Wares, m_wares, iqueue) {
		WaresQueue * queue = *iqueue;
		if (queue->get_filled() > queue->get_max_fill()) {
			queue->set_filled(queue->get_filled() - 1);
			Item_Ware_Descr const & wd = *tribe().get_ware_descr(queue->get_ware());
			WareInstance & item = *new WareInstance(queue->get_ware(), &wd);
			item.init(game);
			worker.start_task_dropoff(game, item);
			return true;
		}
	}

	// Check if we've got wares to consume
	if (m_work_completed < m_work_steps)
	{
		for (uint32_t i = 0; i < m_wares.size(); ++i) {
			WaresQueue & wq = *m_wares[i];

			if (!wq.get_filled())
				continue;

			wq.set_filled(wq.get_filled() - 1);
			wq.set_max_size(wq.get_max_size() - 1);

			//update consumption statistic
			owner().ware_consumed(wq.get_ware(), 1);

			m_working = true;
			m_work_steptime = game.get_gametime() + CONSTRUCTIONSITE_STEP_TIME;

			worker.start_task_idle
				(game, worker.get_animation("work"), CONSTRUCTIONSITE_STEP_TIME);
			m_builder_idle = false;
			return true;
		}
	}
	// The only work we have got for you, is to run around to look cute ;)
	if (!m_builder_idle) {
		worker.set_animation(game, worker.get_animation("idle"));
		m_builder_idle = true;
	}
	worker.schedule_act(game, 2000);
	return true;
}


/*
===============
Called by WaresQueue code when an item has arrived
===============
*/
void ConstructionSite::wares_queue_callback
	(Game & game, WaresQueue *, Ware_Index, void * const data)
{
	ConstructionSite & cs = *static_cast<ConstructionSite *>(data);

	if (!cs.m_working)
		if (Worker * const builder = cs.m_builder.get(game))
			builder->update_task_buildingwork(game);
}


/*
===============
Draw the construction site.
===============
*/
void ConstructionSite::draw
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

	// Draw the partially finished building

	compile_assert(0 <= CONSTRUCTIONSITE_STEP_TIME);
	m_info->totaltime = CONSTRUCTIONSITE_STEP_TIME * m_work_steps;
	m_info->completedtime = CONSTRUCTIONSITE_STEP_TIME * m_work_completed;

	if (m_working) {
		assert
			(m_work_steptime
			 <=
			 m_info->completedtime + CONSTRUCTIONSITE_STEP_TIME + gametime);
		m_info->completedtime += CONSTRUCTIONSITE_STEP_TIME + gametime - m_work_steptime;
	}

	uint32_t anim;
	uint32_t cur_frame;
	try {
		anim = building().get_animation("build");
	} catch (Map_Object_Descr::Animation_Nonexistent) {
		try {
			anim = building().get_animation("unoccupied");
		} catch (Map_Object_Descr::Animation_Nonexistent) {
			anim = building().get_animation("idle");
		}
	}
	const AnimationGfx::Index nr_frames = g_gr->nr_frames(anim);
	cur_frame = m_info->totaltime ? m_info->completedtime * nr_frames / m_info->totaltime : 0;
	// Redefine tanim
	tanim = cur_frame * FRAME_LENGTH;

	uint32_t w, h;
	g_gr->get_animation_size(anim, tanim, w, h);

	uint32_t lines = h * m_info->completedtime * nr_frames;
	if (m_info->totaltime)
		lines /= m_info->totaltime;
	assert(h * cur_frame <= lines);
	lines -= h * cur_frame; //  This won't work if pictures have various sizes.

	if (cur_frame) //  not the first pic
		//  draw the prev pic from top to where next image will be drawing
		dst.drawanimrect(pos, anim, tanim - FRAME_LENGTH, get_owner(), Rect(Point(0, 0), w, h - lines));
	else if (m_prev_building) {
		//  Is the first picture but there was another building here before,
		//  get its most fitting picture and draw it instead.
		uint32_t a;
		try {
			a = m_prev_building->get_animation("unoccupied");
		} catch (Map_Object_Descr::Animation_Nonexistent) {
			a = m_prev_building->get_animation("idle");
		}
		uint32_t wa, ha;
		g_gr->get_animation_size(a, tanim, wa, ha);
		dst.drawanimrect
			(pos, a, tanim - FRAME_LENGTH, get_owner(),
			 Rect(Point(0, 0), w, std::min(ha, h - lines)));
	}

	assert(lines <= h);
	dst.drawanimrect(pos, anim, tanim, get_owner(), Rect(Point(0, h - lines), w, lines));

	// Draw help strings
	draw_help(game, dst, coords, pos);
}

}
