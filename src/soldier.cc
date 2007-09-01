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

#include "soldier.h"

#include "attack_controller.h"
#include "battle.h"
#include "editor_game_base.h"
#include "error.h"
#include "game.h"
#include "graphic.h"
#include "helper.h"
#include "militarysite.h"
#include "player.h"
#include "profile.h"
#include "rendertarget.h"
#include "rgbcolor.h"
#include "transport.h"
#include "tribe.h"
#include "warehouse.h"
#include "wexception.h"

#include <stdio.h>


struct IdleSoldierSupply : public Supply {
	IdleSoldierSupply(Soldier * const);
	~IdleSoldierSupply();

	void set_economy(Economy* e);

	virtual PlayerImmovable* get_position(Game* g);
	virtual int get_amount(const int ware) const;
	virtual bool is_active() const throw ();

	virtual WareInstance* launch_item(Game* g, int ware) __attribute__ ((noreturn));
	virtual Worker* launch_worker(Game* g, int ware);

	virtual Soldier* launch_soldier(Game* g, int ware, Requeriments* req);
	virtual int get_passing_requeriments(Game* g, int ware, Requeriments* r);
	virtual void mark_as_used (Game* g, int ware, Requeriments* r);
private:
	Soldier * m_soldier;
	Economy * m_economy;
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
		m_economy->remove_soldier_supply(m_soldier->get_owner()->tribe().get_worker_index(m_soldier->name().c_str()), this);

	m_economy = e;

