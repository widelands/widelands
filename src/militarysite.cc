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

#include "militarysite.h"

#include "battle.h"
#include "editor_game_base.h"
#include "game.h"
#include "i18n.h"
#include "player.h"
#include "profile/profile.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "soldier.h"
#include "tribe.h"
#include "worker.h"

#include "log.h"

#include "upcast.h"

#include <libintl.h>

#include <clocale>
#include <cstdio>

namespace Widelands {

MilitarySite_Descr::MilitarySite_Descr
	(char        const * const _name,
	 char        const * const _descname,
	 std::string const & directory, Profile & prof,  Section & global_s,
	 Tribe_Descr const & _tribe, EncodeData  const * const encdata)
:
	ProductionSite_Descr
		(_name, _descname, directory, prof, global_s, _tribe, encdata),
m_conquer_radius     (0),
m_num_soldiers       (0),
m_num_medics         (0),
m_heal_per_second    (0),
m_heal_incr_per_medic(0)
{
	m_conquer_radius      = global_s.get_safe_int("conquers");
	m_num_soldiers        = global_s.get_safe_int("max_soldiers");
	m_num_medics          = global_s.get_safe_int("max_medics");
	m_heal_per_second     = global_s.get_safe_int("heal_per_second");
	m_heal_incr_per_medic = global_s.get_safe_int("heal_increase_per_medic");
	if (m_conquer_radius > 0)
		m_workarea_info[m_conquer_radius].insert(descname() + _(" conquer"));
}

/**
===============
Create a new building of this type
===============
*/
Building & MilitarySite_Descr::create_object() const {
	return *new MilitarySite(*this);
}


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
m_nexthealtime(0)
{}


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
			 present, total - present);
	}
	str = buffer;

	if (m_capacity > total) {
		snprintf(buffer, sizeof(buffer), " (+%u)", m_capacity - total);
		str += buffer;
	}

	return str;
}


void MilitarySite::prefill
	(Game                 &       game,
	 uint32_t       const *       ware_counts,
	 uint32_t       const *       worker_counts,
	 Soldier_Counts const * const soldier_counts)
{
	ProductionSite::prefill(game, ware_counts, worker_counts, soldier_counts);
	if (soldier_counts and soldier_counts->size()) {
		Soldier_Descr const & soldier_descr =
			dynamic_cast<Soldier_Descr const &>
				(*tribe().get_worker_descr(tribe().worker_index("soldier")));
		container_iterate_const(Soldier_Counts, *soldier_counts, i) {
			Soldier_Strength const ss = i.current->first;
			for (uint32_t j = i.current->second; j; --j) {
				Soldier & soldier =
					static_cast<Soldier &>
						(soldier_descr.create(game, owner(), 0, get_position()));
				soldier.set_level(ss.hp, ss.attack, ss.defense, ss.evade);
				Building::add_worker(soldier);
				log
					("MilitarySite::prefill: added soldier (economy = %p)\n",
					 soldier.get_economy());
			}
		}
		conquer_area(game);
	}
}


void MilitarySite::init(Editor_Game_Base & egbase)
{
	ProductionSite::init(egbase);
	Game & game = dynamic_cast<Game &>(egbase);
	std::vector<Worker *> const & ws = get_workers();
	container_iterate_const(std::vector<Worker *>, ws, i)
		if (upcast(Soldier, soldier, *i.current)) {
			soldier->set_location_initially(*this);
			assert(not soldier->get_state()); //  Should be newly created.
			soldier->start_task_buildingwork(game);
		}
	update_soldier_request();

	//  schedule the first healing
	m_nexthealtime = game.get_gametime() + 1000;
	schedule_act(game, 1000);
}


/**
===============
MilitarySite::set_economy

Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void MilitarySite::set_economy(Economy * const e)
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
void MilitarySite::cleanup(Editor_Game_Base & egbase)
{
	// unconquer land
	if (m_didconquer)
		egbase.unconquer_area
			(Player_Area<Area<FCoords> >
			 	(owner().get_player_number(),
			 	 Area<FCoords>
			 	 	(egbase.map().get_fcoords(get_position()), get_conquers())),
			 m_defeating_player);

	ProductionSite::cleanup(egbase);

	// Note that removing workers during ProductionSite::cleanup can generate
	// new requests; that's why we delete it at the end of this function.
	delete m_soldier_request;
	m_soldier_request = 0;
}


/*
===============
Called when our soldier arrives.
===============
*/
void MilitarySite::request_soldier_callback
	(Game            &       game,
	 Request         &,
	 Ware_Index,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	MilitarySite & msite = dynamic_cast<MilitarySite &>(target);
	Soldier & s = dynamic_cast<Soldier &>(*w);

	assert(s.get_location(game) == &msite);

	if (not msite.m_didconquer)
		msite.conquer_area(game);

	// Bind the worker into this house, hide him on the map
	s.reset_tasks(game);
	s.start_task_buildingwork(game);

	// Make sure the request count is reduced or the request is deleted.
	msite.update_soldier_request();
}


