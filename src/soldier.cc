/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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
#include "battle.h"
#include "editor_game_base.h"
#include "error.h"
#include "game.h"
#include "graphic.h"
#include "militarysite.h"
#include "player.h"
#include "profile.h"
#include "rendertarget.h"
#include "rgbcolor.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "util.h"
#include "warehouse.h"
#include "wexception.h"


/*
==============================================================================

IdleSoldierSupply IMPLEMENTATION

==============================================================================
*/

class IdleSoldierSupply : public Supply {
	public:
		IdleSoldierSupply(Soldier * const);
		~IdleSoldierSupply();

		void set_economy(Economy* e);

	public:
		virtual PlayerImmovable* get_position(Game* g);
		virtual int get_amount(Game* g, int ware);
		virtual bool is_active(Game* g);

		virtual WareInstance* launch_item(Game* g, int ware);
		virtual Worker* launch_worker(Game* g, int ware);

		virtual Soldier* launch_soldier(Game* g, int ware, Requeriments* req);
		virtual int get_passing_requeriments(Game* g, int ware, Requeriments* r);
		virtual void mark_as_used (Game* g, int ware, Requeriments* r);
	private:
		Soldier*		m_soldier;
		Economy*		m_economy;
};


/*
===============
IdleSoldierSupply::IdleSoldierSupply

Automatically register with the soldier's economy.
===============
*/
IdleSoldierSupply::IdleSoldierSupply(Soldier * const s) :
m_soldier(s), m_economy(0)
{set_economy(s->get_economy());}


/*
===============
IdleSoldierSupply::~IdleSoldierSupply

Automatically unregister from economy.
===============
*/
IdleSoldierSupply::~IdleSoldierSupply()
{
	set_economy(0);
}


/*
===============
IdleSoldierSupply::set_economy

Add/remove this supply from the Economy as appropriate.
===============
*/
void IdleSoldierSupply::set_economy(Economy* e)
{
	if (m_economy == e)
		return;

	if (m_economy)
		m_economy->remove_soldier_supply(m_soldier->get_owner()->get_tribe()->get_worker_index(m_soldier->get_name().c_str()), this);

	m_economy = e;

	if (m_economy)
		m_economy->add_soldier_supply(m_soldier->get_owner()->get_tribe()->get_worker_index(m_soldier->get_name().c_str()), this);
}


/*
===============
IdleSoldierSupply::get_position

Return the soldier's position.
===============
*/
PlayerImmovable* IdleSoldierSupply::get_position(Game* g)
{
	return m_soldier->get_location(g);
}


/*
===============
IdleSoldierSupply::get_amount

It's just the one soldier.
===============
*/
int IdleSoldierSupply::get_amount(Game* g, int ware)
{
	if (ware == m_soldier->get_owner()->get_tribe()->get_worker_index(m_soldier->get_name().c_str()))
		return 1;

	return 0;
}


/*
===============
IdleSoldierSupply::is_active

Idle soldiers are always active supplies, because they need to get into a
Warehouse ASAP.
===============
*/
bool IdleSoldierSupply::is_active(Game* g)
{
	return true;
}


/*
===============
IdleSoldierSupply::launch_item
===============
*/
WareInstance* IdleSoldierSupply::launch_item(Game* g, int ware)
{
	throw wexception("IdleSoldierSupply::launch_item() makes no sense.");
}


/*
===============
IdleSodlierSupply::launch_worker
===============
*/
Worker* IdleSoldierSupply::launch_worker(Game* g, int ware)
{
	log ("IdleSoldierSupply::launch_worker() Warning something can go wrong around here.\n");

	return launch_soldier(g, ware, NULL);
}


