/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "animation.h"
#include "editor_game_base.h"
#include "game.h"
#include "graphic.h"
#include "i18n.h"
#include "player.h"
#include "rendertarget.h"
#include "sound/sound_handler.h"
#include "transport.h"
#include "tribe.h"
#include "wexception.h"
#include "worker.h"

#include "ui_object.h" //only needed for i18n function _()

#include <stdio.h>

static const int32_t CONSTRUCTIONSITE_STEP_TIME = 30000;


/*
===============
ConstructionSite_Descr::ConstructionSite_Descr
===============
*/
ConstructionSite_Descr::ConstructionSite_Descr
(const Tribe_Descr & tribe_descr, const std::string & building_name)
:
Building_Descr(tribe_descr, building_name)
{}


/*
===============
ConstructionSite_Descr::parse

Parse tribe-specific construction site data, such as graphics, worker type,
etc...
===============
*/
void ConstructionSite_Descr::parse(const char* directory, Profile* prof, const EncodeData* encdata)
{
	add_attribute(Map_Object::CONSTRUCTIONSITE);

   Building_Descr::parse(directory, prof, encdata);

	// TODO
}


/*
===============
ConstructionSite_Descr::create_object

Allocate a ConstructionSite
===============
*/
Building* ConstructionSite_Descr::create_object() const
{return new ConstructionSite(*this);}


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
ConstructionSite::~ConstructionSite
===============
*/
ConstructionSite::~ConstructionSite()
{
}


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
void ConstructionSite::log_general_info(Editor_Game_Base* egbase) {
   Building::log_general_info(egbase);

   molog("m_building: %p\n", m_building);
	molog("* m_building (name): %s\n", m_building->name().c_str());
   molog("m_prev_building: %p\n", m_prev_building);
	if (m_prev_building)
		molog("* m_prev_building (name): %s\n", m_prev_building->name().c_str());

   molog("m_builder_request: %p\n", m_builder_request);
   molog("m_builder: %u\n", m_builder.get_serial());

   molog("m_fetchfromflag: %i\n", m_fetchfromflag);

	molog("m_working: %i\n", m_working);
	molog("m_work_steptime: %i\n", m_work_steptime);
	molog("m_work_completed: %i\n", m_work_completed);
	molog("m_work_steps: %i\n", m_work_steps);

   molog("WaresQueue size: %i\n", m_wares.size());
	const std::vector<WaresQueue *>::size_type nr_wares = m_wares.size();
	for (std::vector<WaresQueue *>::size_type i = 0; i < nr_wares; ++i) {
      molog("Dumping WaresQueue %i/%i\n", i+1, m_wares.size());
      molog("* Owner: %i (player nr)\n", m_wares[i]->get_owner()->get_player_number());
      molog("* Ware: %i (index)\n", m_wares[i]->get_ware());
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


/// Print the name of the building we build.
const std::string & ConstructionSite::census_string() const throw ()
{return building().descname();}


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
		 "%u%% %s", (get_built_per64k() * 100) >> 16, _("built").c_str());
	return buffer;
}


/*
===============
ConstructionSite::get_built_per64k

Return the completion "percentage", where 2^16 = completely built,
0 = nothing built.
===============
*/
uint32_t ConstructionSite::get_built_per64k()
{
	const uint32_t time = owner().egbase().get_gametime();
	uint32_t thisstep = m_working ? (CONSTRUCTIONSITE_STEP_TIME - m_work_steptime + time) : 0;
	uint32_t total;

	thisstep = (thisstep << 16) / CONSTRUCTIONSITE_STEP_TIME;
	total = (thisstep + (m_work_completed << 16)) / m_work_steps;

	assert(thisstep <= (1 << 16));
	assert(total <= (1 << 16));

	return total;
}


