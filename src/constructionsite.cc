/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "constructionsite.h"

#include "graphic/animation.h"
#include "economy/request.h"
#include "economy/wares_queue.h"
#include "editor_game_base.h"
#include "game.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "player.h"
#include "graphic/rendertarget.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "upcast.h"
#include "wexception.h"
#include "worker.h"

#include "ui_object.h" //only needed for i18n function _()


#include <cstdio>

namespace Widelands {

#define CONSTRUCTIONSITE_STEP_TIME 30000


/*
===============
ConstructionSite_Descr::ConstructionSite_Descr
===============
*/
ConstructionSite_Descr::ConstructionSite_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const & _tribe, EncodeData const * const encdata)
: Building_Descr(_name, _descname, directory, prof, global_s, _tribe, encdata)
{
	add_attribute(Map_Object::CONSTRUCTIONSITE);
}


Building & ConstructionSite_Descr::create_object() const {
	return *new ConstructionSite(*this);
}


/*
==============================

IMPLEMENTATION

==============================
*/


/*
===============
ConstructionSite::ConstructionSite

Initialize with default values
===============
*/
ConstructionSite::ConstructionSite(const ConstructionSite_Descr & cs_descr) :
Building         (cs_descr),
m_building       (0),
m_prev_building  (0),
m_builder_request(0),
m_fetchfromflag  (0),
m_working        (false),
m_work_steptime  (0),
m_work_completed (0),
m_work_steps     (0)
{}


/*
===============
ConstructionSite::get_size

Override: construction size is always the same size as the building
===============
*/
int32_t ConstructionSite::get_size() const throw ()
{return m_building->get_size();}

/*
 * Write infos over this constructionsite
 */
void ConstructionSite::log_general_info(Editor_Game_Base const & egbase) {
	Building::log_general_info(egbase);

	molog("m_building: %p\n", m_building);
	molog("* m_building (name): %s\n", m_building->name().c_str());
	molog("m_prev_building: %p\n", m_prev_building);
	if (m_prev_building)
		molog("* m_prev_building (name): %s\n", m_prev_building->name().c_str());

	molog("m_builder_request: %p\n", m_builder_request);
	molog("m_builder: %u\n", m_builder.serial());

	molog("m_fetchfromflag: %i\n", m_fetchfromflag);

	molog("m_working: %i\n", m_working);
	molog("m_work_steptime: %i\n", m_work_steptime);
	molog("m_work_completed: %i\n", m_work_completed);
	molog("m_work_steps: %i\n", m_work_steps);

	molog
		("WaresQueue size: %lu\n",
		 static_cast<long unsigned int>(m_wares.size()));
	const std::vector<WaresQueue *>::size_type nr_wares = m_wares.size();
	for (std::vector<WaresQueue *>::size_type i = 0; i < nr_wares; ++i) {
		molog
			("Dumping WaresQueue %lu/%lu\n",
			 static_cast<long unsigned int>(i + 1),
			 static_cast<long unsigned int>(m_wares.size()));
		molog
			("* Owner: %i (player nr)\n",
			 m_wares[i]->owner().get_player_number());
		molog("* Ware: %u (index)\n", m_wares[i]->get_ware().value());
		molog("* Size: %i\n", m_wares[i]->get_size());
		molog("* Filled: %i\n", m_wares[i]->get_filled());
		molog("* Consume Interval: %i\n", m_wares[i]->get_consume_interval());
	}
}

/*
===============
ConstructionSite::get_playercaps

Override: Even though construction sites cannot be built themselves, you can
bulldoze them.
===============
*/
uint32_t ConstructionSite::get_playercaps() const throw () {
	uint32_t caps = Building::get_playercaps();

	caps |= 1 << PCap_Bulldoze;

	return caps;
}


/*
===============
ConstructionSite::get_ui_anim

Return the animation for the building that is in construction, as this
should be more useful to the player.
===============
*/
uint32_t ConstructionSite::get_ui_anim() const
{return building().get_animation("idle");}


