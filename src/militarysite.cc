/*
 * Copyright (C) 2002-2004 by Widelands Development Team
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

#include "editor_game_base.h"
#include "game.h"
#include "militarysite.h"
#include "player.h"
#include "profile.h"
#include "transport.h"
#include "worker.h"


/*
=============================

class MilitarySite_Descr

=============================
*/
MilitarySite_Descr::MilitarySite_Descr(Tribe_Descr* tribe, const char* name)
	: ProductionSite_Descr(tribe, name)
{
	m_conquer_radius = 0;
	m_num_soldiers = 0;
	m_num_medics = 0;
	m_heal_per_second = 0;
	m_heal_incr_per_medic = 0;
}

MilitarySite_Descr::~MilitarySite_Descr()
{
}

/*
===============
MilitarySite_Descr::parse

Parse the additional information necessary for miltary buildings
===============
*/
void MilitarySite_Descr::parse(const char* directory, Profile* prof,
	const EncodeData* encdata)
{
	Section* sglobal = prof->get_section("global");

	Building_Descr::parse(directory, prof, encdata);
	ProductionSite_Descr::parse(directory,prof,encdata);

	m_conquer_radius=sglobal->get_safe_int("conquers");
	m_num_soldiers=sglobal->get_safe_int("max_soldiers");
	m_num_medics=sglobal->get_safe_int("max_medics");
	m_heal_per_second=sglobal->get_safe_int("heal_per_second");
	m_heal_incr_per_medic=sglobal->get_safe_int("heal_increase_per_medic");
}

/*
===============
MilitarySite_Descr::create_object

Create a new building of this type
===============
*/
Building* MilitarySite_Descr::create_object()
{
	return new MilitarySite(this);
}


/*
=============================

class MilitarySite

=============================
*/

/*
===============
MilitarySite::MilitarySite
===============
*/
MilitarySite::MilitarySite(MilitarySite_Descr* descr)
	: ProductionSite(descr)
{
	m_didconquer = false;
	m_soldier = 0;
	m_soldier_request = 0;
}


/*
===============
MilitarySite::~MilitarySite
===============
*/
MilitarySite::~MilitarySite()
{
}


/*
===============
MilitarySite::get_statistics_string

Display number of soldiers.
===============
*/
std::string MilitarySite::get_statistics_string()
{
	return "(soldiers)";
}


/*
===============
MilitarySite::init

Initialize the military site.
===============
*/
void MilitarySite::init(Editor_Game_Base* g)
{
	ProductionSite::init(g);

	if (g->is_game()) {
		// Request soldier
		if(!m_soldier)
			request_soldier((Game*)g);
	}
}

/*
===============
MilitarySite::set_economy

Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void MilitarySite::set_economy(Economy* e)
{
	/*
	Economy* old = get_economy();
	uint i;

	if (old) {
		for(i = 0; i < m_input_queues.size(); i++)
			m_input_queues[i]->remove_from_economy(old);
	}

	Building::set_economy(e);

	if (e) {
		for(i = 0; i < m_input_queues.size(); i++)
			m_input_queues[i]->add_to_economy(e);
	}
	*/
	// TODO: SoldiersQueue migration
	ProductionSite::set_economy(e);
	if (m_soldier_request)
		m_soldier_request->set_economy(e);
}

/*
===============
MilitarySite::cleanup

Cleanup after a military site is removed
===============
*/
void MilitarySite::cleanup(Editor_Game_Base* g)
{
	// Release worker
	if (m_soldier_request) {
		delete m_soldier_request;
		m_soldier_request = 0;
	}

	if (m_soldier) {
		Worker* w = m_soldier;

		m_soldier = 0;
		w->set_location(0);
	}

	// unconquer land
	if (m_didconquer)
		g->unconquer_area(get_owner()->get_player_number(), get_position());

	ProductionSite::cleanup(g);
}


/*
===============
MilitarySite::remove_worker

Intercept remove_worker() calls to unassign our worker, if necessary.
===============

void MilitarySite::remove_worker(Worker* w)
{
	if (m_worker == w) {
		m_worker = 0;

		request_worker((Game*)get_owner()->get_game());
	}

	Building::remove_worker(w);
}
*/

/*
===============
MilitarySite::request_soldier

Issue the soldier request
===============
*/
void MilitarySite::request_soldier(Game* g)
{
	assert(!m_soldier);
	assert(!m_soldier_request);

	int wareid = g->get_safe_ware_id("lumberjack");

	m_soldier_request =
		new Request(this, wareid, &MilitarySite::request_soldier_callback, this);
}


/*
===============
MilitarySite::request_soldier_callback [static]

Called when our soldier arrives.
===============
*/
void MilitarySite::request_soldier_callback(Game* g, Request* rq, int ware,
	Worker* w, void* data)
{
	MilitarySite* psite = (MilitarySite*)data;

	assert(w);
	assert(w->get_location(g) == psite);

	g->conquer_area(psite->get_owner()->get_player_number(),
		psite->get_position(), psite->get_descr());
	psite->m_didconquer = true;
}


/*
===============
MilitarySite::act

Advance the program state if applicable.
===============
*/
void MilitarySite::act(Game* g, uint data)
{
	// TODO: do all kinds of stuff, but if you do nothing, let
	// ProductionSite::act() handle all this. Also note, that some ProductionSite
	// commands rely, that ProductionSite::act() is not called for a certain
	// period (like cmdAnimation). This should be reworked.
	// Maybe a new queueing system like MilitaryAct could be introduced.
	ProductionSite::act(g,data);
}
