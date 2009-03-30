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

#include "soldier.h"

#include "attackable.h"
#include "battle.h"
#include "checkstep.h"
#include "editor_game_base.h"
#include "findimmovable.h"
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

#include "upcast.h"

#include <cstdio>

namespace Widelands {


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
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const & _tribe, EncodeData const * const encdata)
	: Worker_Descr(_name, _descname, directory, prof, global_s, _tribe, encdata)
{
	add_attribute(Map_Object::SOLDIER);

	{ //  hitpoints
		const char * const hp = global_s.get_safe_string("hp");
		std::vector<std::string> list(split_string(hp, "-"));
		if (list.size() != 2)
			throw wexception
				("Parse error in hp string: \"%s\" (must be \"min-max\")", hp);
		container_iterate(std::vector<std::string>, list, i)
			remove_spaces(*i.current);
		char * endp;
		m_min_hp = strtol(list[0].c_str(), &endp, 0);
		if (*endp)
			throw wexception
				("Parse error in hp string: %s is a bad value", list[0].c_str());
		if (0 == m_min_hp)
			throw wexception
				("Parse error in hp string: \"%s\" is not positive", list[0].c_str());
		m_max_hp = strtol(list[1].c_str(), &endp, 0);
		if (*endp)
			throw wexception
				("Parse error in hp string: %s is a bad value", list[1].c_str());
		if (m_max_hp < m_min_hp)
			throw wexception
				("Parse error in hp string: \"%s\" < \"%s\"",
				 list[1].c_str(), list[0].c_str());
	}

	{ //  parse attack
		const char * const attack = global_s.get_safe_string("attack");
		std::vector<std::string> list(split_string(attack, "-"));
		if (list.size() != 2)
			throw wexception
				("Parse error in attack string: \"%s\" (must be \"min-max\")",
				 attack);
		container_iterate(std::vector<std::string>, list, i)
			remove_spaces(*i.current);
		char * endp;
		m_min_attack = strtol(list[0].c_str(), &endp, 0);
		if (*endp)
			throw wexception
				("Parse error in attack string: %s is a bad value",
				 list[0].c_str());
		m_max_attack = strtol(list[1].c_str(), &endp, 0);
		if (*endp)
			throw wexception
				("Parse error in attack string: %s is a bad value",
				 list[1].c_str());
	}

	// Parse defend
	m_defense           = global_s.get_safe_int("defense");

	// Parse evade
	m_evade             = global_s.get_safe_int("evade");

	// Parse increases per level
	m_hp_incr           = global_s.get_safe_int("hp_incr_per_level");
	m_attack_incr       = global_s.get_safe_int("attack_incr_per_level");
	m_defense_incr      = global_s.get_safe_int("defense_incr_per_level");
	m_evade_incr        = global_s.get_safe_int("evade_incr_per_level");

	// Parse max levels
	m_max_hp_level      = global_s.get_safe_int("max_hp_level");
	m_max_attack_level  = global_s.get_safe_int("max_attack_level");
	m_max_defense_level = global_s.get_safe_int("max_defense_level");
	m_max_evade_level   = global_s.get_safe_int("max_evade_level");

	// Load the filenames
	m_hp_pics_fn     .resize(m_max_hp_level      + 1);
	m_attack_pics_fn .resize(m_max_attack_level  + 1);
	m_defense_pics_fn.resize(m_max_defense_level + 1);
	m_evade_pics_fn  .resize(m_max_evade_level   + 1);
	char buffer[256];
	std::string dir = directory;
	dir += "/";
	for (uint32_t i = 0; i <= m_max_hp_level;      ++i) {
		snprintf(buffer, sizeof(buffer), "hp_level_%u_pic",      i);
		m_hp_pics_fn[i] = dir;
		m_hp_pics_fn[i] += global_s.get_safe_string(buffer);
	}
	for (uint32_t i = 0; i <= m_max_attack_level;  ++i) {
		snprintf(buffer, sizeof(buffer), "attack_level_%u_pic",  i);
		m_attack_pics_fn[i] = dir;
		m_attack_pics_fn[i] += global_s.get_safe_string(buffer);
	}
	for (uint32_t i = 0; i <= m_max_defense_level; ++i) {
		snprintf(buffer, sizeof(buffer), "defense_level_%u_pic", i);
		m_defense_pics_fn[i] = dir;
		m_defense_pics_fn[i] += global_s.get_safe_string(buffer);
	}
	for (uint32_t i = 0; i <= m_max_evade_level;   ++i) {
		snprintf(buffer, sizeof(buffer), "evade_level_%i_pic",   i);
		m_evade_pics_fn[i] = dir;
		m_evade_pics_fn[i] += global_s.get_safe_string(buffer);
	}
}

