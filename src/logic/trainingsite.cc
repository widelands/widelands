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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/trainingsite.h"

#include <cstdio>

#include "economy/request.h"
#include "helper.h"
#include "i18n.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/production_program.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/worker.h"
#include "profile/profile.h"
#include "upcast.h"


namespace Widelands {

const uint32_t TrainingSite::training_state_multiplier = 12;

TrainingSite_Descr::TrainingSite_Descr
	(char const * const _name, char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 const Tribe_Descr & _tribe)
	:
	ProductionSite_Descr
		(_name, _descname, directory, prof, global_s, _tribe),

	//  FIXME This is currently hardcoded for "soldier" but should allow any
	//  FIXME soldier type name.
	m_num_soldiers      (global_s.get_safe_int("soldier_capacity")),
	m_max_stall (global_s.get_safe_int("trainer_patience")),

m_train_hp          (false),
m_train_attack      (false),
m_train_defense     (false),
m_train_evade       (false),
m_min_hp            (0),
m_min_attack        (0),
m_min_defense       (0),
m_min_evade         (0),
m_max_hp            (0),
m_max_attack        (0),
m_max_defense       (0),
m_max_evade         (0)
{
	// Read the range of levels that can update this building
	//  FIXME This is currently hardcoded to "soldier" but it should search for
	//  FIXME sections starting with the name of each soldier type.
	//  FIXME These sections also seem redundant. Eliminate them (having the
	//  FIXME programs should be enough).
	if (Section * const s = prof.get_section("soldier hp")) {
		m_train_hp      = true;
		m_min_hp        = s->get_safe_int("min_level");
		m_max_hp        = s->get_safe_int("max_level");
	}
	if (Section * const s = prof.get_section("soldier attack")) {
		m_train_attack  = true;
		m_min_attack    = s->get_safe_int("min_level");
		m_max_attack    = s->get_safe_int("max_level");
	}
	if (Section * const s = prof.get_section("soldier defense")) {
		m_train_defense = true;
		m_min_defense   = s->get_safe_int("min_level");
		m_max_defense   = s->get_safe_int("max_level");
	}
	if (Section * const s = prof.get_section("soldier evade")) {
		m_train_evade   = true;
		m_min_evade     = s->get_safe_int("min_level");
		m_max_evade     = s->get_safe_int("max_level");
	}
}

/**
 * Create a new training site
 * \return  the new training site
 */
Building & TrainingSite_Descr::create_object() const {
	return *new TrainingSite(*this);
}

/**
 * \param at the attribute to investigate
 * \return  the minimum level to which this building can downgrade a
 * specified attribute
 */
int32_t TrainingSite_Descr::get_min_level(const tAttribute at) const {
	switch (at) {
	case atrHP:
		return m_min_hp;
	case atrAttack:
		return m_min_attack;
	case atrDefense:
		return m_min_defense;
	case atrEvade:
		return m_min_evade;
	default:
		throw wexception("Unknown attribute value!");
	}
}

/**
 * Returns the maximum level to which this building can upgrade a
 * specified attribute
 * \param at  the attribute to investigate
 * \return  the maximum level to be attained at this site
 */
int32_t TrainingSite_Descr::get_max_level(const tAttribute at) const {
	switch (at) {
	case atrHP:
		return m_max_hp;
	case atrAttack:
		return m_max_attack;
	case atrDefense:
		return m_max_defense;
	case atrEvade:
		return m_max_evade;
	default:
		throw wexception("Unknown attribute value!");
	}
}

int32_t
TrainingSite_Descr::get_max_stall() const
{
	return m_max_stall;
}

/*
=============================

class TrainingSite

=============================
*/

TrainingSite::TrainingSite(const TrainingSite_Descr & d) :
ProductionSite   (d),
m_soldier_request(nullptr),
m_capacity       (descr().get_max_number_of_soldiers()),
m_build_heroes    (false),
m_result         (Failed)
{
	// Initialize this in the constructor so that loading code may
	// overwrite priorities.
	calc_upgrades();
	m_current_upgrade = nullptr;
	set_post_timer(6000);
	training_failure_count.clear();
	max_stall_val = training_state_multiplier * d.get_max_stall();

	if (d.get_train_hp())
		init_kick_state(atrHP, d);
	if (d.get_train_attack())
		init_kick_state(atrAttack, d);
	if (d.get_train_defense())
		init_kick_state(atrDefense, d);
	if (d.get_train_evade())
		init_kick_state(atrEvade, d);
}
void
TrainingSite::init_kick_state(const tAttribute & art, const TrainingSite_Descr & d)
{
		// Now with kick-out state saving implemented, initializing is an overkill
		for (int t = d.get_min_level(art); t <= d.get_max_level(art); t++)
			trainingAttempted(art, t);
}


/**
 * Retrieve the training program that is currently running.
 * \return  the name of the current program
 */
std::string TrainingSite::get_statistics_string()
{
	return ProductionSite::get_statistics_string();
}

/**
 * Setup the building and request soldiers
 */
void TrainingSite::init(Editor_Game_Base & egbase)
{
	ProductionSite::init(egbase);

	upcast(Game, game, &egbase);

	container_iterate_const(std::vector<Soldier *>, m_soldiers, i) {
		(*i.current)->set_location_initially(*this);
		assert(not (*i.current)->get_state()); //  Should be newly created.

		if (game)
			(*i.current)->start_task_idle(*game, 0, -1);
	}
	update_soldier_request();
}

/**
 * Change the economy this site belongs to.
 * \par e  The new economy. Can be 0 (unconnected buildings have no economy).
 * \note the worker (but not the soldiers) is dealt with in the
 * PlayerImmovable code.
 */
void TrainingSite::set_economy(Economy * e)
{
	ProductionSite::set_economy(e);

	if (m_soldier_request)
		m_soldier_request->set_economy(e);
}

/**
 * Cleanup after a Training site is removed
 *
 * Cancel all soldier requests and release all soldiers
 */
void TrainingSite::cleanup(Editor_Game_Base & egbase)
{
	delete m_soldier_request;
	m_soldier_request = nullptr;

	ProductionSite::cleanup(egbase);
}


void TrainingSite::add_worker(Worker & w)
{
	ProductionSite::add_worker(w);

	if (upcast(Soldier, soldier, &w)) {
		// Note that the given Soldier might already be in the array
		// for loadgames.
		if
			(std::find(m_soldiers.begin(), m_soldiers.end(), soldier) ==
			 m_soldiers.end())
			m_soldiers.push_back(soldier);

		if (upcast(Game, game, &owner().egbase()))
			schedule_act(*game, 100);
	}
}

void TrainingSite::remove_worker(Worker & w)
{
	upcast(Game, game, &owner().egbase());

	if (upcast(Soldier, soldier, &w)) {
		std::vector<Soldier *>::iterator const it =
			std::find(m_soldiers.begin(), m_soldiers.end(), soldier);
		if (it != m_soldiers.end()) {
			m_soldiers.erase(it);

			if (game)
				schedule_act(*game, 100);
		}
	}

	ProductionSite::remove_worker(w);
}


/**
 * Request soldiers up to capacity, or let go of surplus soldiers.
 */
void TrainingSite::update_soldier_request() {
	if (m_soldiers.size() < m_capacity) {
		if (!m_soldier_request) {
			m_soldier_request =
				new Request
					(*this,
					 tribe().safe_worker_index("soldier"),
					 TrainingSite::request_soldier_callback,
					 wwWORKER);

			RequireOr r;

			// set requirements to match this site
			if (descr().get_train_attack())
				r.add
					(RequireAttribute
					 	(atrAttack,
					 	 descr().get_min_level(atrAttack),
					 	 descr().get_max_level(atrAttack)));
			if (descr().get_train_defense())
				r.add
					(RequireAttribute
					 	(atrDefense,
					 	 descr().get_min_level(atrDefense),
					 	 descr().get_max_level(atrDefense)));
			if (descr().get_train_evade())
				r.add
					(RequireAttribute
					 	(atrEvade,
					 	 descr().get_min_level(atrEvade),
					 	 descr().get_max_level(atrEvade)));
			if (descr().get_train_hp())
				r.add
					(RequireAttribute
					 	(atrHP,
					 	 descr().get_min_level(atrHP),
					 	 descr().get_max_level(atrHP)));

			m_soldier_request->set_requirements(r);
		}

		m_soldier_request->set_count(m_capacity - m_soldiers.size());
	} else if (m_soldiers.size() >= m_capacity) {
		delete m_soldier_request;
		m_soldier_request = nullptr;

		while (m_soldiers.size() > m_capacity)
			dropSoldier(**m_soldiers.rbegin());
	}
}


/**
 * Soldier callback. Since the soldier was already added via add_worker,
 * we only need to update the request structure.
 */
void TrainingSite::request_soldier_callback
	(Game            &       game,
#ifndef NDEBUG
	 Request         &       rq,
#else
	 Request         &,
#endif
	 Ware_Index,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	TrainingSite & tsite = ref_cast<TrainingSite, PlayerImmovable>(target);
	Soldier      & s     = ref_cast<Soldier,      Worker>         (*w);

	assert(s.get_location(game) == &tsite);
	assert(tsite.m_soldier_request == &rq);

	tsite.incorporateSoldier(game, s);
}

/*
===============
Takes one soldier and adds him to ours

returns 0 on succes, -1 if there was no room for this soldier
===============
*/
int TrainingSite::incorporateSoldier(Editor_Game_Base & egbase, Soldier & s) {
	if (s.get_location(egbase) != this) {
		if (stationedSoldiers().size() + 1 > descr().get_max_number_of_soldiers())
			return -1;

		s.set_location(this);
	}

	// Bind the worker into this house, hide him on the map
	if (upcast(Game, game, &egbase))
		s.start_task_idle(*game, 0, -1);

	// Make sure the request count is reduced or the request is deleted.
	update_soldier_request();

	return 0;
}


std::vector<Soldier *> TrainingSite::presentSoldiers() const
{
	return m_soldiers;
}

std::vector<Soldier *> TrainingSite::stationedSoldiers() const
{
	return m_soldiers;
}

uint32_t TrainingSite::minSoldierCapacity() const {
	return 0;
}
uint32_t TrainingSite::maxSoldierCapacity() const {
	return descr().get_max_number_of_soldiers();
}
uint32_t TrainingSite::soldierCapacity() const
{
	return m_capacity;
}

void TrainingSite::setSoldierCapacity(uint32_t const capacity) {
	assert(minSoldierCapacity() <= capacity);
	assert                        (capacity <= maxSoldierCapacity());
	assert(m_capacity != capacity);
	m_capacity = capacity;
	update_soldier_request();
}

/**
 * Drop a given soldier.
 *
 * 'Dropping' means releasing the soldier from the site. The soldier then
 * becomes available to the economy.
 *
 * \note This is called from player commands, so we need to verify that the
 * soldier is actually stationed here, without breaking anything if he isn't.
 */
void TrainingSite::dropSoldier(Soldier & soldier)
{
	Game & game = ref_cast<Game, Editor_Game_Base>(owner().egbase());

	std::vector<Soldier *>::iterator it =
		std::find(m_soldiers.begin(), m_soldiers.end(), &soldier);
	if (it == m_soldiers.end()) {
		molog("TrainingSite::dropSoldier: soldier not in training site");
		return;
	}

	m_soldiers.erase(it);

	soldier.reset_tasks(game);
	soldier.start_task_leavebuilding(game, true);

	// Schedule, so that we can call new soldiers on next act()
	schedule_act(game, 100);
}


/**
 * Drop all the soldiers that can not be upgraded further at this building.
 */
void TrainingSite::drop_unupgradable_soldiers(Game &)
{
	std::vector<Soldier *> droplist;

	for (uint32_t i = 0; i < m_soldiers.size(); ++i) {
		std::vector<Upgrade>::iterator it = m_upgrades.begin();
		for (; it != m_upgrades.end(); ++it) {
			int32_t level = m_soldiers[i]->get_level(it->attribute);
			if (level >= it->min && level <= it->max)
				break;
		}

		if (it == m_upgrades.end())
			droplist.push_back(m_soldiers[i]);
	}

	// Drop soldiers only now, so that changes in the soldiers array don't
	// mess things up
	container_iterate_const(std::vector<Soldier *>, droplist, i)
		dropSoldier(**i.current);
}

/**
 * Drop all the soldiers that can not be upgraded further at this level of resourcing.
 *
 */
void TrainingSite::drop_stalled_soldiers(Game &)
{
	Soldier * soldier_to_drop = nullptr;
	uint32_t highest_soldier_level_seen = 0;

	for (uint32_t i = 0; i < m_soldiers.size(); ++i)
	{
		uint32_t this_soldier_level = m_soldiers[i]->get_level(atrTotal);

		bool this_soldier_is_safe = false;
		if (this_soldier_level <= highest_soldier_level_seen)
		{
			// Skip the innermost loop for soldiers that would not be kicked out anyway.
			// level-zero soldiers are excepted from kick-out implicitly. This is intentional.
			this_soldier_is_safe = true;
		}
		else
		{
			std::vector<Upgrade>::iterator it = m_upgrades.begin();
			for (; it != m_upgrades.end(); ++it)
			if  (! this_soldier_is_safe)
			{
				// Soldier is safe, if he:
				//  - is below maximum, and
				//  - is not in a stalled state
				// Check done separately for each art.
				int32_t level = m_soldiers[i]->get_level(it->attribute);

				 // Below maximum -check
				if (level > it->max)
				{
					break;
				}

				TypeAndLevel_t train_tl(it->attribute, level);
				TrainFailCount_t::iterator tstep = training_failure_count.find(train_tl);
				if (tstep ==  training_failure_count.end())
					{
						log("\nTrainingSite::drop_stalled_soldiers: ");
						log("training step %d,%d not found in this school!\n", it->attribute, level);
						break;
					}

				tstep->second.second = 1; // a soldier is present at this level

				// Stalled state -check
				if (max_stall_val > tstep->second.first)
				{
					this_soldier_is_safe = true;
					break;
				}
			}
		}
		if (!this_soldier_is_safe)
		{
			// Make this soldier a kick-out candidate
			soldier_to_drop = m_soldiers[i];
			highest_soldier_level_seen = this_soldier_level;
		}
	}

	// Finally drop the soldier.
	if (nullptr != soldier_to_drop)
		{
			log("TrainingSite::drop_stalled_soldiers: Kicking somebody out.\n");
			dropSoldier (*soldier_to_drop);
		}
}




/**
 * In addition to advancing the program, update soldier status.
 */
void TrainingSite::act(Game & game, uint32_t const data)
{
	ProductionSite::act(game, data);

	update_soldier_request();
}


void TrainingSite::program_end(Game & game, Program_Result const result)
{
	m_result = result;
	ProductionSite::program_end(game, result);

	if (m_current_upgrade) {
		if (m_result == Completed) {
			drop_unupgradable_soldiers(game);
			m_current_upgrade->lastsuccess = true;
			m_current_upgrade->failures = 0;
		}
		else {
			m_current_upgrade->failures++;
			drop_stalled_soldiers(game);
		}
		m_current_upgrade = nullptr;
	}
	trainingDone();
}


/**
 * Find and start the next training program.
 *
 * Prioritize such that if UpgradeA.prio is twice UpgradeB.prio, then
 * start_upgrade will be called twice as often for UpgradeA.
 * If all priorities are zero, nothing will happen.
 */
void TrainingSite::find_and_start_next_program(Game & game)
{
	for (;;) {
		uint32_t maxprio = 0;
		uint32_t maxcredit = 0;

		container_iterate(std::vector<Upgrade>, m_upgrades, i) {
			if (i.current->credit >= 10) {
				i.current->credit -= 10;
				return start_upgrade(game, *i.current);
			}

			if (maxprio   < i.current->prio)
				maxprio    = i.current->prio;
			if (maxcredit < i.current->credit)
				maxcredit  = i.current->credit;
		}

		if (maxprio == 0)
			return program_start(game, "sleep");

		uint32_t const multiplier = 1 + (10 - maxcredit) / maxprio;

		container_iterate(std::vector<Upgrade>, m_upgrades, i)
			i.current->credit += multiplier * i.current->prio;
	}
}


/**
 * The prioritizer decided that the given type of upgrade should run.
 * Let's do our worst.
 */
void TrainingSite::start_upgrade(Game & game, Upgrade & upgrade)
{
	int32_t minlevel = upgrade.max;
	int32_t maxlevel = upgrade.min;

	container_iterate_const(std::vector<Soldier *>, m_soldiers, i) {
		int32_t const level = (*i.current)->get_level(upgrade.attribute);

		if (level > upgrade.max || level < upgrade.min)
			continue;
		if (level < minlevel)
			minlevel = level;
		if (level > maxlevel)
			maxlevel = level;
	}

	if (minlevel > maxlevel)
		return program_start(game, "sleep");

	int32_t level;

	if (upgrade.lastsuccess || upgrade.lastattempt < 0) {
		// Start greedily on the first ever attempt, and restart greedily
		// after a sucessful upgrade
		if (m_build_heroes)
			level = maxlevel;
		else
			level = minlevel;
	} else {
		// The last attempt wasn't successful;
		// This happens e.g. when lots of low-level soldiers are present,
		// but the prerequisites for improving them aren't.
		if (m_build_heroes) {
			level = upgrade.lastattempt - 1;
			if (level < minlevel)
				level = maxlevel;
		} else {
			level = upgrade.lastattempt + 1;
			if (level > maxlevel)
				level = minlevel;
		}
	}

	m_current_upgrade = &upgrade;
	upgrade.lastattempt = level;
	upgrade.lastsuccess = false;

	char buf[200];
	sprintf(buf, "%s%d", upgrade.prefix.c_str(), level);
	return program_start(game, buf);
}

TrainingSite::Upgrade * TrainingSite::get_upgrade(tAttribute const atr)
{
	container_iterate(std::vector<Upgrade>, m_upgrades, i)
		if (i.current->attribute == atr)
			return &*i.current;

	return nullptr;
}


/**
 * Gets the priority of given attribute
 */
int32_t TrainingSite::get_pri(tAttribute atr)
{
	container_iterate_const(std::vector<Upgrade>, m_upgrades, i)
		if (i.current->attribute == atr)
			return i.current->prio;

	return 0;
}

/**
 * Sets the priority of given attribute
 */
void TrainingSite::set_pri(tAttribute atr, int32_t prio)
{
	if (prio < 0)
		prio = 0;

	container_iterate(std::vector<Upgrade>, m_upgrades, i)
		if (i.current->attribute == atr) {
			i.current->prio = prio;
			return;
		}
}

/**
 * Only called from \ref calc_upgrades
 */
void TrainingSite::add_upgrade
	(tAttribute const atr, const std::string & prefix)
{
	Upgrade u;
	u.attribute = atr;
	u.prefix = prefix;
	u.min = descr().get_min_level(atr);
	u.max = descr().get_max_level(atr);
	u.prio = 6;
	u.credit = 0;
	u.lastattempt = -1;
	u.lastsuccess = false;
	u.failures = 0;
	m_upgrades.push_back(u);
}

/**
 * Called once at initialization to populate \ref m_upgrades.
 */
void TrainingSite::calc_upgrades() {
	assert(m_upgrades.empty());

	//  FIXME This is currently hardcoded for "soldier" but it should allow any
	//  FIXME soldier type name.
	if (descr().get_train_hp())
		add_upgrade(atrHP, "upgrade_soldier_hp_");
	if (descr().get_train_attack())
		add_upgrade(atrAttack, "upgrade_soldier_attack_");
	if (descr().get_train_defense())
		add_upgrade(atrDefense, "upgrade_soldier_defense_");
	if (descr().get_train_evade())
		add_upgrade(atrEvade, "upgrade_soldier_evade_");
}


void
TrainingSite::trainingAttempted(uint32_t type, uint32_t level)
	{
	        TypeAndLevel_t key(type, level);
		if (training_failure_count.find(key) == training_failure_count.end())
			training_failure_count[key]  = std::make_pair(training_state_multiplier, 0);
		else
			training_failure_count[key].first +=  training_state_multiplier;
	}

/**
 * Called whenever it was possible to promote another guy
 */

void
TrainingSite::trainingSuccessful(uint32_t type, uint32_t level)
{
	TypeAndLevel_t key(type, level);
	// Here I assume that key exists: training has been attempted before it can succeed.
	training_failure_count[key].first = 0;
}

void
TrainingSite::trainingDone()
{
	TrainFailCount_t::iterator it;
	for (it = training_failure_count.begin(); it != training_failure_count.end(); it++)
	{
		// If a soldier is present at this training level, deteoriate
		if (it->second.second)
		{
			it->second.first++;
			it->second.second = 0;
		}
		else // If no soldier, let's become optimistic
		if (0 < it->second.first)
			it->second.first--;
	}
}

}