/*
===============
ConstructionSite::get_statistics_string

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
===============
Return the completion "percentage", where 2^16 = completely built,
0 = nothing built.
===============
*/
uint32_t ConstructionSite::get_built_per64k()
{
	const uint32_t time = owner().egbase().get_gametime();
	uint32_t thisstep = 0;

	if (m_working) {
		thisstep = CONSTRUCTIONSITE_STEP_TIME - (m_work_steptime - time);
		// The check below is necessary because we drive construction via
		// the construction worker in get_building_work(), and there can be
		// a small delay between the worker completing his job and requesting
		// new work.
		if (thisstep > CONSTRUCTIONSITE_STEP_TIME)
			thisstep = CONSTRUCTIONSITE_STEP_TIME;
	}
	thisstep = (thisstep << 16) / CONSTRUCTIONSITE_STEP_TIME;
	uint32_t total = (thisstep + (m_work_completed << 16));
	if (m_work_steps)
		total /= m_work_steps;

	assert(total <= (1 << 16));

	return total;
}


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
	assert(!m_building);

	m_building = &building_descr;
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
}

/*
===============
ConstructionSite::set_economy

Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void ConstructionSite::set_economy(Economy * const e)
{
	if (Economy * const old = get_economy())
		container_iterate_const(Wares, m_wares, i)
			(*i.current)->remove_from_economy(*old);

	Building::set_economy(e);
	if (m_builder_request)
		m_builder_request->set_economy(e);

	if (e)
		container_iterate_const(Wares, m_wares, i)
			(*i.current)->add_to_economy(*e);
}


/*
===============
Initialize the construction site by starting orders
===============
*/
void ConstructionSite::init(Editor_Game_Base & egbase)
{
	Building::init(egbase);

	if (upcast(Game, game, &egbase)) {
		// TODO: figure out whether planing is necessary

		// Initialize the wares queues
		std::map<Ware_Index, uint8_t> const & buildcost =
			m_building->buildcost();
		size_t const buildcost_size = buildcost.size();
		m_wares.resize(buildcost_size);
		std::map<Ware_Index, uint8_t>::const_iterator it = buildcost.begin();
		for (size_t i = 0; i < buildcost_size; ++i, ++it) {
			WaresQueue & wq =
				*(m_wares[i] = new WaresQueue(*this, it->first, it->second));

			wq.set_callback(ConstructionSite::wares_queue_callback, this);
			wq.set_consume_interval(CONSTRUCTIONSITE_STEP_TIME);
			wq.update();

			m_work_steps += it->second;
		}

		request_builder(*game);

		g_sound_handler.play_fx("create_construction_site", m_position, 255);
	}
}


/*
===============
ConstructionSite::cleanup

Release worker and material (if any is left).
If construction was finished successfully, place the building at our position.
===============
*/
void ConstructionSite::cleanup(Editor_Game_Base & egbase)
{
	// Release worker
	if (m_builder_request) {
		delete m_builder_request;
		m_builder_request = 0;
	}

	// Cleanup the wares queues
	container_iterate_const(Wares, m_wares, i) {
		(*i.current)->cleanup();
		delete *i.current;
	}
	m_wares.clear();

	Building::cleanup(egbase);

	if (m_work_steps <= m_work_completed) {
		// Put the real building in place
		m_building->create(egbase, owner(), m_position, false);
		if (Worker * const builder = m_builder.get(egbase))
			builder->reset_tasks(dynamic_cast<Game &>(egbase));
	}
}


/*
===============
ConstructionSite::burn_on_destroy

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
ConstructionSite::request_builder

Issue a request for the builder.
===============
*/
void ConstructionSite::request_builder(Game &) {
	assert(!m_builder.is_set() && !m_builder_request);

	m_builder_request =
		new Request
			(*this,
			 tribe().safe_worker_index("builder"),
			 ConstructionSite::request_builder_callback,
			 Request::WORKER);
}