/**
 * Load the graphics
 */
void Soldier_Descr::load_graphics() {
	m_hp_pics     .resize(m_max_hp_level      + 1);
	m_attack_pics .resize(m_max_attack_level  + 1);
	m_defense_pics.resize(m_max_defense_level + 1);
	m_evade_pics  .resize(m_max_evade_level   + 1);
	for (uint32_t i = 0; i <= m_max_hp_level;      ++i)
		m_hp_pics[i] = g_gr->get_picture(PicMod_Game,  m_hp_pics_fn[i].c_str());
	for (uint32_t i = 0; i <= m_max_attack_level;  ++i)
		m_attack_pics[i] =
			g_gr->get_picture(PicMod_Game,  m_attack_pics_fn[i].c_str());
	for (uint32_t i = 0; i <= m_max_defense_level; ++i)
		m_defense_pics[i] =
			g_gr->get_picture(PicMod_Game,  m_defense_pics_fn[i].c_str());
	for (uint32_t i = 0; i <= m_max_evade_level;   ++i)
		m_evade_pics[i] =
			g_gr->get_picture(PicMod_Game,  m_evade_pics_fn[i].c_str());
	Worker_Descr::load_graphics();
}


/**
 * Get random animation of specified type
 */
uint32_t Soldier_Descr::get_rand_anim(const char * const animation_name) const {
	//  TODO This is thought to get a random animation like attack_1 attack_2
	//  TODO attack_3 ...
	//  TODO Randomly through this method. By now only gets attack, but is not
	//  TODO very difficult to remake allowing the attack_1 and so.
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

/// all done through init
Soldier::Soldier(const Soldier_Descr & soldier_descr) : Worker(soldier_descr)
{
	m_battle = 0;
}


void Soldier::init(Editor_Game_Base* gg)
{
	m_hp_level      = 0;
	m_attack_level  = 0;
	m_defense_level = 0;
	m_evade_level   = 0;

	m_hp_max        = 0;
	m_min_attack    = descr().get_min_attack();
	m_max_attack    = descr().get_max_attack();
	m_defense       = descr().get_defense   ();
	m_evade         = descr().get_evade     ();
	if (upcast(Game, game, gg)) {
		const uint32_t min_hp = descr().get_min_hp();
		assert(min_hp);
		assert(min_hp <= descr().get_max_hp());
		m_hp_max = min_hp + game->logic_rand() % (descr().get_max_hp() - min_hp);
	}
	m_hp_current    = m_hp_max;

	Worker::init(gg);
}

void Soldier::cleanup(Editor_Game_Base* gg)
{
	Worker::cleanup(gg);
}

/*
 * Set this soldiers level. Automatically sets the new values
 */
void Soldier::set_level
	(uint32_t const hp,
	 uint32_t const attack,
	 uint32_t const defense,
	 uint32_t const evade)
{
	set_hp_level(hp);
	set_attack_level(attack);
	set_defense_level(defense);
	set_evade_level(evade);
}
void Soldier::set_hp_level(const uint32_t hp) {
	assert(m_hp_level <= hp);
	assert              (hp <= descr().get_max_hp_level());

	while (m_hp_level < hp) {
		++m_hp_level;
		m_hp_max     += descr().get_hp_incr_per_level();
		m_hp_current += descr().get_hp_incr_per_level();
	}
}
void Soldier::set_attack_level(const uint32_t attack) {
	assert(m_attack_level <= attack);
	assert                  (attack <= descr().get_max_attack_level());

	while (m_attack_level < attack) {
		++m_attack_level;
		m_min_attack += descr().get_attack_incr_per_level();
		m_max_attack += descr().get_attack_incr_per_level();
	}
}
void Soldier::set_defense_level(const uint32_t defense) {
	assert(m_defense_level <= defense);
	assert                   (defense <= descr().get_max_defense_level());

	while (m_defense_level < defense) {
		++m_defense_level;
		m_defense += descr().get_defense_incr_per_level();
	}
}
void Soldier::set_evade_level(const uint32_t evade) {
	assert(m_evade_level <= evade);
	assert                 (evade <= descr().get_max_evade_level());

	while (m_evade_level < evade) {
		++m_evade_level;
		m_evade += descr().get_evade_incr_per_level();
	}
}

uint32_t Soldier::get_level(tAttribute const at) const {
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

bool Soldier::have_tattributes() const
{
	return true;
}

int32_t Soldier::get_tattribute(uint32_t attr) const
{
	switch (attr) {
	case atrHP: return m_hp_level;
	case atrAttack: return m_attack_level;
	case atrDefense: return m_defense_level;
	case atrEvade: return m_evade_level;
	case atrTotal:
		return m_hp_level + m_attack_level + m_defense_level + m_evade_level;
	}

	return Worker::get_tattribute(attr);
}

//  Unsignedness ensures that we can only heal, not hurt through this method.
void Soldier::heal (const uint32_t hp) {
	molog ("healing (%d+)%d/%d\n", hp, m_hp_current, m_hp_max);
	assert(hp);
	assert(m_hp_current <  m_hp_max);
	m_hp_current += std::min(hp, m_hp_max - m_hp_current);
	assert(m_hp_current <= m_hp_max);
}

/**
 * This only subs the specified number of hitpoints, don't do anything more.
 */
void Soldier::damage (const uint32_t value)
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
	(Editor_Game_Base const & game, RenderTarget & dst, Point const pos) const
{
	if (const uint32_t anim = get_current_anim()) {

		const Point drawpos = calc_drawpos(game, pos);

		uint32_t w, h;
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
		RGBColor color
			(get_owner()->get_playercolor()->r(),
			 get_owner()->get_playercolor()->g(),
			 get_owner()->get_playercolor()->b());
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
			 	(r + Point(1, 1),
			 	 static_cast<int32_t>(fraction * (r.w - 2)), r.h - 2),
			 color);

		//  Draw information fields about levels. First, gather information.
		const uint32_t hppic = get_hp_level_pic();
		const uint32_t attackpic = get_attack_level_pic();
		const uint32_t defensepic = get_defense_level_pic();
		const uint32_t evadepic = get_evade_level_pic();
		uint32_t hpw, hph, atw, ath, dew, deh, evw, evh;
		g_gr->get_picture_size(hppic,      hpw, hph);
		g_gr->get_picture_size(attackpic,  atw, ath);
		g_gr->get_picture_size(defensepic, dew, deh);
		g_gr->get_picture_size(evadepic,   evw, evh);

		{
			const uint32_t w_half = r.w >> 1;
			dst.blit(r + Point(w_half - atw, -(hph + ath)), attackpic);
			dst.blit(r + Point(w_half,       -(evh + deh)), defensepic);
			dst.blit(r + Point(w_half - hpw, -hph),         hppic);
			dst.blit(r + Point(w_half,       -evh),         evadepic);
		}

		draw_inner(game, dst, drawpos);
	}
}

/**
 *
 *
 */
void Soldier::start_animation
	(Editor_Game_Base * gg, char const * const animname, uint32_t const time)
{
	if (upcast(Game, game, gg))
		return start_task_idle (game, descr().get_rand_anim(animname), time);
}


/**
 * \return \c true if this soldier is considered to be on the battlefield
 */
bool Soldier::isOnBattlefield()
{
	return get_state(&taskAttack) || get_state(&taskDefense);
}


Battle * Soldier::getBattle()
{
	return m_battle;
}


/**
 * Determine whether this soldier can be challenged by an opponent.
 *
 * Such a challenge might override a battle that the soldier is currently
 * walking towards, to avoid lockups when the combatants cannot reach
 * each other.
 */
bool Soldier::canBeChallenged()
{
	if (!isOnBattlefield())
		return false;
	if (!m_battle)
		return true;
	return !m_battle->locked(dynamic_cast<Game*>(&get_owner()->egbase()));
}

/**
 * Assign the soldier to a battle (may be zero).
 *
 * \note must only be called by the \ref Battle object
 */
void Soldier::setBattle(Game* g, Battle* battle)
{
	if (m_battle != battle) {
		m_battle = battle;
		send_signal(g, "battle");
	}
}


/**
 * Leave our home building and single-mindedly try to attack
 * and conquer the given building.
 *
 * The following variables are used:
 * \li objvar1 the \ref Building we're attacking.
 */
Bob::Task Soldier::taskAttack = {
	"attack",
	static_cast<Bob::Ptr>(&Soldier::attack_update),
	0,
	static_cast<Bob::Ptr>(&Soldier::attack_pop)
};

void Soldier::startTaskAttack(Game* g, Building* building)
{
	assert(dynamic_cast<Attackable*>(building));

	push_task(g, taskAttack);

	State* s = get_state();
	s->objvar1 = building;
}

void Soldier::attack_update(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal.size()) {
		if (signal == "blocked" || signal == "battle" || signal == "wakeup") {
			signal_handled();
		} else if (signal == "fail") {
			signal_handled();
			if (state->objvar1.get(g)) {
				molog("[attack] failed to reach enemy\n");
				state->objvar1 = 0;
			} else {
				molog("[attack] unexpected fail\n");
				return pop_task(g);
			}
		} else {
			molog("[attack] cancelled by unexpected signal '%s'\n", signal.c_str());
			return pop_task(g);
		}
	}

	PlayerImmovable* location = get_location(g);

	BaseImmovable* imm = g->map()[get_position()].get_immovable();
	upcast(Building, enemy, state->objvar1.get(g));
	if (imm == location) {
		if (!enemy) {
			molog("[attack] returned home\n");
			return pop_task(g);
		}

		return start_task_leavebuilding(g, false);
	}

	if (m_battle)
		return startTaskBattle(g);

	if (signal == "blocked")
		// Wait before we try again. Note that this must come *after*
		// we check for a battle
		return start_task_idle(g, get_animation("idle"), 5000);

	if (!location) {
		molog("[attack] our location disappeared during a battle\n");
		return pop_task(g);
	}

	if (!enemy) {
		Flag* baseflag = location->get_base_flag();
		if (imm == baseflag)
			return
				start_task_move
					(g,
					 WALK_NW,
					 &descr().get_right_walk_anims(does_carry_ware()),
					 true);

		if
			(start_task_movepath
			 	(g, baseflag->get_position(), 0,
			 	 descr().get_right_walk_anims(does_carry_ware())))
			return;
		else {
			molog("[attack] failed to return home\n");
			return pop_task(g);
		}
	}

	if (enemy->get_owner() == get_owner()) {
		if (upcast(SoldierControl, ctrl, enemy)) {
			if (ctrl->stationedSoldiers().size() < ctrl->soldierCapacity()) {
				molog("[attack] enemy belongs to us now, move in\n");
				set_location(enemy);
			}
		}

		state->objvar1 = 0;
		return schedule_act(g, 10);
	}

	// At this point, we know that the enemy building still stands,
	// and that we're outside in the plains.
	if (imm != enemy->get_base_flag()) {
		if
			(start_task_movepath
			 	(g, enemy->get_base_flag()->get_position(), 2,
			 	 descr().get_right_walk_anims(does_carry_ware())))
			return;
		else {
			molog("[attack] failed to move towards building flag\n");
			return pop_task(g);
		}
	}

	upcast(Attackable, attackable, enemy);
	assert(attackable);

	molog("[attack] attacking target building\n");
	//  give the enemy soldier some time to act
	schedule_act(g, attackable->attack(*this) ? 1000 : 10);
}

