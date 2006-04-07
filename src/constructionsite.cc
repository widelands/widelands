/*
 * Copyright (C) 2002-2004, 2006 by Widelands Development Team
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

#include <stdio.h>
#include "animation.h"
#include "constructionsite.h"
#include "editor_game_base.h"
#include "error.h"
#include "game.h"
#include "graphic.h"
#include "player.h"
#include "rendertarget.h"
#include "sound_handler.h"
#include "system.h"
#include "transport.h"
#include "tribe.h"
#include "wexception.h"
#include "worker.h"

#include "ui/ui_basic/ui_object.h" //only needed for i18n function _()

static const int CONSTRUCTIONSITE_STEP_TIME = 30000;


/*
===============
ConstructionSite_Descr::ConstructionSite_Descr
===============
*/
ConstructionSite_Descr::ConstructionSite_Descr(Tribe_Descr* tribe, const char* name)
	: Building_Descr(tribe, name)
{
}


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
Building* ConstructionSite_Descr::create_object()
{
	return new ConstructionSite(this);
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
ConstructionSite::ConstructionSite(ConstructionSite_Descr* descr)
	: Building(descr)
{
	m_building = 0;
	m_prev_building= 0;

	m_builder = 0;
	m_builder_request = 0;

	m_fetchfromflag = 0;

	m_working = false;
	m_work_steptime = 0;
	m_work_completed = 0;
	m_work_steps = 0;
}


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
int ConstructionSite::get_size()
{
	return m_building->get_size();
}

/*
 * Write infos over this constructionsite
 */
void ConstructionSite::log_general_info(Editor_Game_Base* egbase) {
   Building::log_general_info(egbase);

   molog("m_building: %p\n", m_building);
   molog("* m_building (name): %s\n", m_building->get_name());
   molog("m_prev_building: %p\n", m_prev_building);
   if(m_prev_building)
      molog("* m_prev_building (name): %s\n", m_prev_building->get_name());

   molog("m_builder_request: %p\n", m_builder_request);
   molog("m_builder: %p\n", m_builder);

   molog("m_fetchfromflag: %i\n", m_fetchfromflag);

	molog("m_working: %i\n", m_working);
	molog("m_work_steptime: %i\n", m_work_steptime);
	molog("m_work_completed: %i\n", m_work_completed);
	molog("m_work_steps: %i\n", m_work_steps);

   molog("WaresQueue size: %i\n", m_wares.size());
   for(uint i=0; i<m_wares.size(); i++) {
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
uint ConstructionSite::get_playercaps()
{
	uint caps = Building::get_playercaps();

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
uint ConstructionSite::get_ui_anim()
{
	return get_building()->get_animation("idle");
}


/*
===============
ConstructionSite::get_census_string

Print the name of the building we build.
===============
*/
std::string ConstructionSite::get_census_string() const
{
	return get_building()->get_descname();
}


/*
===============
ConstructionSite::get_statistics_string

Print completion percentage.
===============
*/
std::string ConstructionSite::get_statistics_string()
{
	char buf[40];

	snprintf(buf, sizeof(buf), "%u%% %s", (get_built_per64k() * 100) >> 16, _("built"));

	return std::string(buf);
}


/*
===============
ConstructionSite::get_built_per64k

Return the completion "percentage", where 2^16 = completely built,
0 = nothing built.
===============
*/
uint ConstructionSite::get_built_per64k()
{
	uint time = get_owner()->get_game()->get_gametime();
	uint thisstep = m_working ? (CONSTRUCTIONSITE_STEP_TIME - m_work_steptime + time) : 0;
	uint total;

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
void ConstructionSite::set_building(Building_Descr* descr)
{
	assert(!m_building);

	m_building = descr;
}

/*
 * Set previous building
 * That is the building that was here before, we're
 * an enhancement
 */
void ConstructionSite::set_previous_building(Building_Descr* descr) {
   assert(!m_prev_building);

   m_prev_building=descr;

   if(!m_prev_building->get_animation("build"))
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
	uint i;

	if (old) {
		for(i = 0; i < m_wares.size(); i++)
			m_wares[i]->remove_from_economy(old);
	}

	Building::set_economy(e);
	if (m_builder_request)
		m_builder_request->set_economy(e);

	if (e) {
		for(i = 0; i < m_wares.size(); i++)
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

	if (g->is_game()) {
		uint i;

		// TODO: figure out whether planing is necessary

		// Initialize the wares queues
		const Building_Descr::BuildCost* bc = m_building->get_buildcost();

		m_wares.resize(bc->size());

		for(i = 0; i < bc->size(); i++) {
			WaresQueue* wq = new WaresQueue(this);

			m_wares[i] = wq;

			wq->set_callback(&ConstructionSite::wares_queue_callback, this);
			wq->set_consume_interval(CONSTRUCTIONSITE_STEP_TIME);
			wq->init((Game*)g, get_owner()->get_tribe()->get_safe_ware_index((*bc)[i].name.c_str()), (*bc)[i].amount);

			m_work_steps += (*bc)[i].amount;
		}

		request_builder((Game*)g);

		//TODO: should this fx be played for AI players too?
		if ( get_owner()->get_type()==Player::playerLocal)
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
	for(uint i = 0; i < m_wares.size(); i++) {
		m_wares[i]->cleanup((Game*)g);
		delete m_wares[i];
	}
	m_wares.clear();

	Building::cleanup(g);

	if (m_work_completed >= m_work_steps)
	{
		// Put the real building in place
		Building* bld = m_building->create(g, get_owner(), m_position, false);
		bld->set_stop(get_stop());
		// Walk the builder home safely
      if(g->get_objects()->object_still_available(m_builder)) {
         m_builder->reset_tasks((Game*)g);
         m_builder->set_location(bld);
         m_builder->start_task_gowarehouse((Game*)g);
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
void ConstructionSite::request_builder(Game* g)
{
	assert(!m_builder && !m_builder_request);

	m_builder_request = new Request(this, get_owner()->get_tribe()->get_safe_worker_index("builder"),
	                                &ConstructionSite::request_builder_callback, this, Request::WORKER);
}


/*
===============
ConstructionSite::request_builder_callback [static]

Called by transfer code when the builder has arrived on site.
===============
*/
void ConstructionSite::request_builder_callback(Game* g, Request* rq, int ware, Worker* w, void* data)
{
	assert(w);

	ConstructionSite* cs = (ConstructionSite*)data;

	cs->m_builder = w;

	delete rq;
	cs->m_builder_request = 0;

	w->start_task_buildingwork(g);
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

	if (m_builder)
		m_builder->update_task_buildingwork(g);

	return true;
}


/*
===============
ConstructionSite::get_building_work

Called by our builder to get instructions.
===============
*/
bool ConstructionSite::get_building_work(Game* g, Worker* w, bool success)
{
	assert(w == m_builder);

	// Check if one step has completed
	if (m_working) {
		if ((int)(g->get_gametime() - m_work_steptime) < 0) {
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
		w->start_task_fetchfromflag(g);
		return true;
	}

	// Check if we've got wares to consume
	if (m_work_completed < m_work_steps)
	{
		for(uint i = 0; i < m_wares.size(); i++) {
			WaresQueue* wq = m_wares[i];

			if (!wq->get_filled())
				continue;

			molog("ConstructionSite::check_work: wq has %i/%i, begin work\n",
						wq->get_filled(), wq->get_size());

			wq->set_filled(wq->get_filled() - 1);
			wq->set_size(wq->get_size() - 1);
			wq->update(g);

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
void ConstructionSite::wares_queue_callback(Game* g, WaresQueue* wq, int ware, void* data)
{
	ConstructionSite* cs = (ConstructionSite*)data;

	if (!cs->m_working && cs->m_builder)
		cs->m_builder->update_task_buildingwork(g);
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
	const int gametime = game.get_gametime();
	uint tanim = gametime - m_animstart;

	if (coords != m_position)
		return; // draw big buildings only once

	// Draw the construction site marker
	dst.drawanim(pos.x, pos.y, m_anim, tanim, get_owner());

	// Draw the partially finished building
	int totaltime;
	int completedtime;
	int w, h;
	int lines;
	uint anim;

	totaltime = CONSTRUCTIONSITE_STEP_TIME * m_work_steps;
	completedtime = CONSTRUCTIONSITE_STEP_TIME * m_work_completed;

	if (m_working)
		completedtime += CONSTRUCTIONSITE_STEP_TIME + gametime - m_work_steptime;

	anim = get_building()->get_animation("build");
   int nr_pics=g_gr->get_animation_nr_frames(anim);
   uint anim_pic = completedtime * nr_pics / totaltime;
	// Redefine tanim
   tanim = anim_pic*FRAME_LENGTH;

   g_gr->get_animation_size(anim, tanim, &w, &h);

	lines = h * completedtime * nr_pics / totaltime;
   lines -= h*anim_pic; // This won't work if pictures have various sizes

   // NoLog("drawing lines %i/%i from pic %i/%i\n", lines, h, anim_pic, nr_pics);
   if(anim_pic) // not the first pic
      // draw the prev pic completly
      dst.drawanim(pos.x, pos.y, anim, tanim - FRAME_LENGTH, get_owner());

   if(!anim_pic && m_prev_building) {
      // Is the first building, but there was another building here before,
      // get its last build picture and draw it instead
      int w, h;
      int anim = m_prev_building->get_animation("build");
      int nr_pics=g_gr->get_animation_nr_frames(anim);
      g_gr->get_animation_size(anim, tanim, &w, &h);
      int tanim = (nr_pics-1)*FRAME_LENGTH;
      dst.drawanim(pos.x, pos.y, anim, tanim, get_owner());
   }

	dst.drawanimrect
		(pos.x, pos.y, anim, tanim, get_owner(), 0, h - lines, w, lines);

	// Draw help strings
	draw_help(game, dst, coords, pos);
}