/*
===============
IdleSodlierSupply::launch_soldier

No need to explicitly launch the soldier.
===============
*/
Soldier* IdleSoldierSupply::launch_soldier(Game* g, int ware, Requeriments* req)
{
	assert(ware == m_soldier->get_owner()->get_tribe()->get_worker_index(m_soldier->get_name().c_str()));

	if (req)
	{
		if (req->check (
				  m_soldier->get_level(atrHP),
		m_soldier->get_level(atrAttack),
		m_soldier->get_level(atrDefense),
		m_soldier->get_level(atrEvade)
							))
		{
			// Ensures that this soldier is used now
			m_soldier->mark (false);
			return m_soldier;
		}
		else
			throw wexception ("IdleSoldierSupply::launch_soldier Fails. Requeriments aren't accomplished.");
	}
	else
		return m_soldier;
}

/*
===============
IdleSodlierSupply::mark_as_used
===============
*/
void IdleSoldierSupply::mark_as_used(Game* g, int ware, Requeriments* req)
{
	assert(ware == m_soldier->get_owner()->get_tribe()->get_worker_index(m_soldier->get_name().c_str()));

	if (req)
	{
		if (req->check (
				  m_soldier->get_level(atrHP),
		m_soldier->get_level(atrAttack),
		m_soldier->get_level(atrDefense),
		m_soldier->get_level(atrEvade)
							))
		{
			// Ensures that this soldier has a request now
			m_soldier->mark (true);
		}
		else
			throw wexception ("IdleSoldierSupply::launch_soldier Fails. Requeriments aren't accomplished.");
	}
	else
		m_soldier->mark (true);
}


/*
===============
IdleSodlierSupply::get_passing_requeriments

No need to explicitly launch the soldier.
===============
*/
int IdleSoldierSupply::get_passing_requeriments(Game* g, int ware, Requeriments* req)
{
	assert(ware == m_soldier->get_owner()->get_tribe()->get_worker_index(m_soldier->get_name().c_str()));

	// Oops we find a marked soldied (in use)
	if (m_soldier->is_marked())
		return 0;

	if (!req)
		return 1;

	if (req->check (
			m_soldier->get_level(atrHP),
	m_soldier->get_level(atrAttack),
	m_soldier->get_level(atrDefense),
	m_soldier->get_level(atrEvade)
						))
		return 1;
	else
		return 0;
}




/*
==============================================================

SOLDIER DESCRIPTION IMPLEMENTATION

==============================================================
*/

/*
===============
Soldier_Descr::Soldier_Descr
Soldier_Descr::~Soldier_Descr
===============
*/
Soldier_Descr::Soldier_Descr(Tribe_Descr * const tribe, const char * const name)
	: Worker_Descr(tribe, name)
{
	add_attribute(Map_Object::SOLDIER);
}

Soldier_Descr::~Soldier_Descr(void)
{
   m_hp_pics_fn.resize(0);
   m_attack_pics_fn.resize(0);
   m_defense_pics_fn.resize(0);
   m_evade_pics_fn.resize(0);
}

