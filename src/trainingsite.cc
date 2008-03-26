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

#include "trainingsite.h"

#include "editor_game_base.h"
#include "game.h"
#include "helper.h"
#include "i18n.h"
#include "player.h"
#include "production_program.h"
#include "profile.h"
#include "request.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "worker.h"

#include "upcast.h"

#include <stdio.h>

namespace Widelands {

TrainingSite_Descr::TrainingSite_Descr
(const Tribe_Descr & tribe_descr, const std::string & trainingsite_name) :
ProductionSite_Descr(tribe_descr, trainingsite_name),
m_num_soldiers      (0),
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
{}

TrainingSite_Descr::~TrainingSite_Descr() {}

/**
 * Parse the additional information necessary for miltary buildings
 * \param directory  where to find the description file
 * \param prof       the configuration profile to read
 * \param encdata    defaults for building animations' color codes (e.g.
 *                   keycolor, shadow color). \sa EncodeData::parse()
 * \todo Is the encdata stuff still valid with the new transparent-png support?
 */
void TrainingSite_Descr::parse(const char *directory, Profile * prof, const EncodeData * encdata)
{
	Section *sglobal;

	assert(directory);
	assert(prof);

	ProductionSite_Descr::parse(directory, prof, encdata);
	sglobal = prof->get_section("global");
	//TODO: what if there is no global section? can this happen?

	m_stopable = true; // (defaults to false)
	m_num_soldiers = sglobal->get_safe_int("max_soldiers");

	const std::vector<std::string> str_list
		(split_string(sglobal->get_safe_string("train"), ","));
	const std::vector<std::string>::const_iterator str_list_end = str_list.end();
	for
		(std::vector<std::string>::const_iterator it = str_list.begin();
		 it != str_list_end;
		 ++it)
		if      (*it == "hp")      m_train_hp      = true;
		else if (*it == "attack")  m_train_attack  = true;
		else if (*it == "defense") m_train_defense = true;
		else if (*it == "evade")   m_train_evade   = true;
		else
			throw wexception
				("Attribute %s isn't known as a valid attribute", it->c_str());

	// Read the range of levels that can update this building
	if (m_train_hp) {
		Section *sect = prof->get_section("hp");
		assert(sect);
		m_min_hp = sect->get_safe_int("min_level");
		m_max_hp = sect->get_safe_int("max_level");
	}
	if (m_train_attack) {
		Section *sect = prof->get_section("attack");
		assert(sect);
		m_min_attack = sect->get_safe_int("min_level");
		m_max_attack = sect->get_safe_int("max_level");
	}
	if (m_train_defense) {
		Section *sect = prof->get_section("defense");
		assert(sect);
		m_min_defense = sect->get_safe_int("min_level");
		m_max_defense = sect->get_safe_int("max_level");
	}
	if (m_train_evade) {
		Section *sect = prof->get_section("evade");
		assert(sect);
		m_min_evade = sect->get_safe_int("min_level");
		m_max_evade = sect->get_safe_int("max_level");
	}
}

/**
 * Create a new training site
 * \return  the new training site
 */
Building * TrainingSite_Descr::create_object() const
{return new TrainingSite(*this);}

/**
 * \param at the attribute to investigate
 * \return  the minimum level to which this building can downgrade a specified attribute
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
 * Returns the maximum level to which this building can upgrade a specified attribute
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

/*
=============================

class TrainingSite

=============================
*/

TrainingSite::TrainingSite(const TrainingSite_Descr & d) :
ProductionSite   (d),
m_soldier_request(0),
m_capacity       (descr().get_max_number_of_soldiers()),
m_build_heros    (false),
m_success        (false)
{
	// Initialize this in the constructor so that loading code may
	// overwrite priorities.
	calc_upgrades();
	m_current_upgrade = 0;
	set_post_timer(6000);
}


TrainingSite::~TrainingSite() {}


/**
 * Retrieve the training program that is currently running.
 * \return  the name of the current program
 */
std::string TrainingSite::get_statistics_string()
{
	State *state;
	state = get_current_program(); //may also be NULL if there is no current program

	if (state) {
		return state->program->get_name();
	} else if (m_success)
		return _("Resting");
	else
		return _("Not Working");
}


/**
 * Setup the building and request soldiers
 */
void TrainingSite::init(Editor_Game_Base * g)
{
	assert(g);

	ProductionSite::init(g);
	update_soldier_request();
}

/**
 * Change the economy this site belongs to.
 * \par e  The new economy. Can be 0 (unconnected buildings have no economy).
 * \note the worker (but not the soldiers) is dealt with in the PlayerImmovable code.
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
 * \todo code audit, cf. change_soldier_capacity()
 */
void TrainingSite::cleanup(Editor_Game_Base * g)
{
	assert(g);

	delete m_soldier_request;
	m_soldier_request = 0;

	ProductionSite::cleanup(g);
}


void TrainingSite::add_worker(Worker* w)
{
	ProductionSite::add_worker(w);

	if (upcast(Soldier, soldier, w)) {
		// Note that the given Soldier might already be in the array
		// for loadgames.
		if (std::find(m_soldiers.begin(), m_soldiers.end(), soldier) == m_soldiers.end())
			m_soldiers.push_back(soldier);
		if (upcast(Game, game, &owner().egbase()))
			schedule_act(game, 100);
	}
}

void TrainingSite::remove_worker(Worker* w)
{
	if (upcast(Soldier, soldier, w)) {
		std::vector<Soldier*>::iterator it = std::find(m_soldiers.begin(), m_soldiers.end(), soldier);
		if (it != m_soldiers.end()) {
			m_soldiers.erase(it);
			if (upcast(Game, game, &owner().egbase()))
				schedule_act(game, 100);
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
			int32_t soldierid = get_owner()->tribe().get_safe_worker_index("soldier");
			m_soldier_request = new Request
				(this, soldierid, &TrainingSite::request_soldier_callback, this, Request::WORKER);

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
		m_soldier_request = 0;

		while (m_soldiers.size() > m_capacity)
			drop_soldier(m_soldiers[m_soldiers.size()-1]);
	}
}


/**
 * Soldier callback. Since the soldier was already added via add_worker,
 * we only need to update the request structure.
 */
void TrainingSite::request_soldier_callback
(Game * g, Request * rq, Ware_Index, Worker * w, void * data)
{
	TrainingSite* const tsite = static_cast<TrainingSite *>(data);
	Soldier* s = dynamic_cast<Soldier*>(w);

	assert(s->get_location(g) == tsite);
	assert(tsite->m_soldier_request == rq);

	// bind the worker into this house, hide him on the map
	s->start_task_idle(g, 0, -1);

	g->conquer_area
		(Player_Area<Area<FCoords> >
		 	(tsite->owner().get_player_number(),
		 	 Area<FCoords>
		 	 	(g->map().get_fcoords(tsite->get_position()),
		 	 	 tsite->descr().get_conquers())));

	tsite->update_soldier_request();
}


/**
 * Drop a given soldier.
 *
 * 'Dropping' means releasing the soldier from the site. The soldier then becomes available
 * to the economy.
 */
void TrainingSite::drop_soldier(uint32_t serial)
{
	for (std::vector<Soldier*>::iterator it = m_soldiers.begin(); it != m_soldiers.end(); ++it) {
		if ((*it)->get_serial() == serial) {
			drop_soldier(*it);
			return;
		}
	}

	molog
		("TrainingSite::drop_soldier(uint32_t serial): trying to drop nonexistent "
		 "serial number %i !!",
		 serial);
}


/**
 * Drop a given soldier.
 *
 * 'Dropping' means releasing the soldier from the site. The soldier then becomes available
 * to the economy.
 */
void TrainingSite::drop_soldier(Soldier* soldier)
{
	upcast(Game, g, &owner().egbase());

	assert(g);

	std::vector<Soldier*>::iterator it = std::find(m_soldiers.begin(), m_soldiers.end(), soldier);
	if (it == m_soldiers.end())
		throw wexception("TrainingSite::drop_soldier: soldier not in training site");

	m_soldiers.erase(it);

	soldier->reset_tasks(g);
	soldier->start_task_leavebuilding(g, true);

	// Schedule, so that we can call new soldiers on next act()
	schedule_act(g, 100);
}


/**
 * Drop all the soldiers that can not be upgraded further at this building.
 */
void TrainingSite::drop_unupgradable_soldiers(Game *)
{
	std::vector<Soldier*> droplist;

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
	for (std::vector<Soldier*>::iterator it = droplist.begin(); it != droplist.end(); ++it)
		drop_soldier(*it);
}

/**
 * In addition to advancing the program, update soldier status.
 */
void TrainingSite::act(Game * g, uint32_t data)
{
	assert(g);

	ProductionSite::act(g, data);

	update_soldier_request();
}


void TrainingSite::program_end(Game* g, bool success)
{
	m_success = success;
	ProductionSite::program_end(g, success);

	if (m_current_upgrade) {
		if (m_success) {
			drop_unupgradable_soldiers(g);
			m_current_upgrade->lastsuccess = true;
		}
		m_current_upgrade = 0;
	}
}


/**
 * Find and start the next training program.
 *
 * Prioritize such that if UpgradeA.prio is twice UpgradeB.prio, then
 * start_upgrade will be called twice as often for UpgradeA.
 * If all priorities are zero, nothing will happen.
 */
void TrainingSite::find_and_start_next_program(Game * g)
{
	for (;;) {
		uint32_t maxprio = 0;
		uint32_t maxcredit = 0;

		for (std::vector<Upgrade>::iterator it = m_upgrades.begin(); it != m_upgrades.end(); ++it) {
			if (it->credit >= 10) {
				it->credit -= 10;
				start_upgrade(g, &*it);
				return;
			}

			if (it->prio > maxprio)
				maxprio = it->prio;
			if (it->credit > maxcredit)
				maxcredit = it->credit;
		}

		if (maxprio == 0) {
			program_start(g, "Sleep");
			return;
		}

		uint32_t multiplier = 1 + (10-maxcredit) / maxprio;

		for (std::vector<Upgrade>::iterator it = m_upgrades.begin(); it != m_upgrades.end(); ++it)
			it->credit += multiplier * it->prio;
	}
}


/**
 * The prioritizer decided that the given type of upgrade should run.
 * Let's do our worst.
 */
void TrainingSite::start_upgrade(Game* g, Upgrade* upgrade)
{
	int32_t minlevel = upgrade->max;
	int32_t maxlevel = upgrade->min;

	for (std::vector<Soldier*>::const_iterator it = m_soldiers.begin(); it != m_soldiers.end(); ++it) {
		int32_t level = (*it)->get_level(upgrade->attribute);

		if (level > upgrade->max || level < upgrade->min)
			continue;
		if (level < minlevel)
			minlevel = level;
		if (level > maxlevel)
			maxlevel = level;
	}

	if (minlevel > maxlevel) {
		program_start(g, "Sleep");
		return;
	}

	int32_t level;

	if (upgrade->lastsuccess || upgrade->lastattempt < 0) {
		// Start greedily on the first ever attempt, and restart greedily
		// after a sucessful upgrade
		if (m_build_heros)
			level = maxlevel;
		else
			level = minlevel;
	} else {
		// The last attempt wasn't successful;
		// This happens e.g. when lots of low-level soldiers are present,
		// but the prerequisites for improving them aren't.
		if (m_build_heros) {
			level = upgrade->lastattempt - 1;
			if (level < minlevel)
				level = maxlevel;
		} else {
			level = upgrade->lastattempt + 1;
			if (level > maxlevel)
				level = minlevel;
		}
	}

	m_current_upgrade = upgrade;
	upgrade->lastattempt = level;
	upgrade->lastsuccess = false;

	char buf[200];
	sprintf(buf, "%s%d", upgrade->prefix.c_str(), level);
	program_start(g, buf);
}

TrainingSite::Upgrade* TrainingSite::get_upgrade(enum tAttribute atr)
{
	for (std::vector<Upgrade>::iterator it = m_upgrades.begin(); it != m_upgrades.end(); ++it) {
		if (it->attribute == atr)
			return &*it;
	}

	return 0;
}


/**
 * Gets the priority of given attribute
 */
int32_t TrainingSite::get_pri(tAttribute atr)
{
	for (std::vector<Upgrade>::const_iterator it = m_upgrades.begin(); it != m_upgrades.end(); ++it) {
		if (it->attribute == atr)
			return it->prio;
	}

	return 0;
}

/**
 * Sets the priority of given attribute
 */
void TrainingSite::set_pri(tAttribute atr, int32_t prio)
{
	if (prio < 0)
		prio = 0;

	for (std::vector<Upgrade>::iterator it = m_upgrades.begin(); it != m_upgrades.end(); ++it) {
		if (it->attribute == atr) {
			it->prio = prio;
			return;
		}
	}
}

/**
 * Change the soldier capacity at the trainingsite.
 * \post Minimum and maximum capacity will be observed.
 * \param how  number to add/subtract from the current capacity
 * \note Unlike the influence-defining military buildings, a training site can actually be empty of soldiers.
 *
 */
void TrainingSite::change_soldier_capacity(int32_t how)
{
	int32_t new_capacity = m_capacity + how;

	if (new_capacity < 0)
		new_capacity = 0;
	else if (new_capacity > descr().get_max_number_of_soldiers())
		new_capacity = descr().get_max_number_of_soldiers();

	if (static_cast<uint32_t>(new_capacity) != m_capacity) {
		m_capacity = new_capacity;
		update_soldier_request();
	}
}


/**
 * Only called from \ref calc_upgrades
 */
void TrainingSite::add_upgrade(tAttribute atr, const std::string& prefix)
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
	m_upgrades.push_back(u);
}

/**
 * Called once at initialization to populate \ref m_upgrades.
 */
void TrainingSite::calc_upgrades() {
	assert(m_upgrades.size() == 0);

	if (descr().get_train_hp())
		add_upgrade(atrHP, "upgrade_hp_");
	if (descr().get_train_attack())
		add_upgrade(atrAttack, "upgrade_attack_");
	if (descr().get_train_defense())
		add_upgrade(atrDefense, "upgrade_defense_");
	if (descr().get_train_evade())
		add_upgrade(atrEvade, "upgrade_evade_");
}

};
