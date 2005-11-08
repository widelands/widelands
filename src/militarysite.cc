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

#include <stdio.h>
#include "editor_game_base.h"
#include "error.h"
#include "game.h"
#include "militarysite.h"
#include "player.h"
#include "profile.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "worker.h"
#include "system.h"

/**
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

	ProductionSite_Descr::parse(directory,prof,encdata);
   // Defaults to false
   m_stopable=false;
   Building_Descr::parse(directory, prof, encdata);

	m_conquer_radius=sglobal->get_safe_int("conquers");
	m_num_soldiers=sglobal->get_safe_int("max_soldiers");
	m_num_medics=sglobal->get_safe_int("max_medics");
	m_heal_per_second=sglobal->get_safe_int("heal_per_second");
	m_heal_incr_per_medic=sglobal->get_safe_int("heal_increase_per_medic");
	if (m_conquer_radius > 0) {
		std::string description (get_descname());
		description += " conquer";
		m_workarea_info[m_conquer_radius].insert(description);
	}
}

/**
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

/**
===============
MilitarySite::MilitarySite
===============
*/
MilitarySite::MilitarySite(MilitarySite_Descr* descr)
	: ProductionSite(descr)
{
	m_didconquer = false;
	m_capacity = descr->get_max_number_of_soldiers();
}


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
   char str[255];
   if (m_soldier_requests.size())
      snprintf (str, 255, _("%d soldiers (+%d)"), (int)m_soldiers.size(), (int)m_soldier_requests.size());
   else
      snprintf (str, 255, _("%d soldiers"), (int)m_soldiers.size());
   
   return str;
}


/**
===============
MilitarySite::init

Initialize the military site.
===============
*/
void MilitarySite::init(Editor_Game_Base* g)
{
log (">>MilitarySite::init()\n");
   ProductionSite::init(g);

   if (g->is_game()) 
   {
      // Request soldiers
      call_soldiers((Game *) g);
      
      //    Should schedule because all stuff related to healing and removing own
      // soldiers should be scheduled.
      schedule_act((Game*)g, 1000);
   }
log ("<<MilitarySite::init()\n");
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
		for(i = 0; i < m_input_queues.size(); i++)
			m_input_queues[i]->remove_from_economy(old);
	}

	Building::set_economy(e);

	if (e) {
		for(i = 0; i < m_input_queues.size(); i++)
			m_input_queues[i]->add_to_economy(e);
	}
	*/
/*	Economy* old = get_economy();

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
   for(i=0; i < m_soldiers.size(); i++) 
   {
      Soldier* s = m_soldiers[i];

      m_soldiers[i] = 0;
      if(g->get_objects()->object_still_available(s))
         s->set_location(0);
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
   int soldierid = get_owner()->get_tribe()->get_safe_worker_index("soldier");

   // TODO: This should be user-configurable through windows options (still nothing is done to support this)
   Requeriments* r = new Requeriments();
   *r = m_soldier_requeriments;

   Request* req = new Request(this,	soldierid, &MilitarySite::request_soldier_callback, this, Request::SOLDIER);
   req->set_requeriments (r);

   m_soldier_requests.push_back (req);

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

   MilitarySite* msite = (MilitarySite*)data;
   Soldier* s=static_cast<Soldier*>(w);
   
   assert(s);
   assert(s->get_location(g) == msite);

   if (!msite->m_didconquer)
      g->conquer_area(msite->get_owner()->get_player_number(),
   msite->get_position(), msite->get_descr());
   msite->m_didconquer = true;
   
   uint i=0;
   for(i=0; i<msite->m_soldier_requests.size(); i++)
      if(rq==msite->m_soldier_requests[i]) break;

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
   ProductionSite::act(g,data);

   if (g->is_game ()) {
      uint total_heal = 0;
      uint numMedics = 0;	 // FIX THIS when medics were added
      uint i = 0;
      Soldier* s;

      total_heal = get_descr()->get_heal_per_second();
      total_heal += get_descr()->get_heal_increase_per_medic() * numMedics;

      for (i=0; i < m_soldiers.size(); i++) {
         s = m_soldiers[i];
            
            // This is for clean up the soldier killed out of the building
         if (!s ||(s->get_current_hitpoints() == 0))
         {
            m_soldiers[i] = m_soldiers[m_soldiers.size() - 1];
            m_soldiers.pop_back();
            i--;
            continue;
         }
            
            // Fighting soldiers couldn't be healed !
         if (s->is_marked())
            continue;

            // Heal action
            
            // I don't like this 'healing' method, but I don't have any idea to do differently ...
         if (s->get_current_hitpoints() < s->get_max_hitpoints()) {
            s->heal (total_heal);
            total_heal -=	(total_heal/3);
         }
      }
   }
   call_soldiers(g);
   
      // Schedule the next wakeup at 1 second
   schedule_act (g, 1000);
}

/*
===============
MilitarySite::call_soldiers

Send the request for more soldiers if there are not full
===============
 */