/**
===============
Soldier_Descr::parse

Parse carrier-specific configuration data
===============
*/
void Soldier_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Worker_Descr::parse(directory, prof, encdata);
   Section* sglobal=prof->get_section("global");

   // Parse hitpoints
   std::string hp=sglobal->get_safe_string("hp");
   std::vector<std::string> list;
   split_string(hp, &list, "-");
   if(list.size()!=2)
      throw wexception("Parse error in hp string: \"%s\" (must be \"min-max\")", hp.c_str());
   uint i=0;
   for(i=0; i<list.size(); i++)
      remove_spaces(&list[i]);
   char* endp;
   m_min_hp= strtol(list[0].c_str(),&endp, 0);
   if(endp && *endp)
      throw wexception("Parse error in hp string: %s is a bad value", list[0].c_str());
   m_max_hp = strtol(list[1].c_str(),&endp, 0);
   if(endp && *endp)
      throw wexception("Parse error in hp string: %s is a bad value", list[1].c_str());

   // Parse attack
   std::string attack=sglobal->get_safe_string("attack");
   list.resize(0);
   split_string(attack, &list, "-");
   if(list.size()!=2)
      throw wexception("Parse error in attack string: \"%s\" (must be \"min-max\")", attack.c_str());
   for(i=0; i<list.size(); i++)
      remove_spaces(&list[i]);
   m_min_attack= strtol(list[0].c_str(),&endp, 0);
   if(endp && *endp)
      throw wexception("Parse error in attack string: %s is a bad value", list[0].c_str());
   m_max_attack = strtol(list[1].c_str(),&endp, 0);
   if(endp && *endp)
      throw wexception("Parse error in attack string: %s is a bad value", list[1].c_str());

   // Parse defend
   m_defense=sglobal->get_safe_int("defense");

   // Parse evade
   m_evade=sglobal->get_safe_int("evade");

   // Parse increases per level
   m_hp_incr=sglobal->get_safe_int("hp_incr_per_level");
   m_attack_incr=sglobal->get_safe_int("attack_incr_per_level");
   m_defense_incr=sglobal->get_safe_int("defense_incr_per_level");
   m_evade_incr=sglobal->get_safe_int("evade_incr_per_level");

   // Parse max levels
   m_max_hp_level=sglobal->get_safe_int("max_hp_level");
   m_max_attack_level=sglobal->get_safe_int("max_attack_level");
   m_max_defense_level=sglobal->get_safe_int("max_defense_level");
   m_max_evade_level=sglobal->get_safe_int("max_evade_level");

   // Load the filenames
   m_hp_pics_fn.resize(m_max_hp_level+1);
   m_attack_pics_fn.resize(m_max_attack_level+1);
   m_defense_pics_fn.resize(m_max_defense_level+1);
   m_evade_pics_fn.resize(m_max_evade_level+1);
   char buffer[256];
   std::string dir=directory;
   dir+="/";
   for(i=0; i<=m_max_hp_level; i++) {
      sprintf(buffer, "hp_level_%i_pic", i);
      m_hp_pics_fn[i]=dir;
      m_hp_pics_fn[i]+=sglobal->get_safe_string(buffer);
   }
   for(i=0; i<=m_max_attack_level; i++) {
      sprintf(buffer, "attack_level_%i_pic", i);
      m_attack_pics_fn[i]=dir;
      m_attack_pics_fn[i]+=sglobal->get_safe_string(buffer);
   }
   for(i=0; i<=m_max_defense_level; i++) {
      sprintf(buffer, "defense_level_%i_pic", i);
      m_defense_pics_fn[i]=dir;
      m_defense_pics_fn[i]+=sglobal->get_safe_string(buffer);
   }
   for(i=0; i<=m_max_evade_level; i++) {
      sprintf(buffer, "evade_level_%i_pic", i);
      m_evade_pics_fn[i]=dir;
      m_evade_pics_fn[i]+=sglobal->get_safe_string(buffer);
   }
}

/**
 * Load the graphics
 */
void Soldier_Descr::load_graphics(void) {
   m_hp_pics.resize(m_max_hp_level+1);
   m_attack_pics.resize(m_max_attack_level+1);
   m_defense_pics.resize(m_max_defense_level+1);
   m_evade_pics.resize(m_max_evade_level+1);
   uint i;
   for(i=0; i<=m_max_hp_level; i++) {
      m_hp_pics[i]=g_gr->get_picture( PicMod_Game,  m_hp_pics_fn[i].c_str() );
   }
   for(i=0; i<=m_max_attack_level; i++) {
      m_attack_pics[i]=g_gr->get_picture( PicMod_Game,  m_attack_pics_fn[i].c_str() );
   }
   for(i=0; i<=m_max_defense_level; i++) {
      m_defense_pics[i]=g_gr->get_picture( PicMod_Game,  m_defense_pics_fn[i].c_str() );
   }
   for(i=0; i<=m_max_evade_level; i++) {
      m_evade_pics[i]=g_gr->get_picture( PicMod_Game,  m_evade_pics_fn[i].c_str() );
   }
   Worker_Descr::load_graphics();
}