void Soldier::attack_pop(Game* g, State*)
{
	if (m_battle)
		m_battle->cancel(g, this);
}


/**
 * We are defending our home.
 *
 * Variables used:
 * \li ivar1 is \c true when the soldier is supposed to stay on the home flag
 * \li ivar2 used to pause before finally going home
 */
Bob::Task Soldier::taskDefense = {
	"defense",
	static_cast<Bob::Ptr>(&Soldier::defense_update),
	0,
	static_cast<Bob::Ptr>(&Soldier::defense_pop)
};

void Soldier::startTaskDefense(Game* g, bool stayhome)
{
	push_task(g, taskDefense);

	State* state = get_state();
	state->ivar1 = stayhome;
	state->ivar2 = 0;
}

void Soldier::defense_update(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal.size()) {
		if (signal == "blocked" || signal == "battle" || signal == "wakeup") {
			signal_handled();
		} else {
			molog("[defense] cancelled by signal '%s'\n", signal.c_str());
			return pop_task(g);
		}
	}

	PlayerImmovable* location = get_location(g);
	BaseImmovable* position = g->map()[get_position()].get_immovable();
	if (m_battle) {
		if (position == location)
			return start_task_leavebuilding(g, false);

		state->ivar2 = 0;
		return startTaskBattle(g);
	}

	if (!location) {
		molog("[defense] location disappeared during battle\n");
		return pop_task(g);
	}

	if (signal == "blocked")
		// Wait before we try again. Note that this must come *after*
		// we check for a battle
		return start_task_idle(g, get_animation("idle"), 5000);

	if (position == location) {
		molog("[defense] returned home\n");
		return pop_task(g);
	}

	Flag* baseflag = location->get_base_flag();
	if (position == baseflag) {
		if (state->ivar1 && !state->ivar2) {
			state->ivar2 = 1;
			return start_task_idle(g, get_animation("idle"), 250);
		}
		return
			start_task_move
				(g,
				 WALK_NW,
				 &descr().get_right_walk_anims(does_carry_ware()),
				 true);
	}

	molog("[defense] return home\n");
	start_task_movepath
		(g, baseflag->get_position(), 0,
		 descr().get_right_walk_anims(does_carry_ware()));
	return;
}