void MilitarySite::call_soldiers(Game *g) 
{
   if (g->is_game()) 
      while(m_capacity > m_soldiers.size() + m_soldier_requests.size()) 
         request_soldier(g);
}

/*
===============
MilitarySite::drop_soldier

Get out specied soldier from house.
===============
 */
void MilitarySite::drop_soldier (uint serial) 
{
   Game* g = (Game *)get_owner()->get_game();

molog ("**Dropping soldier (%d)\n", serial);

   if (g->is_game() && m_soldiers.size()) 
   {
      int i = 0;
      Soldier* s = m_soldiers[i];
      
      while ((s) && (s->get_serial() != serial) && (i < (int)m_soldiers.size())) 
      {
         molog ("Serial: %d -- \n!", s->get_serial());
         i++;
         s = m_soldiers[i];
      }
      if (s)
         molog ("Serial: %d -- \n!", s->get_serial());

      if ((s) && (s->get_serial() == serial)) 
      {
molog ("**--Sodier localized!\n");
         drop_soldier(g, i);
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
   if (g->is_game()) 
   {
      Soldier *s;

      // Check if its out of bounds
      if ((nr < 0) || (nr > (int) m_soldiers.size()))
         return;

      s = m_soldiers[nr];
      
      assert (s);
      s->set_location(0);

      for (uint i = nr; i < m_soldiers.size(); i++)
         m_soldiers[i] = m_soldiers[i+1];

      m_soldiers.pop_back();

         // Call more soldiers if are enought space
      call_soldiers (g);

         // Walk the soldier home safely
      s->mark (false);
      s->reset_tasks (g);
      s->set_location (this);
      s->start_task_leavebuilding (g, true);
   }
}

/*
===========
MilitarySite::change_soldier_capacity

Changes the soldiers capacity.
===========
*/
void MilitarySite::change_soldier_capacity(int how) 
{
   if (how) 
   {
      if (how > 0) 
      {
         m_capacity += how;

         if (m_capacity > (uint) get_descr()->get_max_number_of_soldiers())
            m_capacity = (uint) get_descr()->get_max_number_of_soldiers();
         call_soldiers((Game*)get_owner()->get_game());
      }
      else 
      {
         how = -how;
         
         if (how >= (int) m_capacity)
            m_capacity = 1;
         else
            m_capacity -= how;
         
         while (m_capacity < m_soldiers.size() + m_soldier_requests.size())
         {
            if (m_soldier_requests.size()) 
            {
	       std::vector<Request*>::iterator it = m_soldier_requests.begin();
	       for ( ; it != m_soldier_requests.end() && !(*it)->is_open(); ++it);

	       if (it == m_soldier_requests.end())
		  (*--it)->cancel_transfer(0);
	       else
		  (*it)->get_economy()->remove_request(*it);

	       m_soldier_requests.erase(it, it+1);
            }
            else if (m_soldiers.size()) 
            {
               Soldier *s = m_soldiers[m_soldiers.size()-1];
               drop_soldier (s->get_serial());
	    }
	 }
      }
   }
}

/// Type : STRONGEST - WEAKEST
int MilitarySite::launch_attack(PlayerImmovable* p_imm, int type)
{
   uint launched = 0;
   int i = 0;
   Soldier* s = 0;
   
   assert (p_imm->get_type() == FLAG);
   Flag* flag = (Flag*) p_imm;
   Game* g = (Game*) get_owner()->get_game();
   
   if (!m_soldiers.size())
      return 0;

   switch (type)
   {
      case STRONGEST:
         for (uint j = 0; j < m_soldiers.size(); j++)
            if (! m_soldiers[j]->is_marked())
               if (m_soldiers[i]->get_level (atrTotal) < m_soldiers[j]->get_level (atrTotal))
                  i = j;
         break;
      case WEAKEST:
         for (uint j = 0; j < m_soldiers.size(); j++)
            if (! m_soldiers[j]->is_marked())
               if (m_soldiers[i]->get_level (atrTotal) > m_soldiers[j]->get_level (atrTotal))
                  i = j;
         break;
      default:
         throw wexception ("Unkown type of attack (%d)", type);
   }
   
   s = m_soldiers[i];
   
   if (s)
   {
      if (s->is_marked())
         return 0;
      
molog("Launching soldier %d\n", s->get_serial());
      s->mark(true);
      s->reset_tasks(g);
      s->set_location(this);
      s->start_task_launchattack(g, flag);
      m_soldiers[i] = m_soldiers[m_soldiers.size() - 1];
      m_soldiers.pop_back();
      launched++;
   }
   return launched;
}

//*** TESTING STUFF ***////
void MilitarySite::defend (Game* g, Soldier* s)
{
   assert(s);
   uint i = 0, J = 9999;
   molog ("[MilitarySite] We are under attack of %d!\n", s->get_serial());
   assert(m_soldiers.size() > 0);

// TODO: Here may be extra checks
   for (i = 0; i < m_soldiers.size(); ++i)
   {
      Soldier* so = m_soldiers[i];
      
      if (!so)
         continue;

      if (so->is_marked())
         continue;

      so->mark(true);
      so->reset_tasks(g);
      so->set_location(this);
      so->start_task_defendbuilding (g, this, s);
      J=0;
      break;
   }
   if (J == 9999)
      s->send_signal(g, "fail");
   else    
      call_soldiers(g);
}

void MilitarySite::conquered_by (Player* who)
{
//schedule_destroy((Game*)get_owner()->get_game());
  // Ensures that all is correct
   assert (who);
   assert (get_owner());

      // Getting game
   Game* g = (Game*)get_owner()->get_game();
   
   // Release worker
   molog("[Conquered MilitarySite]: Releasing Workers\n");
   if (m_soldier_requests.size())
   {
      for (uint i = 0; i < m_soldier_requests.size(); i++)
      {
         delete m_soldier_requests[i];
         m_soldier_requests[i] = 0;
      }
      m_soldier_requests.resize(0);
   }

   Flag* f = get_base_flag();
   assert (f);
   
      //  Destroy roads
   molog ("[Donquered - MilitarySite] : Destroying roads\n");
   if (f->get_road(WALK_NE)) f->detach_road(WALK_NE);
   if (f->get_road(WALK_E))  f->detach_road(WALK_E);
   if (f->get_road(WALK_SE)) f->detach_road(WALK_SE);
   if (f->get_road(WALK_W))  f->detach_road(WALK_W);
   if (f->get_road(WALK_SW)) f->detach_road(WALK_SW);

     // Destroying (at future will be )
   //f->schedule_destroy(g);
   schedule_destroy(g);
   

/*
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
