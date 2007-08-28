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

#include "trainingsite.h"

#include "editor_game_base.h"
#include "game.h"
#include "helper.h"
#include "i18n.h"
#include "player.h"
#include "production_program.h"
#include "profile.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "worker.h"

#include <stdio.h>

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

TrainingSite_Descr::~TrainingSite_Descr()
{
}

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
	std::string trainable;
	std::vector < std::string > str_list;

	assert(directory);
	assert(prof);

	ProductionSite_Descr::parse(directory, prof, encdata);
	sglobal = prof->get_section("global");
	//TODO: what if there is no global section? can this happen?

	m_stopable = true; // (defaults to false)
	m_num_soldiers = sglobal->get_safe_int("max_soldiers");

	trainable = sglobal->get_safe_string("train");
	split_string(trainable, str_list, ",");

	while (str_list.size()) {
		if (str_list[0] == "hp")
			m_train_hp = true;
		else if (str_list[0] == "attack")
			m_train_attack = true;
		else if (str_list[0] == "defense")
			m_train_defense = true;
		else if (str_list[0] == "evade")
			m_train_evade = true;
		else
			throw wexception("Attribute %s isn't known as a valid attribute", str_list[0].c_str());
		str_list.erase(str_list.begin());
	}

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
 * Returns the minimum level to which this building can downgrade a specified attribute
 * \param at  the attribute to investigate
 * \return  the minimum level to be attained at this site
 */
int TrainingSite_Descr::get_min_level(const tAttribute at) const {
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
		throw wexception("Unkown attribute value!");
	}
}

/**
 * Returns the maximum level to which this building can upgrade a specified attribute
 * \param at  the attribute to investigate
 * \return  the maximum level to be attained at this site
 */
int TrainingSite_Descr::get_max_level(const tAttribute at) const {
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
		throw wexception("Unkown attribute value!");
	}
}

/*
=============================

class TrainingSite

=============================
*/

TrainingSite::TrainingSite(const TrainingSite_Descr & d) :
ProductionSite   (d),
m_capacity       (descr().get_max_number_of_soldiers()),
m_total_soldiers (0),
m_build_heros    (false),
m_pri_hp         (descr().get_train_hp     () ? 6 : 0),
m_pri_attack     (descr().get_train_attack () ? 6 : 0),
m_pri_defense    (descr().get_train_defense() ? 6 : 0),
m_pri_evade      (descr().get_train_evade  () ? 6 : 0),
m_pri_hp_mod     (0),
m_pri_attack_mod (0),
m_pri_defense_mod(0),
m_pri_evade_mod  (0),
m_success        (false)
{}


TrainingSite::~TrainingSite()
{
}


/**
 * Retrieve the training program that is currently running.
 * \return  the name of the current program
 */