void Soldier::defense_pop(Game* g, State*)
{
	if (m_battle)
		m_battle->cancel(g, this);
}


/**
 * \return \c true if the defending soldier should not stray from
 * his home flag.
 */
bool Soldier::stayHome()
{
	if (State * const state = get_state(&taskDefense))
		return state->ivar1;
	return false;
}


/**
 * We are out in the open and involved in a challenge/battle.
 * Meet with the other soldier and fight.
 */
Bob::Task Soldier::taskBattle = {
	"battle",
	static_cast<Bob::Ptr>(&Soldier::battle_update),
	0,
	static_cast<Bob::Ptr>(&Soldier::battle_pop)
};

void Soldier::startTaskBattle(Game* g)
{
	assert(m_battle);

	push_task(g, taskBattle);
}

void Soldier::battle_update(Game* g, State*)
{
	std::string signal = get_signal();
	molog
		("[battle] update for player %u's soldier: signal = \"%s\"\n",
		 get_owner()->get_player_number(), signal.c_str());

	if (signal.size()) {
		if (signal == "blocked") {
			signal_handled();
			return start_task_idle(g, get_animation("idle"), 5000);
		} else if (signal == "location" || signal == "battle" || signal == "wakeup") {
			signal_handled();
		} else {
			molog("[battle] interrupted by unexpected signal '%s'\n", signal.c_str());
			return pop_task(g);
		}
	}

	if (!m_battle) {
		molog("[battle] is over\n");
		sendSpaceSignals(g);
		return pop_task(g);
	}

	if (!m_battle->opponent(*this)) {
		molog("[battle] no opponent, starting task idle\n");
		return start_task_idle(g, get_animation("idle"), -1);
	}

	if (stayHome()) {
		if (this == m_battle->first()) {
			molog("[battle] stayHome, so reverse roles\n");
			new Battle(*g, *m_battle->second(), *m_battle->first());
			skip_act(); // we will get a signal via setBattle()
			return;
		}
	} else {
		Soldier & opponent = *m_battle->opponent(*this);

		if (opponent.get_position() != get_position()) {
			Map& map = g->map();
			uint32_t const dist =
				map.calc_distance(get_position(), opponent.get_position());

			if (dist >= 2 || this == m_battle->first()) {
				//  Only make small steps at a time, so we can adjust to the
				//  opponent's change of position.
				Coords dest = opponent.get_position();
				if (upcast(Building, building, map[dest].get_immovable()))
					dest = building->get_base_flag()->get_position();
				start_task_movepath
					(g, dest, 0,
					 descr().get_right_walk_anims(does_carry_ware()),
					 false, (dist+3)/4);
				molog
					("player %u's soldier started task_movepath\n",
					 get_owner()->get_player_number());
				return;
			}
		}
	}

	m_battle->getBattleWork(*g, *this);
}