/**
 * Update the request for soldiers and cause soldiers to be evicted
 * as appropriate.
 */
void MilitarySite::update_soldier_request()
{
	std::vector<Soldier *> present = presentSoldiers();
	uint32_t const stationed = stationedSoldiers().size();

	if (stationed < m_capacity) {
		if (!m_soldier_request) {
			m_soldier_request =
				new Request
					(*this,
					 tribe().safe_worker_index("soldier"),
					 MilitarySite::request_soldier_callback,
					 Request::WORKER);
			m_soldier_request->set_requirements (m_soldier_requirements);
		}

		m_soldier_request->set_count(m_capacity - stationed);
	} else {
		delete m_soldier_request;
		m_soldier_request = 0;
	}

	if (m_capacity < present.size())
		if (upcast(Game, game, &owner().egbase()))
			for (uint32_t i = 0; i < present.size() - m_capacity; ++i) {
				Soldier & soldier = *present[i];
				soldier.reset_tasks(*game);
				soldier.start_task_leavebuilding(*game, true);
			}
}


/*
===============
MilitarySite::act

Advance the program state if applicable.
===============
*/
void MilitarySite::act(Game & game, uint32_t const data)
{
	// TODO: do all kinds of stuff, but if you do nothing, let
	// ProductionSite::act() handle all this. Also note, that some ProductionSite
	// commands rely, that ProductionSite::act() is not called for a certain
	// period (like cmdAnimation). This should be reworked.
	// Maybe a new queueing system like MilitaryAct could be introduced.
	ProductionSite::act(game, data);

	if (m_nexthealtime <= game.get_gametime()) {
		uint32_t total_heal = descr().get_heal_per_second();
		std::vector<Soldier *> soldiers = presentSoldiers();

		for (uint32_t i = 0; i < soldiers.size(); ++i) {
			Soldier *s = soldiers[i];

			// The healing algorithm is totally arbitrary
			if (s->get_current_hitpoints() < s->get_max_hitpoints()) {
				s->heal(total_heal);
				total_heal -= total_heal / 3;
			}
		}

		m_nexthealtime = game.get_gametime() + 1000;
		schedule_act(game, 1000);
	}
}


/**
 * The worker is about to be removed.
 *
 * After the removal of the worker, check whether we need to request
 * new soldiers.
 */
void MilitarySite::remove_worker(Worker & w)
{
	ProductionSite::remove_worker(w);

	if (upcast(Soldier, soldier, &w))
		popSoldierJob(soldier);

	update_soldier_request();
}


/**
 * Called by soldiers in the building.
 */
bool MilitarySite::get_building_work(Game & game, Worker & worker, bool)
{
	if (upcast(Soldier, soldier, &worker)) {
		// Evict soldiers that have returned home if the capacity is too low
		if (m_capacity < presentSoldiers().size()) {
			worker.reset_tasks(game);
			worker.start_task_leavebuilding(game, true);
			return true;
		}

		bool stayhome;
		if (Map_Object * const enemy = popSoldierJob(soldier, &stayhome)) {
			if (upcast(Building, building, enemy)) {
				soldier->startTaskAttack(game, *building);
				return true;
			} else if (upcast(Soldier, opponent, enemy)) {
				if (!opponent->getBattle()) {
					soldier->startTaskDefense(game, stayhome);
					new Battle(game, *soldier, *opponent);
					return true;
				}
			} else
				throw wexception("MilitarySite::get_building_work: bad SoldierJob");
		}
	}

	return false;
}


/**
 * \return \c true if the soldier is currently present and idle in the building.
 */
bool MilitarySite::isPresent(Soldier & soldier) const
{
	return
		soldier.get_location(owner().egbase()) == this                     &&
		soldier.get_state() == soldier.get_state(Worker::taskBuildingwork) &&
		soldier.get_position() == get_position();
}

std::vector<Soldier *> MilitarySite::presentSoldiers() const
{
	std::vector<Soldier *> soldiers;

	std::vector<Worker *> const & w = get_workers();
	container_iterate_const(std::vector<Worker *>, w, i)
		if (upcast(Soldier, soldier, *i.current))
			if (isPresent(*soldier))
				soldiers.push_back(soldier);

	return soldiers;
}

std::vector<Soldier *> MilitarySite::stationedSoldiers() const
{
	std::vector<Soldier *> soldiers;

	std::vector<Worker *> const & w = get_workers();
	container_iterate_const(std::vector<Worker *>, w, i)
		if (upcast(Soldier, soldier, *i.current))
			soldiers.push_back(soldier);

	return soldiers;
}

