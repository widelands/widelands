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
#include "trainingsite.h"
#include "tribe.h"
#include "player.h"
#include "production_program.h"
#include "profile.h"
#include "soldier.h"
#include "transport.h"
#include "util.h"
#include "worker.h"

/*
=============================

class TrainingSite_Descr

=============================
*/
TrainingSite_Descr::TrainingSite_Descr(Tribe_Descr* tribe, const char* name)
	: ProductionSite_Descr(tribe, name)
{
	m_num_soldiers = 0;
	m_train_hp = false;
	m_train_attack = false;
	m_train_defense = false;
	m_train_evade = false;
	m_min_hp = 0;
	m_min_attack = 0;
	m_min_defense = 0;
	m_min_evade = 0;
	m_max_hp = 0;
	m_max_attack = 0;
	m_max_defense = 0;
	m_max_evade = 0;
}

TrainingSite_Descr::~TrainingSite_Descr()
{
}

/*
===============
TrainingSite_Descr::parse

Parse the additional information necessary for miltary buildings
===============
*/
void TrainingSite_Descr::parse(const char* directory, Profile* prof,
	const EncodeData* encdata)
{
	Section* sglobal = prof->get_section("global");
	std::string trainable;
	std::vector<std::string> str_list;

	ProductionSite_Descr::parse(directory,prof,encdata);

	// Defaults to false
	m_stopable=true;

	m_num_soldiers=sglobal->get_safe_int("max_soldiers");

	trainable = sglobal->get_safe_string("train");
	split_string (trainable, &str_list, ",");

	while (str_list.size()) {
			if (str_list[0] == "hp")		m_train_hp = true;
		else if (str_list[0] == "attack")	m_train_attack = true;
		else if (str_list[0] == "defense")	m_train_defense = true;
		else if (str_list[0] == "evade")	m_train_evade = true;
		else
			throw wexception("Attribute %s isn't known as a valid attribute", str_list[0].c_str());
		str_list.erase(str_list.begin());
	}

	// Read the range of levels that can update this building
	if (m_train_hp) {
		Section* sect = prof->get_section("hp");
		m_min_hp = sect->get_safe_int("min_level");
		m_max_hp = sect->get_safe_int("max_level");
	}
	if (m_train_attack) {
		Section* sect = prof->get_section("attack");
		m_min_attack = sect->get_safe_int("min_level");
		m_max_attack = sect->get_safe_int("max_level");
	}
	if (m_train_defense) {
		Section* sect = prof->get_section("defense");
		m_min_defense = sect->get_safe_int("min_level");
		m_max_defense = sect->get_safe_int("max_level");
	}
	if (m_train_evade) {
		Section* sect = prof->get_section("evade");
		m_min_evade = sect->get_safe_int("min_level");
		m_max_evade = sect->get_safe_int("max_level");
	}
}

/*
===============
TrainingSite_Descr::create_object

Create a new building of this type
===============
*/
Building* TrainingSite_Descr::create_object()
{
	return new TrainingSite(this);
}

/*
================
TrainingSite_Descr::get_min_level

Returns the minium level that this building can upgrade of specified attribute
================
*/
int TrainingSite_Descr::get_min_level (tAttribute at) {
	switch (at) {
		case atrHP:			return m_min_hp;
		case atrAttack:	return m_min_attack;
		case atrDefense:	return m_min_defense;
		case atrEvade:		return m_min_evade;
		default:
			throw wexception ("Unkown attribute value!");
	}
}

/*
================
TrainingSite_Descr::get_max_level

Returns the maxium level that this building can upgrade of specified attribute
================
*/
int TrainingSite_Descr::get_max_level (tAttribute at) {
	switch (at) {
		case atrHP:			return m_max_hp;
		case atrAttack:	return m_max_attack;
		case atrDefense:	return m_max_defense;
		case atrEvade:		return m_max_evade;
		default:
			throw wexception ("Unkown attribute value!");
	}
}

/*
=============================

class TrainingSite

=============================
*/

/*
===============
TrainingSite::TrainingSite
===============
*/
TrainingSite::TrainingSite(TrainingSite_Descr* descr)
	: ProductionSite(descr)
{
	m_build_heros = false;
	m_success = false;
	m_pri_hp = get_descr()->get_train_hp() ? 6 : 0;
	m_pri_attack = get_descr()->get_train_attack() ? 6 : 0;
	m_pri_defense = get_descr()->get_train_defense() ? 6 : 0;
	m_pri_evade = get_descr()->get_train_evade() ? 6 : 0;
	m_total_soldiers = 0;
	m_capacity = get_descr()->get_max_number_of_soldiers();
	m_pri_hp_mod = 0;
	m_pri_attack_mod = 0;
	m_pri_defense_mod = 0;
	m_pri_evade_mod = 0;
}