void Soldier::battle_pop(Game* g, State*)
{
	if (m_battle)
		m_battle->cancel(g, this);
}


struct FindSoldierOnBattlefield : public FindBob {
	bool accept(Bob* bob) const
	{
		if (upcast(Soldier, soldier, bob))
			return soldier->isOnBattlefield();
		return false;
	}
};


/**
 * Override \ref Bob::checkFieldBlocked.
 *
 * As long as we're on the battlefield, check for other soldiers.
 */
bool Soldier::checkFieldBlocked(Game* g, const FCoords& field, bool commit)
{
	if (!isOnBattlefield())
		return false;

	if (upcast(Building, building, get_location(g))) {
		if (field == building->get_position()) {
			if (commit)
				sendSpaceSignals(g);
			return false; // we can always walk home
		}
	}

	std::vector<Bob *> soldiers;
	g->map().find_bobs(Area<FCoords>(field, 0), &soldiers, FindSoldierOnBattlefield());

	if
		(soldiers.size() &&
		 (!m_battle ||
		  std::find(soldiers.begin(), soldiers.end(), m_battle->opponent(*this))
		  ==
		  soldiers.end()))
	{
		if (commit && soldiers.size() == 1) {
			Soldier & soldier = dynamic_cast<Soldier &>(*soldiers[0]);
			if (soldier.get_owner() != get_owner() && soldier.canBeChallenged()) {
				molog("[checkFieldBlocked] attacking a soldier\n");
				new Battle(*g, *this, soldier);
			}
		}
		return true;
	}

	if (commit)
		sendSpaceSignals(g);
	return false;
}


/**
 * Send a "wakeup" signal to all surrounding soldiers that are out in the open,
 * so that they may repeat pathfinding.
 */
void Soldier::sendSpaceSignals(Game* g)
{
	std::vector<Bob *> soldiers;

	g->map().find_bobs(Area<FCoords>(get_position(), 1), &soldiers, FindSoldierOnBattlefield());

	for (uint32_t i = 0; i < soldiers.size(); ++i) {
		if (upcast(Soldier, soldier, soldiers[i])) {
			if (soldier != this)
				soldier->send_signal(g, "wakeup");
		}
	}

	if (get_position().field->get_owned_by() != get_owner()->get_player_number()) {
		std::vector<BaseImmovable*> attackables;
		g->map().find_reachable_immovables_unique
			(Area<FCoords>(get_position(), MaxProtectionRadius),
			 &attackables,
			 CheckStepWalkOn(descr().movecaps(), false),
			 FindImmovableAttackable());

		container_iterate_const(std::vector<BaseImmovable *>, attackables, i)
			if
				(dynamic_cast<PlayerImmovable const &>(**i.current).get_owner()
				 !=
				 get_owner())
				dynamic_cast<Attackable &>(**i.current).aggressor(*this);
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

};