/**
 * Get random animation of specified type
 */
uint Soldier_Descr::get_rand_anim(std::string name)
{
   // Todo: This is thought to get a random animation like attack_1 attack_2 attack_3 ...
   // Randimly trhought this method. By now only gets attack, but isn't very difficult
   // to remake allowing the attack_1 and so.
   return get_animation(name.c_str());
}

/**
 * Create a new soldier
 */
Bob* Soldier_Descr::create_object() {
   return new Soldier(this);
}

/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Soldier::Soldier
Soldier::~Soldier
===============
*/
Soldier::Soldier(Soldier_Descr *descr)
	: Worker(descr)
{
   // all done through init
}

Soldier::~Soldier()
{
}

/*
 * Initialize this soldier
 */
void Soldier::init(Editor_Game_Base* gg) {
	m_hp_level=0;
	m_attack_level=0;
	m_defense_level=0;
	m_evade_level=0;

	m_hp_max=0;
	m_min_attack=get_descr()->get_min_attack();
	m_max_attack=get_descr()->get_max_attack();
	m_defense=get_descr()->get_defense();
	m_evade=get_descr()->get_evade();
	if(gg->is_game()) {
		Game* g= static_cast<Game*>(gg);
		int range=get_descr()->get_max_hp()-get_descr()->get_min_hp();
		int value=g->logic_rand() % range;
		m_hp_max=value;
	}
	m_hp_current=m_hp_max;

	m_marked = false;

	Worker::init(gg);
}

/*
 * Set this soldiers level. Automatically sets the new values
 */
void Soldier::set_level
(const uint hp, const uint attack, const uint defense, const uint evade)
{
   set_hp_level(hp);
   set_attack_level(attack);
   set_defense_level(defense);
   set_evade_level(evade);
}
void Soldier::set_hp_level(const uint hp) {
   assert(hp>=m_hp_level && hp<=get_descr()->get_max_hp_level());

   while(m_hp_level<hp) {
      ++m_hp_level;
      m_hp_max+=get_descr()->get_hp_incr_per_level();
      m_hp_current+=get_descr()->get_hp_incr_per_level();
   }
}
void Soldier::set_attack_level(const uint attack) {
   assert(attack>=m_attack_level && attack<=get_descr()->get_max_attack_level());

   while(m_attack_level<attack) {
      ++m_attack_level;
      m_min_attack+=get_descr()->get_attack_incr_per_level();
      m_max_attack+=get_descr()->get_attack_incr_per_level();
   }
}
void Soldier::set_defense_level(const uint defense) {
   assert(defense>=m_defense_level && defense<=get_descr()->get_max_defense_level());

   while(m_defense_level<defense) {
      ++m_defense_level;
      m_defense+=get_descr()->get_defense_incr_per_level();
   }
}
void Soldier::set_evade_level(const uint evade) {
   assert(evade>=m_evade_level && evade<=get_descr()->get_max_evade_level());

   while(m_evade_level<evade) {
      ++m_evade_level;
      m_evade+=get_descr()->get_evade_incr_per_level();
   }
}

uint Soldier::get_level(const tAttribute at)
{
	switch (at)
	{
		case atrHP:        return m_hp_level;
		case atrAttack:    return m_attack_level;
		case atrDefense:   return m_defense_level;
		case atrEvade:     return m_evade_level;
      case atrTotal: return m_hp_level + m_attack_level + m_defense_level + m_evade_level;
	}
	throw wexception ("Soldier::get_level attribute not identified.)");
}

void Soldier::heal (const uint hp)
{
	// Ensures that we can only heal, don't hurt throught this method
	assert (hp >= 0);

   molog ("healing (%d+)%d/%d\n", hp, m_hp_current, m_hp_max);
	m_hp_current += hp;

	if (m_hp_current > m_hp_max)
		m_hp_current = m_hp_max;
}

