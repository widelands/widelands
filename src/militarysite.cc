/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "militarysite.h"

#include "editor_game_base.h"
#include "game.h"
#include "i18n.h"
#include "player.h"
#include "profile.h"
#include "request.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "worker.h"

#include "log.h"

#include "upcast.h"

#include <libintl.h>
#include <locale.h>
#include <stdio.h>

namespace Widelands {

MilitarySite_Descr::MilitarySite_Descr
(const Tribe_Descr & tribe_descr, const std::string & militarysite_name)
:
ProductionSite_Descr (tribe_descr, militarysite_name),
m_conquer_radius     (0),
m_num_soldiers       (0),
m_num_medics         (0),
m_heal_per_second    (0),
m_heal_incr_per_medic(0)
{}

MilitarySite_Descr::~MilitarySite_Descr() {}

/**
===============
Parse the additional information necessary for miltary buildings
===============
*/
void MilitarySite_Descr::parse
	(char       const * const directory,
	 Profile          * const prof,
	 EncodeData const * const encdata)
{
	Section* sglobal = prof->get_section("global");

	ProductionSite_Descr::parse(directory, prof, encdata);
	m_stopable = false; //  Militarysites are not stopable.

	m_conquer_radius=sglobal->get_safe_int("conquers");
	m_num_soldiers=sglobal->get_safe_int("max_soldiers");
	m_num_medics=sglobal->get_safe_int("max_medics");
	m_heal_per_second=sglobal->get_safe_int("heal_per_second");
	m_heal_incr_per_medic=sglobal->get_safe_int("heal_increase_per_medic");
	if (m_conquer_radius > 0)
		m_workarea_info[m_conquer_radius].insert(descname() + _(" conquer"));
}

/**
===============
MilitarySite_Descr::create_object

Create a new building of this type
===============
*/
Building * MilitarySite_Descr::create_object() const
{return new MilitarySite(*this);}


/*
=============================

class MilitarySite

=============================
*/

/**
===============
MilitarySite::MilitarySite
===============
*/
MilitarySite::MilitarySite(const MilitarySite_Descr & ms_descr) :
ProductionSite(ms_descr),
m_soldier_request(0),
m_didconquer  (false),
m_capacity    (ms_descr.get_max_number_of_soldiers()),
m_in_battle   (false),
m_nexthealtime(0)
{}


/**
===============
MilitarySite::~MilitarySite
===============
*/
MilitarySite::~MilitarySite()
{
	assert(!m_soldier_request);
}


/**
===============
Display number of soldiers.
===============
*/
std::string MilitarySite::get_statistics_string()
{
	char buffer[255];
	std::string str;
	uint32_t present = presentSoldiers().size();
	uint32_t total = stationedSoldiers().size();

	if (present == total) {
		snprintf
			(buffer, sizeof(buffer),
			 ngettext("%u soldier", "%u soldiers", total),
			 total);
	} else {
		snprintf
			(buffer, sizeof(buffer),
			 ngettext("%u(+%u) soldier", "%u(+%u) soldiers", total),
			 present, total-present);
	}
	str = buffer;

	if (m_capacity > total) {
		snprintf(buffer, sizeof(buffer), " (+%u)", m_capacity-total);
		str += buffer;
	}

	return str;
}


void MilitarySite::init(Editor_Game_Base* g)
{
	ProductionSite::init(g);

	if (upcast(Game, game, g)) {
		update_soldier_request();

		// Schedule the first healing
		m_nexthealtime = g->get_gametime()+1000;
		schedule_act(game, 1000);
	}
}


/**
===============
MilitarySite::set_economy

Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void MilitarySite::set_economy(Economy* e)
{
	ProductionSite::set_economy(e);

	if (m_soldier_request && e)
		m_soldier_request->set_economy(e);
}

/**
===============
Cleanup after a military site is removed
===============
*/
void MilitarySite::cleanup(Editor_Game_Base* g)
{
	delete m_soldier_request;
	m_soldier_request = 0;

	// unconquer land
	if (m_didconquer)
		g->unconquer_area
			(Player_Area<Area<FCoords> >
			 	(owner().get_player_number(),
			 	 Area<FCoords>
			 	 	(g->map().get_fcoords(get_position()), get_conquers())),
			 m_defeating_player);

	ProductionSite::cleanup(g);
}


/*
===============
MilitarySite::request_soldier_callback [static]

Called when our soldier arrives.
===============
*/
void MilitarySite::request_soldier_callback
(Game * g, Request *, Ware_Index, Worker * w, void * data)
{
	MilitarySite & msite = *static_cast<MilitarySite *>(data);
	Soldier & s = dynamic_cast<Soldier &>(*w);

	assert(s.get_location(g) == &msite);

	if (not msite.m_didconquer)
		msite.conquer_area(*g);

	// Bind the worker into this house, hide him on the map
	s.reset_tasks(g);
	s.start_task_buildingwork(g);

	// Make sure the request count is reduced or the request is deleted.
	msite.update_soldier_request();
}


/**
 * Update the request for soldiers and cause soldiers to be evicted
 * as appropriate.
 */
void MilitarySite::update_soldier_request()
{
	std::vector<Soldier*> present = presentSoldiers();
	uint32_t stationed = stationedSoldiers().size();

	if (stationed < m_capacity) {
		if (!m_soldier_request) {
			int32_t soldierid = get_owner()->tribe().get_safe_worker_index("soldier");

			m_soldier_request = new Request
				(this,
				 soldierid,
				 &MilitarySite::request_soldier_callback,
				 this,
				 Request::WORKER);
			m_soldier_request->set_requirements (m_soldier_requirements);
		}

		m_soldier_request->set_count(m_capacity - stationed);
	} else {
		delete m_soldier_request;
		m_soldier_request = 0;
	}

	if (present.size() > m_capacity) {
		if (upcast(Game, g, &owner().egbase())) {
			for(uint32_t i = 0; i < present.size()-m_capacity; ++i) {
				Soldier* soldier = present[i];
				soldier->reset_tasks(g);
				soldier->start_task_leavebuilding(g, true);
			}
		}
	}
}


/*
===============
MilitarySite::act

Advance the program state if applicable.
===============
*/
void MilitarySite::act(Game* g, uint32_t data)
{
	// TODO: do all kinds of stuff, but if you do nothing, let
	// ProductionSite::act() handle all this. Also note, that some ProductionSite
	// commands rely, that ProductionSite::act() is not called for a certain
	// period (like cmdAnimation). This should be reworked.
	// Maybe a new queueing system like MilitaryAct could be introduced.
	ProductionSite::act(g, data);

	if (g->get_gametime() - m_nexthealtime >= 0) {
		uint32_t total_heal = descr().get_heal_per_second();
		std::vector<Soldier*> soldiers = presentSoldiers();

		for (uint32_t i = 0; i < soldiers.size(); ++i) {
			Soldier *s = soldiers[i];

			// The healing algorithm is totally arbitrary
			if (s->get_current_hitpoints() < s->get_max_hitpoints()) {
				s->heal(total_heal);
				total_heal -= total_heal / 3;
			}
		}

		m_nexthealtime = g->get_gametime() + 1000;
		schedule_act(g, 1000);
	}
}


/**
 * Called by soldiers in the building.
 */
bool MilitarySite::get_building_work(Game* g, Worker* w, bool)
{
	// Evict soldiers that have returned home if the capacity is too low
	if (m_capacity < presentSoldiers().size()) {
		w->reset_tasks(g);
		w->start_task_leavebuilding(g, true);
		return true;
	}

	return false;
}


/**
 * \return \c true if the soldier is currently present and idle in the building.
 */
bool MilitarySite::isPresent(Soldier* soldier) const
{
	return
		soldier->get_location(&owner().egbase()) == this &&
		soldier->get_state() == soldier->get_state(&Worker::taskBuildingwork) &&
		soldier->get_position() == get_position();
}

std::vector<Soldier *> MilitarySite::presentSoldiers() const
{
	std::vector<Soldier*> soldiers;

	const std::vector<Worker*>& w = get_workers();
	for
		(std::vector<Worker*>::const_iterator it = w.begin();
		 it != w.end();
		 ++it)
	{
		if (upcast(Soldier, soldier, *it)) {
			if (isPresent(soldier))
			{
				soldiers.push_back(soldier);
			}
		}
	}

	return soldiers;
}

std::vector<Soldier *> MilitarySite::stationedSoldiers() const
{
	std::vector<Soldier*> soldiers;

	const std::vector<Worker*>& w = get_workers();
	for
		(std::vector<Worker*>::const_iterator it = w.begin();
		it != w.end();
		++it)
	{
		if (upcast(Soldier, soldier, *it))
			soldiers.push_back(soldier);
	}

	return soldiers;
}

uint32_t MilitarySite::soldierCapacity() const
{
	return m_capacity;
}

void MilitarySite::setSoldierCapacity(uint32_t capacity)
{
	if (capacity > static_cast<uint32_t>(descr().get_max_number_of_soldiers()))
		capacity = descr().get_max_number_of_soldiers();

	if (capacity != m_capacity) {
		m_capacity = capacity;
		update_soldier_request();
	}
}

void MilitarySite::dropSoldier(Soldier* soldier)
{
	upcast(Game, g, &owner().egbase());
	assert(g);

	if (!isPresent(soldier)) {
		// This can happen when the "drop soldier" player command is delayed
		// by network delay or a client has bugs.
		molog("MilitarySite::dropSoldier(%u): not present\n", soldier->get_serial());
		return;
	}
	if (presentSoldiers().size() <= 1) {
		molog("cannot drop last soldier\n");
		return;
	}

	soldier->reset_tasks(g);
	soldier->start_task_leavebuilding(g, true);

	update_soldier_request();
}


void MilitarySite::conquer_area(Game & game) {
	assert(not m_didconquer);
	game.conquer_area
		(Player_Area<Area<FCoords> >
		 	(owner().get_player_number(),
		 	 Area<FCoords>
		 	 	(game.map().get_fcoords(get_position()), get_conquers())));
	m_didconquer = true;
}


/*
   MilitarySite::set_requirements

   Easy to use, overwrite with given requirements.
*/
void MilitarySite::set_requirements (const Requirements& r)
{
	m_soldier_requirements = r;
}

/*
   MilitarySite::clear_requirements

   This should cancel any requirement pushed at this house
*/
void MilitarySite::clear_requirements ()
{
	m_soldier_requirements = Requirements();
}

uint32_t MilitarySite::nr_attack_soldiers() {
	uint32_t nr_soldiers = presentSoldiers().size();
	if (nr_soldiers > 1)
		return nr_soldiers - 1;
	return 0;
}

}