/*
===============
Called by transfer code when the builder has arrived on site.
===============
*/
void ConstructionSite::request_builder_callback
	(Game            &       game,
	 Request         &       rq,
	 Ware_Index,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	assert(w);

	ConstructionSite & cs = dynamic_cast<ConstructionSite &>(target);

	cs.m_builder = w;

	delete &rq;
	cs.m_builder_request = 0;

	w->start_task_buildingwork(game);
}


/*
===============
ConstructionSite::fetch_from_flag

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
				 worker.get_animation("idle"),
				 m_work_steptime - game.get_gametime());
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
		worker.start_task_fetchfromflag(game);
		return true;
	}

	// Check if we've got wares to consume
	if (m_work_completed < m_work_steps)
	{
		for (uint32_t i = 0; i < m_wares.size(); ++i) {
			WaresQueue & wq = *m_wares[i];

			if (!wq.get_filled())
				continue;

			wq.set_filled(wq.get_filled() - 1);
			wq.set_size(wq.get_size() - 1);
			wq.update();

			m_working = true;
			m_work_steptime = game.get_gametime() + CONSTRUCTIONSITE_STEP_TIME;

			worker.start_task_idle
				(game, worker.get_animation("idle"), CONSTRUCTIONSITE_STEP_TIME);
			return true;
		}
	}

	return false; // sorry, got no work for you
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
ConstructionSite::draw

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
	const uint32_t totaltime = CONSTRUCTIONSITE_STEP_TIME * m_work_steps;
	uint32_t completedtime = CONSTRUCTIONSITE_STEP_TIME * m_work_completed;

	if (m_working) {
		assert
			(m_work_steptime
			 <=
			 completedtime + CONSTRUCTIONSITE_STEP_TIME + gametime);
		completedtime += CONSTRUCTIONSITE_STEP_TIME + gametime - m_work_steptime;
	}

	uint32_t anim;
	try {
		anim = building().get_animation("build");
	} catch (Map_Object_Descr::Animation_Nonexistent) {
		anim = building().get_animation("idle");
	}
	const AnimationGfx::Index nr_frames = g_gr->nr_frames(anim);
	uint32_t const anim_pic =
		totaltime ? completedtime * nr_frames / totaltime : 0;
	// Redefine tanim
	tanim = anim_pic * FRAME_LENGTH;

	uint32_t w, h;
	g_gr->get_animation_size(anim, tanim, w, h);

	uint32_t lines = h * completedtime * nr_frames;
	if (totaltime)
		lines /= totaltime;
	assert(h * anim_pic <= lines);
	lines -= h * anim_pic; //  This won't work if pictures have various sizes.

	// NoLog("drawing lines %i/%i from pic %i/%i\n", lines, h, anim_pic,
	// nr_pics);
	if (anim_pic) //  not the first pic
		//  draw the prev pic from top to where next image will be drawing
		dst.drawanimrect
			(pos,
			 anim,
			 tanim - FRAME_LENGTH, get_owner(),
			 Rect(Point(0, 0), w, h - lines));
	else if (m_prev_building) {
		//  Is the first building, but there was another building here before,
		//  get its last build picture and draw it instead.
		uint32_t a;
		try {
			a = m_prev_building->get_animation("build");
		} catch (Map_Object_Descr::Animation_Nonexistent) {
			a = m_prev_building->get_animation("idle");
		}
		dst.drawanim
			(pos,
			 a,
			 (g_gr->nr_frames(a) - 1) * FRAME_LENGTH,
			 get_owner());
	}

	assert(lines <= h);
	dst.drawanimrect
		(pos, anim, tanim, get_owner(), Rect(Point(0, h - lines), w, lines));

	// Draw help strings
	draw_help(game, dst, coords, pos);
}

};