/**
 * This only subs the specified number of hitpoints, don't do anything more.
 */
void Soldier::damage (const uint value)
{
   assert (m_hp_current > 0);

   molog ("damage %d(-%d)/%d\n", m_hp_current, value, m_hp_max);
   if (m_hp_current < value)
      m_hp_current = 0;
   else
      m_hp_current -= value;
}

/*
 * Draw this soldier. This basically draws him as a worker, but add hitpoints
 */
void Soldier::draw
(const Editor_Game_Base & game, RenderTarget & dst, const Point pos) const
{
	const uint anim = get_current_anim();

	if (!anim)
		return;

	const Point drawpos = calc_drawpos(game, pos);

   int w, h;
	g_gr->get_animation_size
		(anim, game.get_gametime() - get_animstart(), &w, &h);

   // Draw energy bar
   // first: draw white sourrounding
   const int frame_width = w * 2;
   const int frame_height=5;
   const int frame_beginning_x = drawpos.x - w;  // TODO: these should be calculated from the hot spot, not assumed
   const int frame_beginning_y=drawpos.y-h-7;
   dst.draw_rect
		(frame_beginning_x, frame_beginning_y,
		 frame_width, frame_height,
		 HP_FRAMECOLOR);
	// Draw the actual bar
	const float percent = static_cast<float>(m_hp_current) / m_hp_max;
	const int energy_width = static_cast<int>(percent * (frame_width - 2));
	RGBColor color;
   if (percent <= 0.15)
		color = RGBColor(255, 0, 0);
	else if (percent <= 0.5)
		color = RGBColor(255, 255, 0);
	else
		color = RGBColor(17,192,17);
	dst.fill_rect
		(frame_beginning_x + 1, frame_beginning_y + 1,
		 energy_width, frame_height - 2,
		 color);

   // Draw information fields about levels
   // first, gather informations
	const uint hppic = get_hp_level_pic();
	const uint attackpic = get_attack_level_pic();
	const uint defensepic = get_defense_level_pic();
	const uint evadepic = get_evade_level_pic();
   int hpw,hph,atw,ath,dew,deh,evw,evh;
   g_gr->get_picture_size(hppic, &hpw, &hph);
   g_gr->get_picture_size(attackpic, &atw, &ath);
   g_gr->get_picture_size(defensepic, &dew, &deh);
   g_gr->get_picture_size(evadepic, &evw, &evh);

	dst.blit(frame_beginning_x, frame_beginning_y - hph - ath, attackpic);
	dst.blit
		(frame_beginning_x + frame_width / 2, frame_beginning_y - evh - deh,
		 defensepic);
	dst.blit(frame_beginning_x, frame_beginning_y - hph, hppic);
	dst.blit
		(frame_beginning_x + frame_width / 2, frame_beginning_y - evh, evadepic);

	draw_inner(game, dst, drawpos);
}

/**
 *
 *
 */
void Soldier::start_animation (Editor_Game_Base* gg, std::string name, uint time)
{
   if (gg->is_game())
   {
      start_task_idle ((Game*)gg, get_descr()->get_rand_anim(name), time);
   }
}

/**
===============
Soldier::start_task_gowarehouse

Get the soldier to move to the nearest warehouse.
The soldier is added to the list of usable wares, so he may be reassigned to
a new task immediately.
===============
*/
void Soldier::start_task_gowarehouse(Game* g)
{
	assert(!m_supply);

	push_task(g, &taskGowarehouse);

	m_supply = (IdleWorkerSupply*) new IdleSoldierSupply(this);
}

/**
 * LaunchAttack task
 *
 *
 */

Bob::Task Soldier::taskLaunchAttack = {
   "launchattack",

   (Bob::Ptr)&Soldier::launchattack_update,
   (Bob::Ptr)&Soldier::launchattack_signal,
   0,
};

