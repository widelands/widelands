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

#include "militarysite.h"

#include "editor_game_base.h"
#include "error.h"
#include "game.h"
#include "i18n.h"
#include "player.h"
#include "profile.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "worker.h"

#include <libintl.h>
#include <locale.h>
#include <stdio.h>


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
MilitarySite::get_statistics_string

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


/**
===============
MilitarySite::init

Initialize the military site.
===============
*/
void MilitarySite::init(Editor_Game_Base* g)
{
   ProductionSite::init(g);

	if (Game * const game = dynamic_cast<Game *>(g)) {
      // Request soldiers
		call_soldiers();

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
	uint i;

	if (old) {
		for (i = 0; i < m_input_queues.size(); i++)
			m_input_queues[i]->remove_from_economy(old);
	}

	Building::set_economy(e);

	if (e) {
		for (i = 0; i < m_input_queues.size(); i++)
			m_input_queues[i]->add_to_economy(e);
	}
	*/
	/*Economy* old = get_economy();

	if (old) {
		for (uint i = 0; i < m_soldier_requests.size(); i++) {
			if (m_soldier_requests[i])
				m_soldier_requests[i]->remove_from_economy(old);
		}
	}
*/

   // TODO: SoldiersQueue migration
   ProductionSite::set_economy(e);

   if (e) {
      for (uint i = 0; i < m_soldier_requests.size(); i++) {
         if (m_soldier_requests[i])
            m_soldier_requests[i]->set_economy(e);
		}
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
   if (m_soldier_requests.size())
   {
      for (uint i = 0; i < m_soldier_requests.size(); i++)
      {
         delete m_soldier_requests[i];
         m_soldier_requests[i] = 0;
		}
      m_soldier_requests.resize(0);
	}

   uint i;
   for (i=0; i < m_soldiers.size(); i++)
   {
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
void MilitarySite::request_soldier() {
   int soldierid = get_owner()->tribe().get_safe_worker_index("soldier");

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
(Game * g, Request * rq, int, Worker * w, void * data)
{

   MilitarySite* msite = (MilitarySite*)data;
   Soldier* s=static_cast<Soldier*>(w);

   assert(s);
   assert(s->get_location(g) == msite);

	if (not msite->m_didconquer)
		g->conquer_area
			(Player_Area<Area<FCoords> >
			 (msite->owner().get_player_number(),
			  Area<FCoords>
			  (g->map().get_fcoords(msite->get_position()),
			   msite->descr().get_conquers())));
   msite->m_didconquer = true;

   uint i=0;
   for (i=0; i<msite->m_soldier_requests.size(); i++)
      if (rq==msite->m_soldier_requests[i]) break;

   msite->m_soldier_requests.erase(msite->m_soldier_requests.begin() + i);

   msite->m_soldiers.push_back(s);
   s->start_task_idle(g, 0, -1); // bind the worker into this house, hide him on the map
	s->mark (false);
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
   ProductionSite::act(g, data);

	uint numMedics = 0; // FIX THIS when medics were added
      uint i = 0;
      Soldier* s;

	uint total_heal =
		descr().get_heal_per_second()
		+
		descr().get_heal_increase_per_medic() * numMedics;

      for (i=0; i < m_soldiers.size(); i++) {
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
MilitarySite::call_soldiers

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
void MilitarySite::drop_soldier (uint serial)
{
molog ("**Dropping soldier (%d)\n", serial);

	if (Game * const game = dynamic_cast<Game *>(&owner().egbase()))
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

      if ((s) && (s->get_serial() == serial))
      {
molog ("**--Sodier localized!\n");
         drop_soldier(game, i);
		}
      else
         molog ("--Soldier NOT localized!\n");
	}
}

/*
===============
MilitarySite::drop_soldier (Game *, int)

Drops a soldier at specific position at its table. SHOULD NOT be called directly.
Use throught drop_soldier(int)
===============
 */

void MilitarySite::drop_soldier (Game *g, int nr)
{
      Soldier *s;

      // Check if its out of bounds
	if (nr < 0 or nr > static_cast<int>(m_soldiers.size())) return;

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
void MilitarySite::change_soldier_capacity(int how)
{
	if (how) {
		if (how > 0) {
         m_capacity += how;

			if
				(m_capacity > static_cast<uint>(descr().get_max_number_of_soldiers()))
				m_capacity = static_cast<uint>(descr().get_max_number_of_soldiers());
			call_soldiers();
		}
		else {
         how = -how;

			if (how >= static_cast<int>(m_capacity)) m_capacity  = 1;
			else                                     m_capacity -= how;

			while (m_capacity < m_soldiers.size() + m_soldier_requests.size()) {
				if (m_soldier_requests.size()) {
	       std::vector<Request*>::iterator it = m_soldier_requests.begin();
	       for (; it != m_soldier_requests.end() && !(*it)->is_open(); ++it);

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
	g->conquer_area
		(Player_Area<Area<FCoords> >
		 (owner().get_player_number(),
		  Area<FCoords>(g->map().get_fcoords(get_position()), get_conquers())));
   m_didconquer = true;
   m_soldiers.insert(m_soldiers.begin(), soldiers->begin(), soldiers->end());
   /*for (uint i=0; i<soldiers->size(); i++)
      m_soldiers.push_back((*soldies)[i]);*/
}

MilitarySite* MilitarySite::conquered_by (Game* g, Player* winner) {
   //NOT WORKING IMPLEMENTATION FOR CREATING A COMPLETLY NEW BUILDING
   cleanup(g);
   get_base_flag()->schedule_destroy(g);
	return static_cast<MilitarySite *>
		(static_cast<const MilitarySite_Descr &>(*m_descr)
		 .create(*g, *winner, m_position, false));

   //IMPLEMENTATION FOR OVERTAKING EXISITING BUILDING
   //FIXME: Coorect implementation, someone with deeper knowledge needs
   //to look into this.
  /* log("starting to change owner...\n");
    // Release worker
   if (m_soldier_requests.size())
   {
      for (uint i = 0; i < m_soldier_requests.size(); i++)
      {
         delete m_soldier_requests[i];
         m_soldier_requests[i] = 0;
		}
      m_soldier_requests.resize(0);
	}
   log("removed all soldier requests.\n");
   uint i;
   for (i=0; i < m_soldiers.size(); i++)
   {
      Soldier* s = m_soldiers[i];

      m_soldiers[i] = 0;
		if (g->objects().object_still_available(s))
         s->set_location(0);
	}
   log("removed all remainig soldiers\n");

   if (m_didconquer)
	  g->unconquer_area(get_owner()->get_player_number(), get_position());

   Flag* f = get_base_flag();

   //  Destroy roads
   log ("[Donquered - MilitarySite] : Destroying roads\n");
   if (f->get_road(WALK_NE)) f->detach_road(WALK_NE);
   if (f->get_road(WALK_E))  f->detach_road(WALK_E);
   if (f->get_road(WALK_SE)) f->detach_road(WALK_SE);
   if (f->get_road(WALK_W))  f->detach_road(WALK_W);
   if (f->get_road(WALK_SW)) f->detach_road(WALK_SW);
   log("destroyed roads...\n");

   get_economy()->remove_flag(f);
   log("removed flag from economy\n");

   set_owner(who);
   log("owner of building set\n");

   f->set_owner(who);
   log("owner of flag set\n");

   who->get_economy_by_number(who->get_player_number())->add_flag(f);
   log("flag added economy\n");

   g->conquer_area(get_owner()->get_player_number(), get_position(), get_descr());

   // unconquer land


   //g->unconquer_area(get_owner()->get_player_number(), get_position());

/* ORIGINAL CODE FOLLOWS
   // unconquer land
   if (m_didconquer)
      g->unconquer_area(get_owner()->get_player_number(), get_position());
molog("%s %d\n", __FILE__, __LINE__);
   g->player_immovable_notification(this, Game::LOSE);
molog("%s %d\n", __FILE__, __LINE__);

      //  Become the new owner
   set_owner (who);

   Become the owner of the fields
   Field* fi = get_owner()->get_game()->get_map()->get_field(get_position());
   assert (fi);
   fi->set_owned_by (get_owner()->get_player_number());

   fi = get_owner()->get_game()->get_map()->get_field(f->get_position());
   assert(fi);
   fi->set_owned_by (get_owner()->get_player_number());

   Become the owner of the  base flag
   f->conquered_by (who);

   Reconquer land (crash arround here)
   if (m_didconquer)
      g->conquer_area(who->get_player_number(), get_position(), get_descr());
molog("%s %d\n", __FILE__, __LINE__);
   g->player_immovable_notification(this, Game::GAIN);
molog("%s %d\n", __FILE__, __LINE__);*/

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

uint MilitarySite::nr_not_marked_soldiers() {
   if (m_soldiers.size() <= 0) {
      return 0;
	}
   uint nr_soldiers = 0;
	for (uint i = 0; i < m_soldiers.size(); ++i) {
      if (!m_soldiers[i]->is_marked())
         nr_soldiers++;
	}
   return nr_soldiers;
}

uint MilitarySite::nr_attack_soldiers() {
   uint not_marked = nr_not_marked_soldiers();
   if (not_marked > 1)
      return not_marked-1;
   return 0;
}
