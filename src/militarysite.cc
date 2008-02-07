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

MilitarySite_Descr::~MilitarySite_Descr()
{
}

/**
===============
MilitarySite_Descr::parse

Parse the additional information necessary for miltary buildings
===============
*/
void MilitarySite_Descr::parse(const char* directory, Profile* prof,
	const EncodeData* encdata)
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
m_didconquer  (false),
m_capacity    (ms_descr.get_max_number_of_soldiers()),
m_in_battle   (false)
{}


/**
===============
MilitarySite::~MilitarySite
===============
*/
MilitarySite::~MilitarySite()
{
	if (m_soldier_requests.size())
      log ("[MilitarySite] Ouch! Still have soldier requests!\n");

	if (m_soldiers.size())
      log ("[MilitarySite] Ouch! Still have soldiers!\n");
}


/**
===============
Display number of soldiers.
===============
*/
std::string MilitarySite::get_statistics_string()
{
	char buffer[255];
	if (m_soldier_requests.size())
		snprintf
			(buffer, sizeof(buffer),
			 ngettext
			 ("%u soldier (+%u)", "%u soldiers (+%u)",
			  m_soldiers.size()), m_soldiers.size(), m_soldier_requests.size());
	else
		snprintf
			(buffer, sizeof(buffer),
			 ngettext
			 ("%u soldier", "%u soldiers", m_soldiers.size()), m_soldiers.size());

	return buffer;
}


void MilitarySite::fill(Game & game) {
	ProductionSite::fill(game);
	assert(m_soldiers        .empty());
	assert(m_soldier_requests.empty());
	Tribe_Descr const & tribe = owner().tribe();
	std::vector<std::string> const & workers = descr().workers();
	std::vector<std::string>::const_iterator const workers_end =
		workers.end();
	for (uint32_t i = descr().get_max_number_of_soldiers(); i; --i) {
		Soldier & soldier =
			dynamic_cast<Soldier &>
			(tribe.get_worker_descr(tribe.worker_index("soldier"))->create
			 (game, owner(), *get_base_flag(), get_position()));
		soldier.start_task_idle(&game, 0, -1);
		m_soldiers.push_back(&soldier);
	}
	conquer_area(game);
}