/**
 * soldier::start_task_launchattack
 */
void Soldier::start_task_launchattack(Game* g, Flag* f)
{
   assert (f);

   if (f->get_owner() == get_owner())
      return;


log ("Soldier::start_task_launchattack\n");
   push_task(g, &taskLaunchAttack);

   State* s = get_state();

   // First step (we must exit from building), and set what is the target
   s->ivar1 = 1;
   s->ivar2 = 0;     // Not requested to attack (used by signal 'combat')
   s->objvar1 = get_location(g);   // objvar1 is the owner flag (where is attached the FRIEND military site)
   s->coords = f->get_position(); // Destination
}


/**
 * soldier::launchattack_update
 */
void Soldier::launchattack_update (Game* g, State* state)
{
   std::string signal = get_signal();

molog("[launchattack]: Task Updated %d\n", __LINE__);

/*   if (signal == "combat" || state->ivar2 == 1)
   {
      molog("[launchattack] Combat requested for '%s'\n", signal.c_str());
      set_signal("");
      state->ivar2 = 1;
      molog("[launchattack] Combat not found, continue with launchattack\n");
      state->ivar2 = 0;
   }
   if (signal == "fail")
   {
      molog("[launchattack]: Caught signal '%s'\n", signal.c_str());
      set_signal("");
   }
   else if (signal.size())
   {
      molog("[launchattack]: Interrupted by signal '%s'\n", signal.c_str());
      pop_task(g);
      return;
   }*/

   // See if it's at building and drop of it
   if (state->ivar1 == 1)
   {
      BaseImmovable* position = g->get_map()->get_immovable(get_position());

      if (position && position->get_type() == BUILDING)
      {
         // We are in our building, try to exit
         if (start_task_waitforcapacity(g, (Flag*)get_location(g)->get_base_flag()))
            return;
         state->ivar1 = 2;
         start_task_leavebuilding (g, 1);
         return;
      }
   }

   // Ensures that the owner of this is the base flag of the militarysite that launchs the attack
   Map* map = g->get_map();
   PlayerImmovable* location = get_location(g);
   Flag* owner;

   assert(location);
   assert(location->get_type() == FLAG);

   owner = (Flag*)location;

   if (state->ivar1 > 0)
   {
      Flag* f_target;
      Coords c_target;

      location = (PlayerImmovable*) map->get_immovable(state->coords);

      assert (location);
      assert (location->get_type() == FLAG);

      f_target = (Flag*) location;
      c_target = f_target->get_position();

         // Time to return home (in future, time to enter enemu house)
      if (get_position() == c_target)
      {
         Building* bs = f_target->get_building();
            // If there are enemy soldiers, time to kill they !!!
         if (start_task_waitforassault(g, bs))
            return;

            // Now, this soldier will enter to the house!
         if (bs->get_owner() == get_owner())
         {
            state->ivar1 = 0;
            schedule_act (g, 10);
            return;
         }
         molog ("Hey\n");
         bs->conquered_by (get_owner());
         molog ("Hey\n");
         state->ivar1 = 0;       // This makes return to home flag
         schedule_act(g, 50);    // Waits a little before trying to enter to the house
         return;
      }

      // Move towards enemy flag
      if (!start_task_movepath(g, c_target, 0, get_descr()->get_right_walk_anims(does_carry_ware())))
      {
         molog("[launchattack]: Couldn't find path to enemy flag!\n");
         set_signal("fail");
         mark(false);   // Now can be healed
         pop_task(g);
         return;
      }
      else
         return;
   }

   // Return to friend owner flag
   if (get_position() == owner->get_position())
   {
      molog("[launchattack]: We are on home!\n");
      mark(false);   // Now can be healed
      pop_task(g);
      return;
   }

   molog ("[launchattack]: Return home\n");
   if (!start_task_movepath(g, owner->get_position(), 0, get_descr()->get_right_walk_anims(does_carry_ware())))
   {
      molog("[launchattack]: Couldn't find path home\n");
      set_signal("fail");
      mark(false);   // Now can be healed
      pop_task(g);
      return;
   }
}