	if (m_economy)
		m_economy->add_soldier_supply(m_soldier->get_owner()->tribe().get_worker_index(m_soldier->name().c_str()), this);
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
int IdleSoldierSupply::get_amount(const int ware) const {
	if (ware == m_soldier->get_owner()->tribe().get_worker_index(m_soldier->name().c_str()))
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
bool IdleSoldierSupply::is_active() const throw () {return true;}


/*
===============
IdleSoldierSupply::launch_item
===============
*/
WareInstance* IdleSoldierSupply::launch_item(Game *, int)
{throw wexception("IdleSoldierSupply::launch_item() makes no sense.");}


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
Soldier * IdleSoldierSupply::launch_soldier
(Game *, int ware, Requeriments * req)
{
	assert(ware == m_soldier->get_owner()->tribe().get_worker_index(m_soldier->name().c_str()));

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
void IdleSoldierSupply::mark_as_used(Game *, int ware, Requeriments * req) {
	assert(ware == m_soldier->get_owner()->tribe().get_worker_index(m_soldier->name().c_str()));

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
int IdleSoldierSupply::get_passing_requeriments
(Game *, int ware, Requeriments * req)
{
	assert(ware == m_soldier->get_owner()->tribe().get_worker_index(m_soldier->name().c_str()));

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
Soldier_Descr::Soldier_Descr
(const Tribe_Descr & tribe_descr, const std::string & soldier_name)
: Worker_Descr(tribe_descr, soldier_name)
{
	add_attribute(Map_Object::SOLDIER);
}

Soldier_Descr::~Soldier_Descr()
{
   m_hp_pics_fn.resize(0);
   m_attack_pics_fn.resize(0);
   m_defense_pics_fn.resize(0);
   m_evade_pics_fn.resize(0);
}

void Soldier_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Worker_Descr::parse(directory, prof, encdata);
	Section* sglobal=prof->get_section("global");

	// Parse hitpoints
	std::string hp=sglobal->get_safe_string("hp");
	std::vector<std::string> list;
	split_string(hp, list, "-");
	if (list.size() != 2)
		throw wexception
			("Parse error in hp string: \"%s\" (must be \"min-max\")", hp.c_str());
	uint i=0;
	for (i=0; i<list.size(); i++)
		remove_spaces(&list[i]);
	char* endp;
	m_min_hp= strtol(list[0].c_str(), &endp, 0);
	if (endp and *endp)
		throw wexception
			("Parse error in hp string: %s is a bad value", list[0].c_str());
	if (0 == m_min_hp)
		throw wexception
			("Parse error in hp string: \"%s\" is not positive", list[0].c_str());
	m_max_hp = strtol(list[1].c_str(), &endp, 0);
	if (endp and *endp)
		throw wexception
			("Parse error in hp string: %s is a bad value", list[1].c_str());
	if (m_max_hp < m_min_hp)
		throw wexception
			("Parse error in hp string: \"%s\" < \"%s\"",
			 list[1].c_str(), list[0].c_str());

	// Parse attack
	std::string attack=sglobal->get_safe_string("attack");
	list.resize(0);
	split_string(attack, list, "-");
	if (list.size() != 2)
		throw wexception
			("Parse error in attack string: \"%s\" (must be \"min-max\")",
			 attack.c_str());
	for (i=0; i<list.size(); i++)
		remove_spaces(&list[i]);
	m_min_attack= strtol(list[0].c_str(), &endp, 0);
	if (endp and *endp)
		throw wexception
			("Parse error in attack string: %s is a bad value", list[0].c_str());
	m_max_attack = strtol(list[1].c_str(), &endp, 0);
	if (endp and *endp)
		throw wexception
			("Parse error in attack string: %s is a bad value", list[1].c_str());

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
	for (i=0; i<=m_max_hp_level; i++) {
		sprintf(buffer, "hp_level_%i_pic", i);
		m_hp_pics_fn[i]=dir;
		m_hp_pics_fn[i]+=sglobal->get_safe_string(buffer);
	}
	for (i=0; i<=m_max_attack_level; i++) {
		sprintf(buffer, "attack_level_%i_pic", i);
		m_attack_pics_fn[i]=dir;
		m_attack_pics_fn[i]+=sglobal->get_safe_string(buffer);
	}
	for (i=0; i<=m_max_defense_level; i++) {
		sprintf(buffer, "defense_level_%i_pic", i);
		m_defense_pics_fn[i]=dir;
		m_defense_pics_fn[i]+=sglobal->get_safe_string(buffer);
	}
	for (i=0; i<=m_max_evade_level; i++) {
		sprintf(buffer, "evade_level_%i_pic", i);
		m_evade_pics_fn[i]=dir;
		m_evade_pics_fn[i]+=sglobal->get_safe_string(buffer);
	}
}

/**
 * Load the graphics
 */
void Soldier_Descr::load_graphics() {
	m_hp_pics.resize(m_max_hp_level+1);
	m_attack_pics.resize(m_max_attack_level+1);
	m_defense_pics.resize(m_max_defense_level+1);
	m_evade_pics.resize(m_max_evade_level+1);
	uint i;
	for (i=0; i<=m_max_hp_level; i++) {
		m_hp_pics[i]=g_gr->get_picture(PicMod_Game,  m_hp_pics_fn[i].c_str());
	}
	for (i=0; i<=m_max_attack_level; i++) {
		m_attack_pics[i]=g_gr->get_picture(PicMod_Game,  m_attack_pics_fn[i].c_str());
	}
	for (i=0; i<=m_max_defense_level; i++) {
		m_defense_pics[i]=g_gr->get_picture(PicMod_Game,  m_defense_pics_fn[i].c_str());
	}
	for (i=0; i<=m_max_evade_level; i++) {
		m_evade_pics[i]=g_gr->get_picture(PicMod_Game,  m_evade_pics_fn[i].c_str());
	}
	Worker_Descr::load_graphics();
}


/**
 * Get random animation of specified type
 */
uint Soldier_Descr::get_rand_anim(const char * const animation_name) const {
	// Todo: This is thought to get a random animation like attack_1 attack_2 attack_3 ...
	// Randimly trhought this method. By now only gets attack, but isn't very difficult
	// to remake allowing the attack_1 and so.
	return get_animation(animation_name);
}

/**
 * Create a new soldier
 */
Bob * Soldier_Descr::create_object() const {return new Soldier(*this);}

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
Soldier::Soldier(const Soldier_Descr & soldier_descr) : Worker(soldier_descr) {}
   // all done through init

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
	m_min_attack=descr().get_min_attack();
	m_max_attack=descr().get_max_attack();
	m_defense=descr().get_defense();
	m_evade=descr().get_evade();
	if (Game * const game = dynamic_cast<Game *>(gg)) {
		const uint min_hp = descr().get_min_hp();
		assert(min_hp);
		assert(min_hp <= descr().get_max_hp());
		m_hp_max = min_hp + game->logic_rand() % (descr().get_max_hp() - min_hp);
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
	assert(hp>=m_hp_level && hp<=descr().get_max_hp_level());

	while (m_hp_level<hp) {
		++m_hp_level;
		m_hp_max+=descr().get_hp_incr_per_level();
		m_hp_current+=descr().get_hp_incr_per_level();
	}
}
void Soldier::set_attack_level(const uint attack) {
	assert(attack>=m_attack_level && attack<=descr().get_max_attack_level());

	while (m_attack_level<attack) {
		++m_attack_level;
		m_min_attack+=descr().get_attack_incr_per_level();
		m_max_attack+=descr().get_attack_incr_per_level();
	}
}
void Soldier::set_defense_level(const uint defense) {
	assert(defense>=m_defense_level && defense<=descr().get_max_defense_level());

	while (m_defense_level<defense) {
		++m_defense_level;
		m_defense+=descr().get_defense_incr_per_level();
	}
}
void Soldier::set_evade_level(const uint evade) {
	assert(evade>=m_evade_level && evade<=descr().get_max_evade_level());

	while (m_evade_level<evade) {
		++m_evade_level;
		m_evade+=descr().get_evade_incr_per_level();
	}
}

uint Soldier::get_level(const tAttribute at)
{
	switch (at) {
	case atrHP:      return m_hp_level;
	case atrAttack:  return m_attack_level;
	case atrDefense: return m_defense_level;
	case atrEvade:   return m_evade_level;
	case atrTotal:
		return m_hp_level + m_attack_level + m_defense_level + m_evade_level;
	}
	throw wexception ("Soldier::get_level attribute not identified.");
}

// Unsignedness ensures that we can only heal, don't hurt throught this method.
void Soldier::heal (const uint hp) {
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
	if (const uint anim = get_current_anim()) {

		const Point drawpos = calc_drawpos(game, pos);

		uint w, h;
		g_gr->get_animation_size(anim, game.get_gametime() - get_animstart(), w, h);

	// Draw energy bar
	// first: draw white sourrounding
		Rect r(Point(drawpos.x - w, drawpos.y - h - 7), w * 2, 5);
		dst.draw_rect(r, HP_FRAMECOLOR);
		// Draw the actual bar
		assert(m_hp_max);
		const float fraction = static_cast<float>(m_hp_current) / m_hp_max;
		//FIXME:
		//Draw bar in playercolor, should be removed when soldier is correctly painted
		RGBColor color(
				get_owner()->get_playercolor()->r(),
				get_owner()->get_playercolor()->g(),
				get_owner()->get_playercolor()->b()
		);
		/*if (fraction <= 0.15)
			color = RGBColor(255, 0, 0);
		else if (fraction <= 0.5)
			color = RGBColor(255, 255, 0);
		else
			color = RGBColor(17, 192, 17);*/
		assert(2 <= r.w);
		assert(2 <= r.h);
		dst.fill_rect
				(Rect
				(r + Point(1, 1), static_cast<int>(fraction * (r.w - 2)), r.h - 2),
			color);

	// Draw information fields about levels
	// first, gather informations
		const uint hppic = get_hp_level_pic();
		const uint attackpic = get_attack_level_pic();
		const uint defensepic = get_defense_level_pic();
		const uint evadepic = get_evade_level_pic();
		uint hpw, hph, atw, ath, dew, deh, evw, evh;
		g_gr->get_picture_size(hppic,      hpw, hph);
		g_gr->get_picture_size(attackpic,  atw, ath);
		g_gr->get_picture_size(defensepic, dew, deh);
		g_gr->get_picture_size(evadepic,   evw, evh);

		dst.blit(r + Point      (0, -(hph + ath)), attackpic);
		dst.blit(r + Point(r.w / 2, -(evh + deh)), defensepic);
		dst.blit(r + Point      (0, -        hph), hppic);
		dst.blit(r + Point(r.w / 2, -        evh), evadepic);

		draw_inner(game, dst, drawpos);
	}
}

/**
 *
 *
 */
void Soldier::start_animation
(Editor_Game_Base* gg, const char * const animname, const uint time)
{
	if (Game * const game = dynamic_cast<Game *>(gg))
		start_task_idle (game, descr().get_rand_anim(animname), time);
}

/**
===============
Soldier::start_task_gowarehouse

Get the soldier to move to the nearest warehouse.
The soldier is added to the list of usable wares, so he may be reassigned to
a new task immediately.
===============
*/
void Soldier::start_task_gowarehouse() {
	assert(!m_supply);

	push_task(taskGowarehouse);

	m_supply = (IdleWorkerSupply*) new IdleSoldierSupply(this);
}

Bob::Task Soldier::taskMoveToBattle = {
   "moveToBattle",
   (Bob::Ptr)&Soldier::moveToBattleUpdate,
   (Bob::Ptr)&Soldier::moveToBattleSignal,
   0,
};

void Soldier::startTaskMoveToBattle(Game * g, Flag *, Coords coords) {
	log ("Soldier::startTaskMoveToBattle\n");
	push_task(taskMoveToBattle);

	State* s = get_state();

	// First step (we must exit from building), and set what is the target
	s->ivar1 = 1;
	s->ivar2 = 0;     // Not requested to attack (used by signal 'combat')
	s->objvar1 = get_location(g);   // objvar1 is the owner flag (where is attached the FRIEND military site)
	s->coords = coords; // Destination
}

void Soldier::moveToBattleUpdate(Game* g, State* state) {
	// See if soldier is at building and drop of it
	if (state->ivar1 == 1) {
		BaseImmovable* position = g->get_map()->get_immovable(get_position());

		if (position && position->get_type() == BUILDING) {
			state->ivar1 = 2;
			start_task_leavebuilding (g, 1);
			return;
		}
	}
	else {
		if (get_position() == state->coords) {
			if (state->ivar1 != 3)
				m_attack_ctrl->moveToReached(this);
			state->ivar1 = 3;
			start_task_idle(g, descr().get_animation("idle"), 1000);
			return;
		}
		if (!start_task_movepath(g, state->coords, 0, descr().get_right_walk_anims(does_carry_ware()))) {
			molog("[moveToBattleUpdate]: Couldn't find path to flag!\n");
			set_signal("fail");
			mark(false);
			pop_task();
			return;
		}
	}
}

void Soldier::moveToBattleSignal(Game * g, State *) {
	std::string signal = get_signal();
	set_signal("");

	log("moveToBattleSignal got signal: %s", signal.c_str());

	if (signal == "won_battle") {
		m_attack_ctrl->soldierWon(this);
		return;
	}
	else if (signal == "die") {
		m_attack_ctrl->soldierDied(this);
		return;
	}
	else if (signal == "return_home") {
		pop_task();
		startTaskMoveHome(g);
		return;
	}
}

Bob::Task Soldier::taskMoveHome = {
   "moveHome",

   (Bob::Ptr)&Soldier::moveHomeUpdate,
   (Bob::Ptr)&Soldier::moveHomeSignal,
   0,
};

void Soldier::startTaskMoveHome(Game* g) {
	log ("Soldier::startTaskMoveHome\n");
	push_task(taskMoveHome);

	State* s = get_state();
	s->ivar1 = 1;
	s->ivar2 = 0;
	s->objvar1 = get_location(g);
}

void Soldier::moveHomeUpdate(Game* g, State* state) {
	// Move home
	if (state->ivar1 == 1) {
		state->ivar1 = 2;
		start_task_return(g, false);
		return;
	}
	else {
		start_task_idle(g, 0, -1); // bind the worker into this house, hide him on the map
		mark (false);              // can be healed now
	}
}

void Soldier::moveHomeSignal(Game *, State *) {
	std::string signal = get_signal();
	set_signal("");

	log("moveToSignal got signal, don't know what to do with it.: %s", signal.c_str());
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