uint32_t MilitarySite::minSoldierCapacity() const throw () {
	return 1;
}
uint32_t MilitarySite::maxSoldierCapacity() const throw () {
	return descr().get_max_number_of_soldiers();
}
uint32_t MilitarySite::soldierCapacity() const
{
	return m_capacity;
}

void MilitarySite::setSoldierCapacity(uint32_t const capacity) {
	assert(minSoldierCapacity() <= capacity);
	assert                        (capacity <= maxSoldierCapacity());
	assert(m_capacity != capacity);
	m_capacity = capacity;
	update_soldier_request();
}

void MilitarySite::dropSoldier(Soldier & soldier)
{
	Game & game = dynamic_cast<Game &>(owner().egbase());

	if (!isPresent(soldier)) {
		// This can happen when the "drop soldier" player command is delayed
		// by network delay or a client has bugs.
		molog("MilitarySite::dropSoldier(%u): not present\n", soldier.serial());
		return;
	}
	if (presentSoldiers().size() <= 1) {
		molog("cannot drop last soldier\n");
		return;
	}

	soldier.reset_tasks(game);
	soldier.start_task_leavebuilding(game, true);

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


bool MilitarySite::canAttack()
{
	return m_didconquer;
}

void MilitarySite::aggressor(Soldier & enemy)
{
	Game & game = dynamic_cast<Game &>(owner().egbase());
	Map  & map  = game.map();
	if
		(enemy.get_owner() == &owner() ||
		 enemy.getBattle() ||
		 get_conquers()
		 <=
		 map.calc_distance(enemy.get_position(), get_position()))
		return;

	if
		(map.find_bobs
		 	(Area<FCoords>(map.get_fcoords(base_flag().get_position()), 2),
		 	 0,
		 	 FindBobEnemySoldier(owner())))
		return;

	// We're dealing with a soldier that we might want to keep busy
	// Now would be the time to implement some player-definable
	// policy as to how many soldiers are allowed to leave as defenders
	std::vector<Soldier *> present = presentSoldiers();

	if (1 < present.size())
		container_iterate_const(std::vector<Soldier *>, present, i)
			if (!haveSoldierJob(**i.current)) {
				SoldierJob sj;
				sj.soldier  = *i.current;
				sj.enemy = &enemy;
				sj.stayhome = false;
				m_soldierjobs.push_back(sj);
				(*i.current)->update_task_buildingwork(game);
				return;
			}
}

bool MilitarySite::attack(Soldier & enemy)
{
	Game & game = dynamic_cast<Game &>(owner().egbase());
	std::vector<Soldier *> present = presentSoldiers();

	Soldier * defender = 0;

	if (present.size()) {
		defender = present[0];
	} else {
		// If one of our stationed soldiers is currently walking into the
		// building, give us another chance.
		std::vector<Soldier *> stationed = stationedSoldiers();
		container_iterate_const(std::vector<Soldier *>, stationed, i)
			if ((*i.current)->get_position() == get_position()) {
				defender = *i.current;
				break;
			}
	}

	if (defender) {
		popSoldierJob(defender); // defense overrides all other jobs

		SoldierJob sj;
		sj.soldier = defender;
		sj.enemy = &enemy;
		sj.stayhome = true;
		m_soldierjobs.push_back(sj);

		defender->update_task_buildingwork(game);
		return true;
	} else {
		//TODO: Conquer building
		set_defeating_player(enemy.owner().get_player_number());
		schedule_destroy(game);
		return false;
	}
}


/*
   MilitarySite::set_requirements

   Easy to use, overwrite with given requirements.
*/
void MilitarySite::set_requirements (Requirements const & r)
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

void MilitarySite::sendAttacker(Soldier & soldier, Building & target)
{
	assert(isPresent(soldier));

	if (haveSoldierJob(soldier))
		return;

	SoldierJob sj;
	sj.soldier  = &soldier;
	sj.enemy    = &target;
	sj.stayhome = false;
	m_soldierjobs.push_back(sj);

	soldier.update_task_buildingwork(dynamic_cast<Game &>(owner().egbase()));
}


bool MilitarySite::haveSoldierJob(Soldier & soldier)
{
	container_iterate_const(std::vector<SoldierJob>, m_soldierjobs, i)
		if (i.current->soldier == &soldier)
			return true;

	return false;
}


/**
 * \return the enemy, if any, that the given soldier was scheduled
 * to attack, and remove the job.
 */
Map_Object * MilitarySite::popSoldierJob
	(Soldier * const soldier, bool * const stayhome)
{
	container_iterate(std::vector<SoldierJob>, m_soldierjobs, i)
		if (i.current->soldier == soldier) {
			Map_Object * const enemy = i.current->enemy.get(owner().egbase());
			if (stayhome)
				*stayhome = i.current->stayhome;
			m_soldierjobs.erase(i.current);
			return enemy;
		}
	return 0;
}

}