void Soldier::launchattack_signal (Game* g, State* state)
{
   std::string signal = get_signal ();

   molog ("[LaunchAttack] Interrupted by signal '%s'\n", signal.c_str());
   set_signal("");
   mark(false);   // Now can be healed
   state->ivar1 = 0;
   launchattack_update(g, state);
   //pop_task(g);
}

/**
 * WaitForAssault task
 *
 *
 */

Bob::Task Soldier::taskWaitForAssault = {
   "waitforassault",

   (Bob::Ptr)&Soldier::waitforassault_update,
   (Bob::Ptr)&Soldier::waitforassault_signal,
   0,
};

/**
 * soldier::start_task_waitforassault (building)
 *
 * Returns true if is needed to wait.
 */
bool Soldier::start_task_waitforassault (Game* g, Building* b)
{
   assert (g);
   if (!b)
      return false;
   assert (b);

   if (!b->has_soldiers())
      return false;

   push_task(g, &taskWaitForAssault);

   State* s = get_state();

   s->objvar1 = b;
  return true;
}

void Soldier::waitforassault_update (Game* g, State* state)
{
   Map_Object* imm = state->objvar1.get(g);

   assert (imm);
   assert (imm->get_type() == BUILDING);

   Building* b = (Building*) imm;

   if (!b->has_soldiers())
   {
      MilitarySite* ms = (MilitarySite*) b;
      molog("[waitforassault]: House %i (%p) empty of soldiers.\n", ms->get_serial(), ms);
      if (ms->get_owner() == get_owner())
      {
         pop_task(g);
         return;
      }
      ms->conquered_by (get_owner());
      molog("[waitforassault]: House %i (%p) empty of soldiers.\n", ms->get_serial(), ms);
      pop_task(g);
      return;
   }
   else
   {
      switch (b->get_building_type())
      {
         case Building::MILITARYSITE:
            {
               MilitarySite* ms = (MilitarySite*) b;
                  // This should launch a soldier to defend the house !
               skip_act(g);
               ms->defend(g, this);
               break;
            }
         case Building::WAREHOUSE:
            {
               Warehouse* wh = (Warehouse*) b;
                  // This should launch a soldier to defend the house !
               skip_act(g);
               wh->defend(g, this);
               break;
            }
         default:
            molog("[waitforassault] Nothing to do with this building!!\n");
            pop_task (g);
            send_signal(g, "fail");
            break;;
      }
      return;
   }

   schedule_act(g, 50);
   return;
}

void Soldier::waitforassault_signal (Game* g, State* state)
{
   std::string signal = get_signal();

   if (signal == "end_combat")
   {
      molog("[WaitForAssault] Caught signal '%s' (Waking up!)\n", signal.c_str());
      set_signal("");
      waitforassault_update(g, state);
      return;
   }
   else if (signal == "die")
   {
      molog ("[WaitForAssault] Caught signal '%s' (Death!)\n", signal.c_str());
      schedule_destroy(g);
      pop_task(g);
      return;
   }
   else  if (signal.size() > 0)
   {
      molog ("[WaitForAssault] : Interrupted by signal '%s'\n", signal.c_str());
      pop_task(g);
   }
}

/**
 * DefendBuilding task
 *
 *
 */

Bob::Task Soldier::taskDefendBuilding = {
   "defendbuilding",

   (Bob::Ptr)&Soldier::defendbuilding_update,
   (Bob::Ptr)&Soldier::defendbuilding_signal,
   0,
};