/*
===============
ConstructionSite::set_building

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
(const Building_Descr * const previous_building_descr)
{
   assert(!m_prev_building);

	m_prev_building = previous_building_descr;

	if (not m_prev_building->get_animation("build"))
      throw wexception("Trying to enhance a non buildable building!\n");
}

/*
===============
ConstructionSite::set_economy

Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void ConstructionSite::set_economy(Economy* e)
{
	Economy* old = get_economy();
	uint32_t i;

	if (old) {
		for (i = 0; i < m_wares.size(); i++)
			m_wares[i]->remove_from_economy(old);
	}

	Building::set_economy(e);
	if (m_builder_request)
		m_builder_request->set_economy(e);

	if (e) {
		for (i = 0; i < m_wares.size(); i++)
			m_wares[i]->add_to_economy(e);
	}
}


/*
===============
ConstructionSite::init

Initialize the construction site by starting orders
===============
*/
void ConstructionSite::init(Editor_Game_Base* g)
{
	Building::init(g);

	if (Game * const game = dynamic_cast<Game *>(g)) {
		const Tribe_Descr & tribe = owner().tribe();
		// TODO: figure out whether planing is necessary

		// Initialize the wares queues
		const Building_Descr::BuildCost & buildcost = m_building->get_buildcost();
		const Building_Descr::BuildCost::size_type buildcost_size =
			buildcost.size();
		m_wares.resize(buildcost_size);

		for (Building_Descr::BuildCost::size_type i = 0; i < buildcost_size; ++i)
		{
			WaresQueue* wq = new WaresQueue(this);

			m_wares[i] = wq;

			wq->set_callback(&ConstructionSite::wares_queue_callback, this);
			wq->set_consume_interval(CONSTRUCTIONSITE_STEP_TIME);
			wq->init
				(tribe.get_safe_ware_index(buildcost[i].name.c_str()),
				 buildcost[i].amount);

			m_work_steps += buildcost[i].amount;
		}

		request_builder(game);

		//TODO: should this fx be played for AI players too?
		if (owner().get_type() == Player::Local)
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
void ConstructionSite::cleanup(Editor_Game_Base* g)
{
	// Release worker
	if (m_builder_request) {
		delete m_builder_request;
		m_builder_request = 0;
	}

	// Cleanup the wares queues
	for (uint32_t i = 0; i < m_wares.size(); i++) {
		m_wares[i]->cleanup();
		delete m_wares[i];
	}
	m_wares.clear();

	Building::cleanup(g);

	if (m_work_completed >= m_work_steps)
	{
		// Put the real building in place
		Building * const bld = m_building->create(*g, owner(), m_position, false);
		bld->set_stop(get_stop());
		// Walk the builder home safely
		Worker* builder = m_builder.get(g);
		if (builder) {
			builder->reset_tasks((Game*)g);
			builder->set_location(bld);
			builder->start_task_gowarehouse();
		}
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

	if (m_work_completed)
		return true;

	return false;
}


/*
===============
ConstructionSite::request_builder

Issue a request for the builder.
===============
*/
void ConstructionSite::request_builder(Game *) {
	assert(!m_builder.is_set() && !m_builder_request);

	m_builder_request = new Request(this, get_owner()->tribe().get_safe_worker_index("builder"),
	                                &ConstructionSite::request_builder_callback, this, Request::WORKER);
}


/*
===============
ConstructionSite::request_builder_callback [static]

Called by transfer code when the builder has arrived on site.
===============
*/
void ConstructionSite::request_builder_callback
(Game *, Request * rq, int32_t, Worker * w, void * data)
{
	assert(w);

	ConstructionSite* cs = (ConstructionSite*)data;

	cs->m_builder = w;

	delete rq;
	cs->m_builder_request = 0;

	w->start_task_buildingwork();
}


/*
===============
ConstructionSite::fetch_from_flag

Remember the item on the flag. The worker will be sent from get_building_work().
===============
*/
bool ConstructionSite::fetch_from_flag(Game* g)
{
	m_fetchfromflag++;

	Worker* builder = m_builder.get(g);
	if (builder)
		builder->update_task_buildingwork(g);

	return true;
}


/*
===============
ConstructionSite::get_building_work

Called by our builder to get instructions.
===============
*/
bool ConstructionSite::get_building_work(Game * g, Worker * w, bool) {
	assert(w == m_builder.get(g));

	// Check if one step has completed
	if (m_working) {
		if (static_cast<int32_t>(g->get_gametime() - m_work_steptime) < 0) {
			w->start_task_idle(g, w->get_animation("idle"), m_work_steptime - g->get_gametime());
			return true;
		} else {
			molog("ConstructionSite::check_work: step %i completed\n", m_work_completed);
			//TODO(fweber): cause "construction sounds" to be played - perhaps dependent on kind of construction?

			m_work_completed++;
			if (m_work_completed >= m_work_steps)
				schedule_destroy(g);

			m_working = false;
		}
	}

	// Fetch items from flag
	if (m_fetchfromflag) {
		m_fetchfromflag--;
		w->start_task_fetchfromflag();
		return true;
	}

	// Check if we've got wares to consume
	if (m_work_completed < m_work_steps)
	{
		for (uint32_t i = 0; i < m_wares.size(); i++) {
			WaresQueue* wq = m_wares[i];

			if (!wq->get_filled())
				continue;

			molog
				("ConstructionSite::check_work: wq has %i/%i, begin work\n",
				 wq->get_filled(), wq->get_size());

			wq->set_filled(wq->get_filled() - 1);
			wq->set_size(wq->get_size() - 1);
			wq->update();

			m_working = true;
			m_work_steptime = g->get_gametime() + CONSTRUCTIONSITE_STEP_TIME;

			w->start_task_idle(g, w->get_animation("idle"), CONSTRUCTIONSITE_STEP_TIME);
			return true;
		}
	}

	return false; // sorry, got no work for you
}


/*
===============
ConstructionSite::wares_queue_callback [static]

Called by WaresQueue code when an item has arrived
===============
*/
void ConstructionSite::wares_queue_callback
(Game * g, WaresQueue *, int32_t, void * data)
{
	ConstructionSite* cs = (ConstructionSite*)data;

	if (!cs->m_working) {
		Worker* builder = cs->m_builder.get(g);

		if (builder)
			builder->update_task_buildingwork(g);
	}
}


/*
===============
ConstructionSite::draw

Draw the construction site.
===============
*/
void ConstructionSite::draw
(const Editor_Game_Base & game,
 RenderTarget & dst,
 const FCoords coords,
 const Point pos)
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

	const uint32_t anim = building().get_animation("build");
	const AnimationGfx::Index nr_frames = g_gr->nr_frames(anim);
   uint32_t anim_pic = completedtime * nr_frames / totaltime;
	// Redefine tanim
   tanim = anim_pic*FRAME_LENGTH;

	uint32_t w, h;
   g_gr->get_animation_size(anim, tanim, w, h);

	uint32_t lines = h * completedtime * nr_frames / totaltime;
	assert(h * anim_pic <= lines);
   lines -= h*anim_pic; // This won't work if pictures have various sizes

   // NoLog("drawing lines %i/%i from pic %i/%i\n", lines, h, anim_pic, nr_pics);
	if (anim_pic) //  not the first pic
      // draw the prev pic from top to where next image will be drawing
      dst.drawanimrect
        (pos, anim, tanim - FRAME_LENGTH, get_owner(), Rect(Point(0, 0), w, h - lines));
	else if (m_prev_building) {
      // Is the first building, but there was another building here before,
      // get its last build picture and draw it instead
		const uint32_t a = m_prev_building->get_animation("build");
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
