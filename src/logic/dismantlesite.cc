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

// TODO SirVer: other tribes have no dismantlesite

#include <cstdio>

#include "upcast.h"
#include "wexception.h"

#include "economy/request.h"
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

#define DISMANTLESITE_STEP_TIME 45000
#define RATIO_RETURNED_WARES 2  // you get half the wares back

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
Building         (descr),
m_building       (0),
m_builder_request(0),
m_work_completed (0),
m_work_steps     (0),
m_work_steptime  (0)
{}


/*
===============
Override: always the same size as the building
===============
*/
int32_t DismantleSite::get_size() const throw () {
	return m_building->get_size();
}

/*
===============
Override: Even though construction sites cannot be built themselves, you can
bulldoze them.
===============
*/
uint32_t DismantleSite::get_playercaps() const throw () {
	uint32_t caps = Building::get_playercaps();

	caps |= 1 << PCap_Bulldoze;

	return caps;
}

/*
===============
Return the animation for the building that is in construction, as this
should be more useful to the player.
===============
*/
uint32_t DismantleSite::get_ui_anim() const
{
	return m_building->get_animation("idle");
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
	snprintf(buffer, sizeof(buffer), "Working");
	return buffer;
}
/*
===============
Return the completion "percentage", where 2^16 = completely built,
0 = nothing built.
===============
*/
// SirVer TODO: should take the gametime
uint32_t DismantleSite::get_built_per64k() const
{
	const uint32_t time = owner().egbase().get_gametime();
	uint32_t thisstep = 0;

	if (is_working()) {
		thisstep = DISMANTLESITE_STEP_TIME - (m_work_steptime - time);
		// The check below is necessary because we drive construction via
		// the construction worker in get_building_work(), and there can be
		// a small delay between the worker completing his job and requesting
		// new work.
		if (thisstep > DISMANTLESITE_STEP_TIME)
			thisstep = DISMANTLESITE_STEP_TIME;
	}

	thisstep = (thisstep << 16) / DISMANTLESITE_STEP_TIME;
	uint32_t total = (thisstep + (m_work_completed << 16));
	if (m_work_steps)
		total /= m_work_steps;

	assert(total <= (1 << 16));

	return total;
}




/*
===============
Set the type of building we're going to build
===============
*/
void DismantleSite::set_building(const Building_Descr & building_descr) {
	assert(!m_building);

	m_building = &building_descr;
}

/*
===============
Initialize the construction site by starting orders
===============
*/
void DismantleSite::init(Editor_Game_Base & egbase)
{
	Building::init(egbase);

	std::map<Ware_Index, uint8_t> const & buildcost = m_building->buildcost();
	size_t const buildcost_size = buildcost.size();
	m_wares.resize(buildcost_size);
	std::map<Ware_Index, uint8_t>::const_iterator it = buildcost.begin();

	for (size_t i = 0; i < buildcost_size; ++i, ++it) {
		uint8_t nwares = (it->second + RATIO_RETURNED_WARES - 1) / RATIO_RETURNED_WARES;
		WaresQueue & wq =
			*(m_wares[i] = new WaresQueue(*this, it->first, nwares));

		wq.set_filled(nwares);
		m_work_steps += nwares;
	}

	if (upcast(Game, game, &egbase))
		request_builder(*game);

	g_sound_handler.play_fx("create_construction_site", m_position, 255);
}


/*
===============
Release worker and material (if any is left).
If construction was finished successfully, place the building at our position.
===============
*/
void DismantleSite::cleanup(Editor_Game_Base & egbase)
{
	// Release worker
	if (m_builder_request) {
		delete m_builder_request;
		m_builder_request = 0;
	}

	Building::cleanup(egbase);
}

/*
===============
Issue a request for the builder.
===============
*/
void DismantleSite::request_builder(Game &) {
	assert(!m_builder.is_set() && !m_builder_request);

	m_builder_request =
		new Request
			(*this,
			 tribe().safe_worker_index("builder"),
			 DismantleSite::request_builder_callback,
			 Request::WORKER);
}


/*
===============
Called by transfer code when the builder has arrived on site.
===============
*/
void DismantleSite::request_builder_callback
	(Game            &       game,
	 Request         &       rq,
	 Ware_Index,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	assert(w);

	DismantleSite & ds = ref_cast<DismantleSite, PlayerImmovable>(target);

	ds.m_builder = w;

	delete &rq;
	ds.m_builder_request = 0;

	w->start_task_buildingwork(game);
	ds.set_seeing(true);
}

bool DismantleSite::is_working() const {
	if (!m_builder_request)
		return true;
	return false;
}

/*
===============
Called by our builder to get instructions.
===============
*/
bool DismantleSite::get_building_work(Game & game, Worker & worker, bool) {
	return false;
#if 0
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
#endif
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
	// TODO: SirVer, draws nothing currently
#if 0
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
		dst.drawanimrect(pos, a, tanim - FRAME_LENGTH, get_owner(), Rect(Point(0, 0), w, h - lines));
	}

	assert(lines <= h);
	dst.drawanimrect(pos, anim, tanim, get_owner(), Rect(Point(0, h - lines), w, lines));

	// Draw help strings
	draw_help(game, dst, coords, pos);
#endif
}

}