/*
===============
TrainingSite::~TrainingSite
===============
*/
TrainingSite::~TrainingSite()
{
}


/*
===============
TrainingSite::get_statistics_string

Display progran running.
===============
*/
std::string TrainingSite::get_statistics_string()
{
	State *state;
	state = get_current_program();
	if (state) {
		return m_prog_name;
	} else
		if (m_success)
			return "Resting";
		else
			return "Not Working";
}


/*
===============
TrainingSite::init

Initialize the Training site (request worker).
===============
*/
void TrainingSite::init(Editor_Game_Base* g)
{
	ProductionSite::init(g);

	if (g->is_game()) {
		// Request soldiers
		call_soldiers((Game *)g);
	}
}

/*
===============
TrainingSite::set_economy

Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void TrainingSite::set_economy(Economy* e)
{
	// TODO: SoldiersQueue migration <--- This it does at call_soldier and drop_soldier
	ProductionSite::set_economy(e);

	for (uint i = 0; i < m_soldier_requests.size(); i++) {
		if (m_soldier_requests[i])
			m_soldier_requests[i]->set_economy(e);
	}
}

/*
===============
TrainingSite::cleanup

Cleanup after a Training site is removed
===============
*/
void TrainingSite::cleanup(Editor_Game_Base* g)
{
	// Release soldier
	if (m_soldier_requests.size()) {
		for (uint i = 0; i < m_soldier_requests.size(); i++) {
			delete m_soldier_requests[i];
			m_soldier_requests[i] = 0;
		}
		m_soldier_requests.resize(0);
	}

	if (m_soldiers.size()) {
		uint i;
		for(i=0; i<m_soldiers.size(); i++) {
			Soldier* s = m_soldiers[i];
		 	m_soldiers[i] = 0;
         if(g->get_objects()->object_still_available(s))
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

/*
===============
TrainingSite::request_soldier

Issue the soldier request
===============
*/
void TrainingSite::request_soldier(Game* g)
{
	int soldierid = get_owner()->get_tribe()->get_safe_worker_index("soldier");

	Request* req = new Request(this, soldierid, &TrainingSite::request_soldier_callback, this, Request::SOLDIER);
	Requeriments* r = new Requeriments();

   // setting requirements to match this site
   int totalmax = 0;
   int totalmin = 0;
   if( get_descr()->get_train_attack()) {
      totalmin += get_descr()->get_min_level(atrAttack);
      totalmax += get_descr()->get_max_level(atrAttack);
      r->set(atrAttack, get_descr()->get_min_level(atrAttack), get_descr()->get_max_level(atrAttack));
   }
   if( get_descr()->get_train_defense()) {
      totalmin += get_descr()->get_min_level(atrDefense);
      totalmax += get_descr()->get_max_level(atrDefense);
      r->set(atrDefense, get_descr()->get_min_level(atrDefense), get_descr()->get_max_level(atrDefense));
   }
   if( get_descr()->get_train_evade()) {
      totalmin += get_descr()->get_min_level(atrEvade);
      totalmax += get_descr()->get_max_level(atrEvade);
      r->set(atrEvade, get_descr()->get_min_level(atrEvade), get_descr()->get_max_level(atrEvade));
   } 
   if( get_descr()->get_train_hp()) {
      totalmin += get_descr()->get_min_level(atrHP);
      totalmax += get_descr()->get_max_level(atrHP);
      r->set(atrHP, get_descr()->get_min_level(atrHP), get_descr()->get_max_level(atrHP));
   }
   r->set(atrTotal, totalmin, totalmax - 1); // To make sure that fully trained soldiers are not requested

   req->set_requeriments (r);

	m_soldier_requests.push_back(req);
	m_total_soldiers ++;
}


/*
===============
TrainingSite::request_soldier_callback [static]

Called when our soldier arrives.
===============
*/
void TrainingSite::request_soldier_callback(Game* g, Request* rq, int ware,
	Worker* w, void* data)
{
	TrainingSite* tsite = (TrainingSite*)data;
	Soldier* s=static_cast<Soldier*>(w);

	assert(s);
	assert(s->get_location(g) == tsite);

	g->conquer_area(tsite->get_owner()->get_player_number(),
		tsite->get_position(), tsite->get_descr());

	uint i=0;
 	for(i=0; i<tsite->m_soldier_requests.size(); i++)
		if(rq==tsite->m_soldier_requests[i]) break;

	tsite->m_soldier_requests.erase(tsite->m_soldier_requests.begin() + i);

	tsite->m_soldiers.push_back(s);
	tsite->m_total_soldiers = tsite->m_soldiers.size() + tsite->m_soldier_requests.size();
	s->start_task_idle(g, 0, -1); // bind the worker into this house, hide him on the map
   s->mark(false);
}

/*
===========
TrainingSite::call_soldiers

Send the request for more soldiers if there are not full
===========
 */
void TrainingSite::call_soldiers(Game *g) {
	if (g->is_game()) {
		while(m_capacity > m_total_soldiers) {
			request_soldier(g);
		}
	}
}

/*
===========
TrainingSite::drop_soldier.

Get out specied soldier from house. (Game queue usage)
===========
 */
void TrainingSite::drop_soldier (uint serial) {
	Game* g = (Game *)get_owner()->get_game();

	if (g->is_game() && m_soldiers.size()) {
		int i = 0;
		Soldier* s = m_soldiers[i];
		while ((s->get_serial() != serial) && (i < (int)m_soldiers.size())) {
			i++;
			s = m_soldiers[i];
		}
		if ((i < (int)m_soldiers.size()) && (s->get_serial() == serial)) {
			drop_soldier(g, i);
		}
	}
}

/*
==========
TrainingSite::drop_soldier(Game, uint)

Drops the soldier at given row in table.
Note: This function shouldn't be called NEVER directly, use drop_soldier(serial) instead.
==========
*/
void TrainingSite::drop_soldier (Game *g, uint nr) {

	if (g->is_game()) {
		Soldier *s;
		// Is out of bounds ?
		if (nr >= m_soldiers.size())
			return;

		s = m_soldiers[nr];
		s->set_location(0);

		m_total_soldiers--;

		for (uint i = nr; i < m_soldiers.size(); i++)
			m_soldiers[i] = m_soldiers[i+1];

		m_soldiers.pop_back();

		// Call more soldiers if are enought space
		call_soldiers (g);

		// Walk the soldier home safely
		s->reset_tasks(g);
		s->set_location(this);
      s->mark(false);
		s->start_task_leavebuilding(g,true);
	}
}

/*
===========
TrainingSite::drop_unupgradable_soldiers.

This is for drop all the soldiers that can not be upgraded at this building
===========
 */
void TrainingSite::drop_unupgradable_soldiers(Game *g) {
	uint count_upgrades = 0;
	if (get_descr()->get_train_hp())		count_upgrades++;
	if (get_descr()->get_train_attack())	count_upgrades++;
	if (get_descr()->get_train_defense())	count_upgrades++;
	if (get_descr()->get_train_evade())		count_upgrades++;

	if (g->is_game()) {
		for (uint i = 0; i < m_soldiers.size(); i++) {
			uint count;
			count = 0;
			if (((m_soldiers[i]->get_level(atrHP) < (uint)get_descr()->get_min_level(atrHP)) ||
				(m_soldiers[i]->get_level(atrHP) >= (uint)get_descr()->get_max_level(atrHP)))
				&& (get_descr()->get_train_hp()))
				count ++;

			if (((m_soldiers[i]->get_level(atrAttack) < (uint)get_descr()->get_min_level(atrAttack)) ||
				(m_soldiers[i]->get_level(atrAttack) >= (uint)get_descr()->get_max_level(atrAttack)))
				&& (get_descr()->get_train_attack()))
				count ++;

			if (((m_soldiers[i]->get_level(atrDefense) < (uint)get_descr()->get_min_level(atrDefense)) ||
				(m_soldiers[i]->get_level(atrDefense) >= (uint) get_descr()->get_max_level(atrDefense)))
				&& (get_descr()->get_train_defense()))
				count ++;

			if (((m_soldiers[i]->get_level(atrEvade) < (uint) get_descr()->get_min_level(atrEvade)) ||
				(m_soldiers[i]->get_level(atrEvade) >= (uint) get_descr()->get_max_level(atrEvade)))
				&& (get_descr()->get_train_evade()))
				count ++;

			if (count >= count_upgrades)
				drop_soldier(g, i);
		}
	}
}
/*
===============
TrainingSite::act

Advance the program state if applicable, calling the training program to upgrade soldiers
===============
*/
void TrainingSite::act(Game* g, uint data)
{
	Building::act(g, data);
	if (m_program_timer && (int)(g->get_gametime() - m_program_time) >= 0) {
		m_program_timer = false;

		if (!m_program.size())
		{
			find_next_program(g);
			return;
		}

		State* state = get_current_program();

		assert(state);

		if (state->ip >= state->program->get_size()) {
			program_end(g, true);
			return;
		}

		if (m_anim != get_descr()->get_animation ("idle")) {
			// Restart idle animation, which is the default
			start_animation(g, get_descr()->get_animation("idle"));
		}

		program_act(g);
	}
}

/*
==========
TrainingSite::find_next_program.

Usefull to find the next program that can be tried. It also start to run it.
==========
 */
void TrainingSite::find_next_program (Game *g) {
	tAttribute attrib;
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
		std::vector<std::string> str;

		split_string (m_list_upgrades[i], &str, "_");

      molog (m_list_upgrades[i].c_str());
		assert (str.size() == 2); // upgrade what

		if (str[1] == "hp") attrib = atrHP;
		else if (str[1] == "attack") attrib = atrAttack;
		else if (str[1] == "defense") attrib = atrDefense;
		else if (str[1] == "evade") attrib = atrEvade;
		else
			throw wexception ("Unkown attribute to upgrade %s.", str[1].c_str());

		if (m_soldiers.size()) {
			max_level = get_descr()->get_max_level(attrib);
			MAX_level = max_level;

			if (m_build_heros) {

				while ((min_level < max_level) && (!done)) {

					for (j = 0; j < (int) m_soldiers.size(); j++)
						if ((int)m_soldiers[j]->get_level(attrib) == max_level)
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

			if (level >= MAX_level)
				level = 5000;
		}

		i = m_list_upgrades.size() - 1;

		if (level < 10) {
			char buf [200];

			switch (attrib) {
				case atrHP:			level += m_pri_hp_mod; break;
				case atrAttack:		level += m_pri_attack_mod; break;
				case atrDefense:	level += m_pri_defense_mod; break;
				case atrEvade:		level += m_pri_evade_mod; break;
            case atrTotal: break;
			}

			if ((level >= 0) && (level < MAX_level)) {
				sprintf (buf, "%s%d", (m_list_upgrades[i]).c_str(), level);
				m_list_upgrades[i] = buf;
				program_start (g, m_list_upgrades[i]);
			} else {
				m_list_upgrades.pop_back();
				program_start (g, "Sleep");
            return; 
			}

			switch (attrib) {
				case atrHP:			m_pri_hp_mod = 0; break;
				case atrAttack:	m_pri_attack_mod = 0; break;
				case atrDefense:	m_pri_defense_mod = 0; break;
				case atrEvade:		m_pri_evade_mod = 0; break;
            case atrTotal: break;
			}
		} else {

			if (m_build_heros)
				modif_priority (attrib, -1);
			else
				modif_priority (attrib, 1);
			m_list_upgrades.pop_back();
			program_start (g, "Sleep");
			return;
		}

		m_list_upgrades.pop_back();
		return;
	} else
		throw wexception ("Critical Error: TrainingSite that hasn't  a list of upgrades!!");
}

/*
===========
TrainingSite::modify_priority

Performs a modificatoin at train priorities.
===========
 */
void TrainingSite::modif_priority (enum tAttribute atr, int value) {
	switch (atr) {
		case atrHP:			m_pri_hp_mod		+= value; break;
		case atrAttack:		m_pri_attack_mod	+= value; break;
		case atrDefense:	m_pri_defense_mod	+= value; break;
		case atrEvade:		m_pri_evade_mod		+= value; break;
		default:
			throw wexception ("Unknown attribute at %s:%d", __FILE__, __LINE__);
	}
}

/**
===========
TrainingSite::get_pri(tAttribute)

Gets the priority of given attribute
===========
*/
uint TrainingSite::get_pri (tAttribute atr) {
	switch (atr) {
		case atrHP:		 return m_pri_hp;	break;
		case atrAttack:	 return m_pri_attack;	break;
		case atrDefense:	return m_pri_defense;	break;
		case atrEvade:	  return m_pri_evade;	 break;
		default:
			throw wexception ("Invalid soldier attribute at %s:%d", __FILE__, __LINE__);
	}
}

/**
===========
TrainingSite::add_pri(tAttribute)

Adds a value of given priority. Also mantain the total value of 12 priorities
===========
*/
void TrainingSite::add_pri (enum tAttribute atr) {
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
			throw wexception ("Invalid soldier attribute at %s:%d", __FILE__, __LINE__);
	}
}


/*
===========
TrainingSite::sub_pri

Subs a value of given priority. Also mantain the maxium value of 12 priorities and a minium
of 2 priority points total.
===========
 */
void TrainingSite::sub_pri (tAttribute atr) {
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
			throw wexception ("Invalid soldier attribute at %s:%d", __FILE__, __LINE__);
	}
	if ((m_pri_hp + m_pri_attack + m_pri_defense + m_pri_evade == 1)) {
		// At least we need to have TWO priority points on atributes
		add_pri (atr);
	}
}

/*
==========
TrainingSite::up_capacity

Performs an increase of the soldiers capacity at house, if it isn't reached the maxium capacity
==========
 */
void TrainingSite::change_soldier_capacity (int how) {
	if (how) {
    	if (how > 0) {
        	m_capacity += how;

            if (m_capacity > (uint) get_descr()->get_max_number_of_soldiers())
				m_capacity = (uint) get_descr()->get_max_number_of_soldiers();
			call_soldiers((Game*)get_owner()->get_game());
		} else {
			how = -how;
        	if (how >= (int) m_capacity)
            	m_capacity = 1;
			else
            	m_capacity -= how;
			if (m_capacity < m_total_soldiers) {
				if (m_soldier_requests.size()) {
					m_soldier_requests[0]->cancel_transfer(0);
					m_soldier_requests.erase(m_soldier_requests.begin());
					m_total_soldiers --;
				} else
					if (m_soldiers.size()) {
						Soldier *s = m_soldiers[0];
						drop_soldier (s->get_serial());
				}
			}
		}
	}
}


/*
===========
TrainingSite::calc_list_upgrades.

Private function to get a list of strings order by priority. This list is used at
find_next_program
===========
*/
void TrainingSite::calc_list_upgrades(Game *g) {
	int higher;
	int r_hp = m_pri_hp ;
	int r_attack = m_pri_attack;
	int r_defense = m_pri_defense;
	int r_evade = m_pri_evade;
	std::vector<std::string> list;

	if (!get_descr()->get_train_hp())		r_hp = 0;
	if (!get_descr()->get_train_attack())	r_attack = 0;
	if (!get_descr()->get_train_defense())	r_defense = 0;
	if (!get_descr()->get_train_evade())	r_evade = 0;

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
				r_hp --;
				list.push_back("upgrade_hp_");
				molog (" Added upgrade_hp_#\n");
				break;
			case atrAttack:
				r_attack--;
				list.push_back("upgrade_attack_");
				molog (" Added upgrade_attack_#\n");
				break;
			case atrDefense:
				r_defense--;
				list.push_back("upgrade_defense_");
				molog (" Added upgrade_defense_#\n");
				break;
			case atrEvade:
				r_evade--;
				list.push_back("upgrade_evade_");
				molog (" Added upgrade_evade_#\n");
				break;
		}
	};

	// Invert priorities
	for (int i = list.size() - 1; i >= 0; i--) {
		m_list_upgrades.push_back (list[i]);
	}
}

/*
==========
TrainingSite::program_start

Hacks the start program of production program (training program).
==========
 */
void TrainingSite::program_start (Game *g, std::string name) {
	set_post_timer (6000);
	m_prog_name = name;
	ProductionSite::program_start (g, name);
}

/*
==========
TrainingSite::program_end

Handle the end of a training program, this is usefull to find what will be the next program
that should be loaded at this training site, and to shows what is doing the training site.
==========
 */

void TrainingSite::program_end (Game *g, bool success) {
	bool relaunch = false;
	std::string	string;

	m_success = success;

	if (!m_success)
		string = m_prog_name;

	if (m_prog_name == "Sleep")
		relaunch = true;

	m_prog_name = "Not Working";
	set_post_timer (6000);
	drop_unupgradable_soldiers(g);
	ProductionSite::program_end (g, success);

	if (relaunch)
		find_next_program(g);
}