void Soldier::start_task_defendbuilding (Game* g, Building* b, Bob* enemy)
{
   molog ("Soldier: start_task_defendbuilding\n");
   assert (b);
   assert (enemy);
   assert (((Building*) get_location(g)) == b);

   push_task(g, &taskDefendBuilding);
   mark(true);             // This is for prevent to heal soldiers out of the building
   State* s = get_state();
   s->ivar1 = 1;
   s->ivar2 = b->get_serial();
   s->objvar1 =  enemy;

}

void Soldier::defendbuilding_update (Game* g, State* state)
{
   if (get_current_hitpoints() < 1)
   {
      molog ("[DefendBuilding] : Ohh! I'm deading!!\n");
      set_location(0);
      schedule_destroy (g);
      pop_task(g);
      schedule_act(g, 100);
      return;
   }

   if (state->ivar1 == 4)
   {
      pop_task(g);   // We finally finish this task
      start_task_idle(g, 0, -1); // Bind the soldier to the MS, and hide him of the map
      mark(false);   // Now the soldier can be healed
      return;
   }
      // 1-. Exit of this MS
   if (state->ivar1 == 1)
   {
      BaseImmovable* position = g->get_map()->get_immovable(get_position());

      if (position && position->get_type() == BUILDING)
      {
         // We are in our building, try to exit
         if (start_task_waitforcapacity(g, (Flag*)get_location(g)->get_base_flag()))
            return;
         state->ivar1 = 2;
         start_task_leavebuilding (g, 0);
         return;
      }
   }

   // 2-. Create a BattleField with this soldier and attach emeny soldier
   if (state->ivar1 == 2)
   {
      PlayerImmovable* imm = (PlayerImmovable*)state->objvar1.get(g);
      Soldier* s;

      assert (imm);
      s = (Soldier*) imm;

         // Starts real combat!!
      Battle* battle = g->create_battle();
      battle->soldiers (this, s);
      skip_act(g);
      return;
   }

     // Here at least the soldier has ended of fighting (with this, the soldier will be healed)
   mark(false);

   if (state->ivar1 == 3)
   {
      // Now return home building !

      // Get the building
      Building* build = (Building*)g->get_objects()->get_object(state->ivar2);
      if (build)
      {
         molog ("[DefendBuilding] Returning home (%d)\n", state->ivar2);
         state->ivar1 = 4; // Next time  Idle !
         set_location(build);
         start_task_return(g, false);
         return;
      }
      else
      {
         molog ("[DefendBuilding] Home (%d) doesn't exists!\n", state->ivar2);
         pop_task(g);
         return;
      }

   }

   molog("[DefendBuilding] End of function reached\n");
   pop_task(g);
}

void Soldier::defendbuilding_signal (Game* g, State* state)
{
   std::string signal = get_signal();
   if (signal == "end_combat")
   {
      molog("[DefendBuilding] : Caught signal '%s'\n", signal.c_str());
      set_signal("");
      state->ivar1 = 3;
      defendbuilding_update(g, state);
   }
   else if (signal == "die")
   {
      molog("[DefendBuilding] : Caught signal '%s'\n", signal.c_str());

        // Better to add here a start_task_die (usefull too for use at fugitive task)
      state->ivar1 = 5;
      schedule_destroy(g);
      schedule_act(g, 5000);
      return;
   }
   else
   {
      molog("[DefendBuilding] Interrupted by signal '%s'\n");
      pop_task(g);
      mark(false);
   }
}


void Soldier::log_general_info(Editor_Game_Base* egbase)
{
   Worker::log_general_info(egbase);
   molog("[Soldier]\n");
   molog("Levels: %d/%d/%d/%d\n", m_hp_level, m_attack_level, m_defense_level, m_evade_level);
   molog ("HitPoints: %d/%d\n", m_hp_current, m_hp_max);
   molog ("Attack :  %d-%d\n", m_min_attack, m_max_attack);
   molog ("Defense : %d%%\n", m_defense);
   molog ("Evade:    %d%%\n", m_evade);
}