/**
===============
MilitarySite::init

Initialize the military site.
===============
*/
void MilitarySite::init(Editor_Game_Base* g)
{
   ProductionSite::init(g);

	if (upcast(Game, game, g)) {
		if (m_soldiers.size()) {
			std::vector<Soldier *>::const_iterator const soldiers_end =
				m_soldiers.end();
			for
				(std::vector<Soldier *>::const_iterator it = m_soldiers.begin();
				 it != soldiers_end;
				 ++it)
				(*it)->set_location(this);
		} else {
      // Request soldiers
		call_soldiers();
		}

      //    Should schedule because all stuff related to healing and removing own
      // soldiers should be scheduled.
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
	/*
	Economy* old = get_economy();
	uint32_t i;

	if (old) {
		for (i = 0; i < m_input_queues.size(); ++i)
			m_input_queues[i]->remove_from_economy(old);
	}

	Building::set_economy(e);

	if (e) {
		for (i = 0; i < m_input_queues.size(); ++i)
			m_input_queues[i]->add_to_economy(e);
	}
	*/
	/*Economy* old = get_economy();

	if (old) {
		for (uint32_t i = 0; i < m_soldier_requests.size(); ++i) {
			if (m_soldier_requests[i])
				m_soldier_requests[i]->remove_from_economy(old);
		}
	}
*/

   // TODO: SoldiersQueue migration
   ProductionSite::set_economy(e);

	if (e)
		for (uint32_t i = 0; i < m_soldier_requests.size(); ++i) {
			if (m_soldier_requests[i])
            m_soldier_requests[i]->set_economy(e);
		}
}

/**
===============
MilitarySite::cleanup

Cleanup after a military site is removed
===============
*/
void MilitarySite::cleanup(Editor_Game_Base* g)
{
   // Release worker
	if (m_soldier_requests.size()) {
		for (uint32_t i = 0; i < m_soldier_requests.size(); ++i) {
         delete m_soldier_requests[i];
         m_soldier_requests[i] = 0;
		}
      m_soldier_requests.resize(0);
	}

	for (uint32_t i = 0; i < m_soldiers.size(); ++i) {
      Soldier* s = m_soldiers[i];

      m_soldiers[i] = 0;
		if (g->objects().object_still_available(s))
         s->set_location(0);
	}
	// unconquer land
	if (m_didconquer)
		g->unconquer_area
			(Player_Area<Area<FCoords> >
			 (owner().get_player_number(),
			  Area<FCoords>(g->map().get_fcoords(get_position()), get_conquers())),
			 m_defeating_player);

	ProductionSite::cleanup(g);
}


/*
===============
MilitarySite::request_soldier

Issue the soldier request
===============
*/
void MilitarySite::request_soldier() {
   int32_t soldierid = get_owner()->tribe().get_safe_worker_index("soldier");

   // TODO: This should be user-configurable through windows options (still nothing is done to support this)
   Requeriments* r = new Requeriments();
   *r = m_soldier_requeriments;

	Request * req = new Request
		(this,
		 soldierid,
		 &MilitarySite::request_soldier_callback,
		 this,
		 Request::SOLDIER);
   req->set_requeriments (r);

   m_soldier_requests.push_back (req);

}


/*
===============
MilitarySite::request_soldier_callback [static]

Called when our soldier arrives.
===============
*/
void MilitarySite::request_soldier_callback
(Game * g, Request * rq, Ware_Index, Worker * w, void * data)
{

	MilitarySite & msite = *static_cast<MilitarySite *>(data);
   Soldier & s = dynamic_cast<Soldier &>(*w);

	assert(s.get_location(g) == &msite);

	if (not msite.m_didconquer)
		msite.conquer_area(*g);

	for (uint32_t i = 0; i < msite.m_soldier_requests.size(); ++i)
		if (rq == msite.m_soldier_requests[i]) {
			msite.m_soldier_requests.erase(msite.m_soldier_requests.begin() + i);
			break;
		}


	msite.m_soldiers.push_back(&s);

	// bind the worker into this house, hide him on the map
	s.start_task_idle(g, 0, -1);

	s.mark (false);
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

	uint32_t numMedics = 0; // FIX THIS when medics were added
      uint32_t i = 0;
      Soldier* s;

	uint32_t total_heal =
		descr().get_heal_per_second()
		+
		descr().get_heal_increase_per_medic() * numMedics;

	for (i = 0; i < m_soldiers.size(); ++i) {
         s = m_soldiers[i];

            // This is for clean up the soldier killed out of the building
			if (not s or s->get_current_hitpoints() == 0) {
            m_soldiers[i] = m_soldiers[m_soldiers.size() - 1];
            m_soldiers.pop_back();
            --i;
            continue;
			}

            // Fighting soldiers couldn't be healed !
		if (s->is_marked())
            continue;

            // Heal action

            // I don't like this 'healing' method, but I don't have any idea to do differently ...
			if (s->get_current_hitpoints() < s->get_max_hitpoints()) {
            s->heal (total_heal);
				total_heal -= total_heal / 3;
		}
	}
	if (not m_in_battle) call_soldiers();

      // Schedule the next wakeup at 1 second
   schedule_act (g, 1000);
}

/*
===============
Send the request for more soldiers if there are not full
===============
 */
void MilitarySite::call_soldiers() {
	while (m_capacity > m_soldiers.size() + m_soldier_requests.size())
		request_soldier();
}

/*
===============
MilitarySite::drop_soldier

Get out specied soldier from house.
===============
 */
void MilitarySite::drop_soldier (uint32_t serial)
{
molog ("**Dropping soldier (%d)\n", serial);

	if (upcast(Game, game, &owner().egbase()))
		if (m_soldiers.size()) {
			size_t i = 0;
      Soldier* s = m_soldiers[i];

			while (s and s->get_serial() != serial and i < m_soldiers.size()) {
         molog ("Serial: %d -- \n!", s->get_serial());
         i++;
         s = m_soldiers[i];
			}
			if (s)
         molog ("Serial: %d -- \n!", s->get_serial());

			if (s && s->get_serial() == serial) {
molog ("**--Sodier localized!\n");
         drop_soldier(game, i);
			} else
         molog ("--Soldier NOT localized!\n");
	}
}

/*
===============
MilitarySite::drop_soldier (Game *, int32_t)

Drops a soldier at specific position at its table. SHOULD NOT be called directly.
Use throught drop_soldier(int32_t)
===============
 */

void MilitarySite::drop_soldier (Game *g, int32_t nr)
{
      Soldier *s;

      // Check if its out of bounds
	if (nr < 0 or nr > static_cast<int32_t>(m_soldiers.size())) return;

      s = m_soldiers.at(nr);

      assert (s);
      s->set_location(0);

	{
		const std::vector<Soldier *>::const_iterator soldiers_end =
			m_soldiers.end();
		for
			(std::vector<Soldier *>::iterator
			 current = m_soldiers.begin() + nr, next = current + 1;
			 next < soldiers_end;
			 current = next, ++next)
			*current = *next;
	}

      m_soldiers.pop_back();

         // Call more soldiers if are enought space
	call_soldiers ();

         // Walk the soldier home safely
      s->mark (false);
      s->reset_tasks (g);
      s->set_location (this);
      s->start_task_leavebuilding (g, true);
}

/*
===========
MilitarySite::change_soldier_capacity

Changes the soldiers capacity.
===========
*/
void MilitarySite::change_soldier_capacity(int32_t how)
{
	if (how) {
		if (how > 0) {
         m_capacity += how;

			if
				(m_capacity > static_cast<uint32_t>(descr().get_max_number_of_soldiers()))
				m_capacity = static_cast<uint32_t>(descr().get_max_number_of_soldiers());
			call_soldiers();
		}
		else {
         how = -how;

			if (how >= static_cast<int32_t>(m_capacity)) m_capacity  = 1;
			else                                     m_capacity -= how;

			while (m_capacity < m_soldiers.size() + m_soldier_requests.size()) {
				if (m_soldier_requests.size()) {
					std::vector<Request*>::iterator it = m_soldier_requests.begin();
					for
						(;
						 it != m_soldier_requests.end() && !(*it)->is_open();
						 ++it);

					if (it == m_soldier_requests.end())
						(*--it)->cancel_transfer(0);
					else
						(*it)->get_economy()->remove_request(*it);

					m_soldier_requests.erase(it, it+1);
				}
				else if (m_soldiers.size()) {
               Soldier *s = m_soldiers[m_soldiers.size()-1];
               drop_soldier (s->get_serial());
				}
			}
		}
	}
}

void MilitarySite::init_after_conquering (Game* g, std::vector<Soldier*>* soldiers) {
	conquer_area(*g);
   m_soldiers.insert(m_soldiers.begin(), soldiers->begin(), soldiers->end());
   /*for (uint32_t i=0; i<soldiers->size(); ++i)
      m_soldiers.push_back((*soldies)[i]);*/
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

MilitarySite* MilitarySite::conquered_by (Game* g, Player* winner) {
   //NOT WORKING IMPLEMENTATION FOR CREATING A COMPLETLY NEW BUILDING
   cleanup(g);
   get_base_flag()->schedule_destroy(g);
	return static_cast<MilitarySite *>
		(static_cast<const MilitarySite_Descr &>(*m_descr).create
		 (*g, *winner, m_position, false));
}


/*
   MilitarySite::set_requeriments

   Easy to use, overwrite with given requeriments, pointer to faster running, so
   in releases can avoid the assert
*/
void MilitarySite::set_requeriments (Requeriments* R)
{
   assert (R);
   m_soldier_requeriments = (*R);
}

/*
   MilitarySite::clear_requeriments

   This should cancel any requeriment pushed at this house
*/
void MilitarySite::clear_requeriments ()
{
   Requeriments R;
   m_soldier_requeriments = R;
}

uint32_t MilitarySite::nr_not_marked_soldiers() {
	if (m_soldiers.size() <= 0)
      return 0;
   uint32_t nr_soldiers = 0;
	for (uint32_t i = 0; i < m_soldiers.size(); ++i) {
		if (!m_soldiers[i]->is_marked())
         nr_soldiers++;
	}
   return nr_soldiers;
}

uint32_t MilitarySite::nr_attack_soldiers() {
   uint32_t not_marked = nr_not_marked_soldiers();
	if (not_marked > 1)
      return not_marked-1;
   return 0;
}

};