std::string TrainingSite::get_statistics_string()
{
	State *state;
	state = get_current_program(); //may also be NULL if there is no current program

	if (state) {
		return m_prog_name;
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
	call_soldiers();
}

/**
 * Change the economy this site belongs to.
 * \par e  The new economy. Can be 0 (unconnected buildings have no economy).
 * \note the worker (but not the soldiers) is dealt with in the PlayerImmovable code.
 */
void TrainingSite::set_economy(Economy * e)
{
	ProductionSite::set_economy(e);

	for (uint i = 0; i < m_soldier_requests.size(); i++) {
		if (m_soldier_requests[i])
			m_soldier_requests[i]->set_economy(e);
	}
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

	if (m_soldier_requests.size()) {
		for (uint i = 0; i < m_soldier_requests.size(); i++) {
			delete m_soldier_requests[i];
			m_soldier_requests[i] = 0;
		}
		m_soldier_requests.resize(0);
	}

	if (m_soldiers.size()) {
		uint i;
		for (i = 0; i < m_soldiers.size(); i++) {
			Soldier *s = m_soldiers[i];
			m_soldiers[i] = 0;

			if (g->objects().object_still_available(s))
				s->set_location(0);
		}
		m_soldiers.resize(0);
	}

	ProductionSite::cleanup(g);
}


/*
===============
TrainingSite::remove_worker

Intercept remove_worker() calls to unassign our worker, if necessary.
===============

void TrainingSite::remove_worker(Worker* w)
{
	if (m_worker == w) {
		m_worker = 0;

		request_worker((Game*)get_owner()->get_game());
	}

	Building::remove_worker(w);
}
*/

/**
 * Request exactly one soldier
 */
void TrainingSite::request_soldier() {
	int soldierid = get_owner()->tribe().get_safe_worker_index("soldier");

	Request *req = new Request(this, soldierid, &TrainingSite::request_soldier_callback, this, Request::SOLDIER);
	Requeriments *r = new Requeriments();

	// set requirements to match this site
	int totalmax = 0;
	int totalmin = 0;
	if (descr().get_train_attack()) {
		totalmin += descr().get_min_level(atrAttack);
		totalmax += descr().get_max_level(atrAttack);
		r->set(atrAttack, descr().get_min_level(atrAttack), descr().get_max_level(atrAttack));
	}
	if (descr().get_train_defense()) {
		totalmin += descr().get_min_level(atrDefense);
		totalmax += descr().get_max_level(atrDefense);
		r->set(atrDefense, descr().get_min_level(atrDefense), descr().get_max_level(atrDefense));
	}
	if (descr().get_train_evade()) {
		totalmin += descr().get_min_level(atrEvade);
		totalmax += descr().get_max_level(atrEvade);
		r->set(atrEvade, descr().get_min_level(atrEvade), descr().get_max_level(atrEvade));
	}
	if (descr().get_train_hp()) {
		totalmin += descr().get_min_level(atrHP);
		totalmax += descr().get_max_level(atrHP);
		r->set(atrHP, descr().get_min_level(atrHP), descr().get_max_level(atrHP));
	}

	//  To make sure that fully trained soldiers are not requested.
	r->set(atrTotal, totalmin, totalmax - 1);

	req->set_requeriments(r);

	m_soldier_requests.push_back(req);
	m_total_soldiers++;
}


/**
 * When a soldier arrives, bring it into the fold
 */
// this is a static method
void TrainingSite::request_soldier_callback
(Game * g, Request * rq, int, Worker * w, void * data)
{
	assert(g);
	assert(rq);
	assert(w);
	assert(data);

	TrainingSite *tsite = (TrainingSite *) data;
	Soldier *s = static_cast < Soldier * >(w);

	assert(s);
	assert(s->get_location(g) == tsite);

	g->conquer_area
		(Player_Area<Area<FCoords> >
		 (tsite->owner().get_player_number(),
		  Area<FCoords>
		  (g->map().get_fcoords(tsite->get_position()),
		   tsite->descr().get_conquers())));

	uint i;
	for (i = 0; i < tsite->m_soldier_requests.size(); i++) {
		if (rq == tsite->m_soldier_requests[i]) {
			tsite->m_soldier_requests.erase(tsite->m_soldier_requests.begin() + i);
			break;
		}
	}

	tsite->m_soldiers.push_back(s);
	tsite->m_total_soldiers = tsite->m_soldiers.size() + tsite->m_soldier_requests.size();

	//  bind the worker into this house, hide him on the map
	s->start_task_idle(g, 0, -1);

	s->mark(false);
}

/**
 * If the site is not fully staffed, request as many soldiers as can be accomodated
 */
void TrainingSite::call_soldiers()
{while (m_capacity > m_total_soldiers) request_soldier();}

/**
 * Drop a given soldier.
 *
 * 'Dropping' means releasing the soldier from the site. The soldier then becomes available
 * to the economy.
 */
void TrainingSite::drop_soldier(uint serial)
{
	Game * const g = dynamic_cast<Game *>(&owner().egbase());

	assert(g);

	if (m_soldiers.size()) {
		int i = 0;
		Soldier *s = m_soldiers[i];
		while ((s->get_serial() != serial) && (i < (int) m_soldiers.size())) {
			i++;
			s = m_soldiers.at(i);
		}
		if ((i < (int) m_soldiers.size()) && (s->get_serial() == serial)) {
			drop_soldier(g, i);
		}
	} else
		molog("TrainingSite::drop_soldier(uint serial): trying to drop non-existant serial number %i !!",
		      serial);
}

/**
 * Drop a given soldier.
 * \internal
 * 'Dropping' means releasing the soldier from the site. The soldier then becomes available
 * to the economy.
 * \note This function should \b NEVER be called directly, use \ref drop_soldier(uint serial) instead.
 */
void TrainingSite::drop_soldier(Game * g, uint nr)
{
	Soldier *s;

	assert(g);
	assert(nr < m_soldiers.size());

	s = m_soldiers[nr];
	s->set_location(0);

	//remove the soldier-to-be-dropped from m_soldiers (it is still alive in s) by overwriting any reference
	//*must not* erase(), we still need the soldier
	for (uint i = nr; i < m_soldiers.size() - 1; i++)
		m_soldiers[i] = m_soldiers[i + 1];
	m_soldiers.pop_back();

	// Call more soldiers if are enought space
	call_soldiers();

	// Walk the soldier home safely
	s->reset_tasks(g);
	s->set_location(this);
	s->mark(false);
	s->start_task_leavebuilding(g, true);
}

/**
 * Drop all the soldiers that can not be upgraded further at this building.
 */
void TrainingSite::drop_unupgradable_soldiers(Game * g)
{
	uint count_upgrades = 0;

	assert(g);

	if (descr().get_train_hp())
		count_upgrades++;
	if (descr().get_train_attack())
		count_upgrades++;
	if (descr().get_train_defense())
		count_upgrades++;
	if (descr().get_train_evade())
		count_upgrades++;

	for (uint i = 0; i < m_soldiers.size(); i++) {
		uint count;
		count = 0;
		if
			((m_soldiers[i]->get_level(atrHP)
			  <
			  static_cast<uint>(descr().get_min_level(atrHP))
			  or
			  m_soldiers[i]->get_level(atrHP)
			  >
			  static_cast<uint>(descr().get_max_level(atrHP)))
			 and
			 (descr().get_train_hp()))
			++count;

		if
			((m_soldiers[i]->get_level(atrAttack)
			  <
			  static_cast<uint>(descr().get_min_level(atrAttack))
			  or
			  m_soldiers[i]->get_level(atrAttack)
			  >
			  static_cast<uint>(descr().get_max_level(atrAttack)))
			 and
			 (descr().get_train_attack()))
			++count;

		if
			((m_soldiers[i]->get_level(atrDefense)
			  <
			  static_cast<uint>(descr().get_min_level(atrDefense))
			  or
			  m_soldiers[i]->get_level(atrDefense)
			  >
			  static_cast<uint>(descr().get_max_level(atrDefense)))
		    and
			 (descr().get_train_defense()))
			++count;

		if
			((m_soldiers[i]->get_level(atrEvade)
			  <
			  static_cast<uint>(descr().get_min_level(atrEvade))
			  or
			  m_soldiers[i]->get_level(atrEvade)
			  >
			  static_cast<uint>(descr().get_max_level(atrEvade)))
			 and
			 descr().get_train_evade())
			++count;

		if (count >= count_upgrades)
			drop_soldier(g, i);
	}
}

/**
 * Advance the program state (if a program is running) or call the training program
 * The real training is done by a normal \ref ProductionProgram that gets
 * executed by \ref ProductionSite::program_act() like all other production
 * programs.
*/
void TrainingSite::act(Game * g, uint data)
{
	assert(g);

	Building::act(g, data);

	if (m_program_timer && (int) (g->get_gametime() - m_program_time) >= 0) {
		m_program_timer = false;

		if (!m_program.size()) {
			find_and_start_next_program(g);
			return;
		}

		State *state = get_current_program();

		assert(state);

		if (state->ip >= state->program->get_size()) {
			program_end(g, true);
			return;
		}

		if (m_anim != descr().get_animation("idle")) {
			// Restart idle animation (which is the default animation)
			start_animation(g, descr().get_animation("idle"));
		}

		program_act(g); //  this will do the actual training
	}
}

/**
 * Find and start the next training program.
 */
void TrainingSite::find_and_start_next_program(Game * g)
{
	tAttribute attrib;

	assert(g);

	if (!m_list_upgrades.size())
		calc_list_upgrades(g);

	if (m_list_upgrades.size()) {
		int i = m_list_upgrades.size() - 1;
		int j;
		int min_level = 0;
		int max_level = 0;
		int level = 0;
		int MAX_level = 0;
		bool done = false;
		std::vector < std::string > str;

		split_string(m_list_upgrades[i], str, "_");

		molog(m_list_upgrades[i].c_str());
		assert(str.size() == 2); //  upgrade what

		if (str[1] == "hp")
			attrib = atrHP;
		else if (str[1] == "attack")
			attrib = atrAttack;
		else if (str[1] == "defense")
			attrib = atrDefense;
		else if (str[1] == "evade")
			attrib = atrEvade;
		else
			throw wexception("Unkown attribute to upgrade %s.", str[1].c_str());

		if (m_soldiers.size()) {
			max_level = descr().get_max_level(attrib);
			MAX_level = max_level;

			if (m_build_heros) {

				while ((min_level < max_level) && (!done)) {

					for (j = 0; j < (int) m_soldiers.size(); j++)
						if ((int) m_soldiers[j]->get_level(attrib) == max_level)
							done = true;

					if (!done)
						max_level--;
				}
			} else {

				while ((min_level < max_level) && (!done)) {

					for (j = 0; j < (int) m_soldiers.size(); j++)
						if ((int) m_soldiers[j]->get_level(attrib) == min_level)
							done = true;

					if (!done)
						min_level++;
				}
			}

			if (m_build_heros)
				level = max_level;
			else
				level = min_level;

			if (level > MAX_level)
				level = 5000;
		}

		i = m_list_upgrades.size() - 1;

		if (level < 10) {
			char buf[200];

			switch (attrib) {
			case atrHP:
				level += m_pri_hp_mod;
				break;
			case atrAttack:
				level += m_pri_attack_mod;
				break;
			case atrDefense:
				level += m_pri_defense_mod;
				break;
			case atrEvade:
				level += m_pri_evade_mod;
				break;
			case atrTotal:
				break;
			}

			if ((level >= 0) && (level <= MAX_level)) {
				sprintf(buf, "%s%d", (m_list_upgrades[i]).c_str(), level);
				m_list_upgrades[i] = buf;
				program_start(g, m_list_upgrades[i]);
			} else {
				m_list_upgrades.pop_back();
				program_start(g, "Sleep");
				return;
			}

			switch (attrib) {
			case atrHP:
				m_pri_hp_mod = 0;
				break;
			case atrAttack:
				m_pri_attack_mod = 0;
				break;
			case atrDefense:
				m_pri_defense_mod = 0;
				break;
			case atrEvade:
				m_pri_evade_mod = 0;
				break;
			case atrTotal:
				break;
			}
		} else {

			if (m_build_heros)
				modif_priority(attrib, -1);
			else
				modif_priority(attrib, 1);
			m_list_upgrades.pop_back();
			program_start(g, "Sleep");
			return;
		}

		m_list_upgrades.pop_back();
		return;
	} else
		throw wexception("Critical Error: TrainingSite that hasn't  a list of upgrades!!");
}

/**
 * Change the priorities for training
 */
void TrainingSite::modif_priority(tAttribute atr, int value)
{
	switch (atr) {
	case atrHP:
		m_pri_hp_mod += value;
		break;
	case atrAttack:
		m_pri_attack_mod += value;
		break;
	case atrDefense:
		m_pri_defense_mod += value;
		break;
	case atrEvade:
		m_pri_evade_mod += value;
		break;
	default:
		throw wexception("Unknown attribute at %s:%d", __FILE__, __LINE__);
	}
}

/**
 * Gets the priority of given attribute
 */
uint TrainingSite::get_pri(tAttribute atr)
{
	switch (atr) {
	case atrHP:
		return m_pri_hp;
		break;
	case atrAttack:
		return m_pri_attack;
		break;
	case atrDefense:
		return m_pri_defense;
		break;
	case atrEvade:
		return m_pri_evade;
		break;
	default:
		throw wexception("Invalid soldier attribute at %s:%d", __FILE__, __LINE__);
	}
}

/**
 * Adds a value of given priority. Also mantain the total value of 12 priorities
 */
void TrainingSite::add_pri(tAttribute atr)
{
	switch (atr) {
	case atrHP:
		if (m_pri_hp < 12)
			m_pri_hp++;
		break;
	case atrAttack:
		if (m_pri_attack < 12)
			m_pri_attack++;
		break;
	case atrDefense:
		if (m_pri_defense < 12)
			m_pri_defense++;
		break;
	case atrEvade:
		if (m_pri_evade < 12)
			m_pri_evade++;
		break;
	default:
		throw wexception("Invalid soldier attribute at %s:%d", __FILE__, __LINE__);
	}
}


/**
 * Lower the given priority and make sure that the total of all priority points is >=2
 * \par atr  the priority to lower
 */
void TrainingSite::sub_pri(tAttribute atr)
{
	switch (atr) {
	case atrHP:
		if (m_pri_hp > 0)
			m_pri_hp--;
		break;
	case atrAttack:
		if (m_pri_attack > 0)
			m_pri_attack--;
		break;
	case atrDefense:
		if (m_pri_defense > 0)
			m_pri_defense--;
		break;
	case atrEvade:
		if (m_pri_evade > 0)
			m_pri_evade--;
		break;
	default:
		throw wexception("Invalid soldier attribute at %s:%d", __FILE__, __LINE__);
	}
	if ((m_pri_hp + m_pri_attack + m_pri_defense + m_pri_evade == 1)) {
		// At least we need to have TWO priority points on attributes
		add_pri(atr);
	}
}

/**
 * Change the soldier capacity at the trainingsite.
 * \post Minimum and maximum capacity will be observed.
 * \param how  number to add/subtract from the current capacity
 * \note Unlike the influence-defining military buildings, a training site can actually be empty of soldiers.
 *
 */
void TrainingSite::change_soldier_capacity(int how)
{
	int temp_capacity;
	temp_capacity = m_capacity + how;

	if (temp_capacity < 0)
		m_capacity = 0;
	else if (temp_capacity > descr().get_max_number_of_soldiers())
		m_capacity = descr().get_max_number_of_soldiers();
	else
		m_capacity = temp_capacity;

	if (m_capacity > m_total_soldiers) call_soldiers();

	while (m_capacity < m_total_soldiers) {
		if (m_soldier_requests.size()) {
			delete m_soldier_requests[0];
			m_soldier_requests[0] = m_soldier_requests[m_soldier_requests.size() - 1];
			m_soldier_requests.pop_back();
			break;
		} else if (m_soldiers.size()) {
			drop_soldier(m_soldiers[0]->get_serial());
			break;
		} else
			throw
			    wexception
			    ("TrainingSite::change_soldier_capacity(): m_capacity<m_total_soldiers although m_total_soldiers==0");
	}

	m_total_soldiers = m_soldiers.size() + m_soldier_requests.size();
}


/**
 * Get a list of possible upgrades ordered by priority. This list is used in
 * \par g  the curent game object
 * \sa find_and_start_next_program()
 * \return n/a, the output is in \ref m_list_upgrades
 */
void TrainingSite::calc_list_upgrades(Game *) {
	int higher;
	int r_hp = m_pri_hp;
	int r_attack = m_pri_attack;
	int r_defense = m_pri_defense;
	int r_evade = m_pri_evade;
	std::vector < std::string > list;

	if (!descr().get_train_hp())
		r_hp = 0;
	if (!descr().get_train_attack())
		r_attack = 0;
	if (!descr().get_train_defense())
		r_defense = 0;
	if (!descr().get_train_evade())
		r_evade = 0;

	while ((r_hp > 0) || (r_attack > 0) || (r_defense > 0) || (r_evade > 0)) {
		// Calculate the higher
		if (r_hp >= r_attack) {
			if (r_hp >= r_defense) {
				if (r_hp >= r_evade)
					higher = atrHP;
				else
					higher = atrEvade;
			} else {
				if (r_defense >= r_evade)
					higher = atrDefense;
				else
					higher = atrEvade;
			}
		} else {
			if (r_attack >= r_defense) {
				if (r_attack >= r_evade)
					higher = atrAttack;
				else
					higher = atrEvade;
			} else {
				if (r_defense >= r_evade)
					higher = atrDefense;
				else
					higher = atrEvade;
			}
		}
		switch (higher) {
		case atrHP:
			r_hp--;
			list.push_back("upgrade_hp_");
			molog(" Added upgrade_hp_#\n");
			break;
		case atrAttack:
			r_attack--;
			list.push_back("upgrade_attack_");
			molog(" Added upgrade_attack_#\n");
			break;
		case atrDefense:
			r_defense--;
			list.push_back("upgrade_defense_");
			molog(" Added upgrade_defense_#\n");
			break;
		case atrEvade:
			r_evade--;
			list.push_back("upgrade_evade_");
			molog(" Added upgrade_evade_#\n");
			break;
		}
	};

	// Invert priorities
	for (int i = list.size() - 1; i >= 0; i--) {
		m_list_upgrades.push_back(list[i]);
	}
}

/**
 * Start a training program
 * \param game  the current game object
 * \param name  the program to start
 */
void TrainingSite::program_start(Game * g, std::string program_name)
{
	assert(g);

	set_post_timer(6000);
	m_prog_name = program_name;
	ProductionSite::program_start(g, program_name);
}

/**
 * Clean up after the end of a training program and find the next program that should
 * be run at this training site
 * \param game     the current game object
 * \param success  whether the program was finished successfully
 */
void TrainingSite::program_end(Game * g, bool success)
{
	bool relaunch = false;
	std::string string;

	assert(g);

	assert(g);

	m_success = success;

	if (!m_success)
		string = m_prog_name;

	if (m_prog_name == "Sleep")
		relaunch = true;

	m_prog_name = "Not Working";
	set_post_timer(6000);
	drop_unupgradable_soldiers(g);
	ProductionSite::program_end(g, success);

	if (relaunch)
		find_and_start_next_program(g);
}
