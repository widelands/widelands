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
#include "economy/flag.h"
#include "editor_game_base.h"
#include "findimmovable.h"
#include "game.h"
#include "game_data_error.h"
#include "gamecontroller.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "message_queue.h"
#include "militarysite.h"
#include "player.h"
#include "profile/profile.h"
#include "graphic/rendertarget.h"
#include "tribe.h"
#include "upcast.h"
#include "warehouse.h"
#include "wexception.h"
#include "wui/mapviewpixelconstants.h"

#include <cstdio>

namespace Widelands {

Soldier_Descr::Soldier_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const & _tribe, EncodeData const * const encdata)
	: Worker_Descr(_name, _descname, directory, prof, global_s, _tribe, encdata)
{
	add_attribute(Map_Object::SOLDIER);

	try { //  hitpoints
		const char * const hp = global_s.get_safe_string("hp");
		std::vector<std::string> list(split_string(hp, "-"));
		if (list.size() != 2)
			throw game_data_error
				(_("expected %s but found \"%s\""), _("\"min-max\""), hp);
		container_iterate(std::vector<std::string>, list, i)
			remove_spaces(*i.current);
		char * endp;
		m_min_hp = strtol(list[0].c_str(), &endp, 0);
		if (*endp or 0 == m_min_hp)
			throw game_data_error
				(_("expected %s but found \"%s\""),
				 _("positive integer"), list[0].c_str());
		m_max_hp = strtol(list[1].c_str(), &endp, 0);
		if (*endp or m_max_hp < m_min_hp)
			throw game_data_error
				(_("expected positive integer >= %u but found \"%s\""),
				 m_min_hp, list[1].c_str());
	} catch (_wexception const & e) {
		throw game_data_error("hp: %s", e.what());
	}

	try { //  parse attack
		const char * const attack = global_s.get_safe_string("attack");
		std::vector<std::string> list(split_string(attack, "-"));
		if (list.size() != 2)
			throw game_data_error
				(_("expected %s but found \"%s\""), _("\"min-max\""), attack);
		container_iterate(std::vector<std::string>, list, i)
			remove_spaces(*i.current);
		char * endp;
		m_min_attack = strtol(list[0].c_str(), &endp, 0);
		if (*endp or 0 == m_min_attack)
			throw game_data_error
				(_("expected %s but found \"%s\""),
				 _("positive integer"), list[0].c_str());
		m_max_attack = strtol(list[1].c_str(), &endp, 0);
		if (*endp or m_max_attack < m_min_attack)
			throw game_data_error
				(_("expected positive integer >= %u but found \"%s\""),
				 m_min_attack, list[1].c_str());
	} catch (_wexception const & e) {
		throw game_data_error("attack: %s", e.what());
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

	{  ///  Battle animations
		/// attack_success_*-> soldier is attacking and hit his opponent
		/// attack_failure_*-> soldier is attacking and miss hit, defender evades
		/// evade_success_* -> soldier is defending and opponent misses
		/// evade_failure_* -> soldier is defending and opponent hits
		/// die_*           -> soldier is dying
		m_attack_success_w_name =
			load_animations_from_string
				(directory, prof, global_s, encdata, "attack_success_w");
		m_attack_success_e_name =
			load_animations_from_string
				(directory, prof, global_s, encdata, "attack_success_e");
		m_attack_failure_w_name =
			load_animations_from_string
				(directory, prof, global_s, encdata, "attack_failure_w");
		m_attack_failure_e_name =
			load_animations_from_string
				(directory, prof, global_s, encdata, "attack_failure_e");
		m_evade_success_w_name =
			load_animations_from_string
				(directory, prof, global_s, encdata, "evade_success_w");
		m_evade_success_e_name =
			load_animations_from_string
				(directory, prof, global_s, encdata, "evade_success_e");
		m_evade_failure_w_name =
			load_animations_from_string
				(directory, prof, global_s, encdata, "evade_failure_w");
		m_evade_failure_e_name =
			load_animations_from_string
				(directory, prof, global_s, encdata, "evade_failure_e");
		m_die_w_name =
			load_animations_from_string
				(directory, prof, global_s, encdata, "die_w");
		m_die_e_name =
			load_animations_from_string
				(directory, prof, global_s, encdata, "die_e");
	}

}

std::vector<std::string> Soldier_Descr::load_animations_from_string
	(std::string const & directory, Profile & prof,
	 Section & global_s, EncodeData const * const encdata,
	 const char * anim_name)
{
	try {
		const char * anim_string = global_s.get_safe_string(anim_name);
		std::vector<std::string> list(split_string(anim_string, ","));
		if (list.size() < 1)
			throw game_data_error
				(_("expected %s but found \"%s\""),
				 _("\"anim_name[,another_anim,...]\""), anim_string);

		// Sanitation
		container_iterate(std::vector<std::string>, list, i) {
			remove_spaces(*i.current);

			// Check that section exists
			Section &
				anim_s = prof.get_safe_section((*i.current).c_str());

			m_default_encodedata.parse(anim_s);

			add_animation
				((*i.current).c_str(),
				 g_anim.get (directory, anim_s, "idle_00.png", encdata));
		}
		return list;
	} catch (_wexception const & e) {
		throw game_data_error("%s : %s", anim_name, e.what());
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
uint32_t Soldier_Descr::get_rand_anim
	(Game & game, const char * const animation_name) const
{
	std::string run = animation_name;

	if (strcmp(animation_name, "attack_success_w") == 0) {
		assert(m_attack_success_w_name.size() > 0);
		uint32_t i = game.logic_rand() % m_attack_success_w_name.size();
		run = m_attack_success_w_name[i];
	}

	if (strcmp(animation_name, "attack_success_e") == 0) {
		assert(m_attack_success_e_name.size() > 0);
		uint32_t i = game.logic_rand() % m_attack_success_e_name.size();
		run = m_attack_success_e_name[i];
	}

	if (strcmp(animation_name, "attack_failure_w") == 0) {
		assert(m_attack_failure_w_name.size() > 0);
		uint32_t i = game.logic_rand() % m_attack_failure_w_name.size();
		run = m_attack_failure_w_name[i];
	}

	if (strcmp(animation_name, "attack_failure_e") == 0) {
		assert(m_attack_failure_e_name.size() > 0);
		uint32_t i = game.logic_rand() % m_attack_failure_e_name.size();
		run = m_attack_failure_e_name[i];
	}

	if (strcmp(animation_name, "evade_success_w") == 0) {
		assert(m_evade_success_w_name.size() > 0);
		uint32_t i = game.logic_rand() % m_evade_success_w_name.size();
		run = m_evade_success_w_name[i];
	}

	if (strcmp(animation_name, "evade_success_e") == 0) {
		assert(m_evade_success_e_name.size() > 0);
		uint32_t i = game.logic_rand() % m_evade_success_e_name.size();
		run = m_evade_success_e_name[i];
	}

	if (strcmp(animation_name, "evade_failure_w") == 0) {
		assert(m_evade_failure_w_name.size() > 0);
		uint32_t i = game.logic_rand() % m_evade_failure_w_name.size();
		run = m_evade_failure_w_name[i];
	}

	if (strcmp(animation_name, "evade_failure_e") == 0) {
		assert(m_evade_failure_e_name.size() > 0);
		uint32_t i = game.logic_rand() % m_evade_failure_e_name.size();
		run = m_evade_failure_e_name[i];
	}
	if (strcmp(animation_name, "die_w") == 0) {
		assert(m_die_w_name.size() > 0);
		uint32_t i = game.logic_rand() % m_die_w_name.size();
		run = m_die_w_name[i];
	}

	if (strcmp(animation_name, "die_e") == 0) {
		assert(m_die_e_name.size() > 0);
		uint32_t i = game.logic_rand() % m_die_e_name.size();
		run = m_die_e_name[i];
	}

	log(" get %s\n", run.c_str());
	return get_animation(run.c_str());
}

/**
 * Create a new soldier
 */
Bob & Soldier_Descr::create_object() const {return *new Soldier(*this);}

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


void Soldier::init(Editor_Game_Base & egbase)
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
	{
		const uint32_t min_hp = descr().get_min_hp();
		assert(min_hp);
		assert(min_hp <= descr().get_max_hp());
		m_hp_max =
			min_hp
			+
			ref_cast<Game, Editor_Game_Base>(egbase).logic_rand()
			%
			(descr().get_max_hp() - (min_hp - 1));
	}
	m_hp_current    = m_hp_max;

	m_combat_walking   = CD_NONE;
	m_combat_walkstart = 0;
	m_combat_walkend   = 0;

	Worker::init(egbase);
}

void Soldier::cleanup(Editor_Game_Base & egbase)
{
	Worker::cleanup(egbase);
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


int32_t Soldier::get_tattribute(uint32_t const attr) const
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
	molog ("[soldier] healing (%d+)%d/%d\n", hp, m_hp_current, m_hp_max);
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

	molog ("[soldier] damage %d(-%d)/%d\n", m_hp_current, value, m_hp_max);
	if (m_hp_current < value)
		m_hp_current = 0;
	else
		m_hp_current -= value;
}

/// Calculates the actual position to draw on from the base node position.
/// This function takes battling into account.
///
/// pos is the location, in pixels, of the node m_position (height is already
/// taken into account).
Point Soldier::calc_drawpos
	(const Editor_Game_Base & game, const Point pos) const
{
	if (m_combat_walking == CD_NONE) {
		return Bob::calc_drawpos(game, pos);
	}

	const FCoords end = get_position();
	bool moving = false;
	Point spos = pos, epos = pos;

	switch (m_combat_walking) {
		case CD_WALK_W:
			moving = true;
			epos.x -= TRIANGLE_WIDTH / 4;
			break;
		case CD_WALK_E:
			moving = true;
			epos.x += TRIANGLE_WIDTH / 4;
			break;
		case CD_RETURN_W:
			moving = true;
			spos.x -= TRIANGLE_WIDTH / 4;
			break;
		case CD_RETURN_E:
			moving = true;
			spos.x += TRIANGLE_WIDTH / 4;
			break;
		case CD_COMBAT_W:
			moving = false;
			epos.x -= TRIANGLE_WIDTH / 4;
			break;
		case CD_COMBAT_E:
			moving = false;
			epos.x += TRIANGLE_WIDTH / 4;
			break;
		case CD_NONE:
			break;
	}

	if (moving) {

		float f =
			static_cast<float>(game.get_gametime() - m_combat_walkstart)
			/
			(m_combat_walkend - m_combat_walkstart);
		assert(m_combat_walkstart <= game.get_gametime());
		assert(m_combat_walkstart < m_combat_walkend);

		if (f < 0)
			f = 0;
		else if (f > 1)
			f = 1;

		epos.x = static_cast<int32_t>(f * epos.x + (1 - f) * spos.x);
	}
	return epos;
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
		g_gr->get_animation_size
			(anim,
			 game.get_gametime() - get_animstart(),
			 w,
			 h);

	// Draw energy bar
	// first: draw white sourrounding
		// Reduces energy bar length, so no overlap between soldiers
		w = w * 3 / 5;
		Rect r(Point(drawpos.x - w, drawpos.y - h - 7), w * 2, 5);
		dst.draw_rect(r, HP_FRAMECOLOR);
		// Draw the actual bar
		assert(m_hp_max);
		const float fraction = static_cast<float>(m_hp_current) / m_hp_max;
		//  FIXME Draw bar in playercolor, should be removed when soldier is
		//  FIXME correctly painted.
		RGBColor color
			(owner().get_playercolor()->r(),
			 owner().get_playercolor()->g(),
			 owner().get_playercolor()->b());
#if 0
		if (fraction <= 0.15)
			color = RGBColor(255, 0, 0);
		else if (fraction <= 0.5)
			color = RGBColor(255, 255, 0);
		else
			color = RGBColor(17, 192, 17);
#endif
		assert(2 <= r.w);
		assert(2 <= r.h);
		dst.fill_rect
			(Rect
			 	(r + Point(1, 1),
			 	 static_cast<int32_t>(fraction * (r.w - 2)), r.h - 2),
			 color);

		//  Draw information fields about levels. First, gather information.
		const PictureID hppic = get_hp_level_pic();
		const PictureID attackpic = get_attack_level_pic();
		const PictureID defensepic = get_defense_level_pic();
		const PictureID evadepic = get_evade_level_pic();
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
	(Editor_Game_Base & egbase,
	 char const * const animname,
	 uint32_t const time)
{
	molog("[soldier] starting animation %s", animname);
	return
		start_task_idle
			(ref_cast<Game, Editor_Game_Base>(egbase),
			 descr().get_rand_anim
			 	(ref_cast<Game, Editor_Game_Base>(egbase), animname),
			 time);
}


/**
 * \return \c true if this soldier is considered to be on the battlefield
 */
bool Soldier::isOnBattlefield()
{
	return get_state(taskAttack) || get_state(taskDefense);
}


/**
 * \return \c true if this soldier is considered to be attacking the player
 */
bool Soldier::is_attacking_player(Game & game, Player & player)
{
	State * state = get_state(taskAttack);
	if (state) {
		if (upcast(PlayerImmovable, imm, state->objvar1.get(game))) {
			return (imm->get_owner() == &player);
		}
	}
	return false;
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
	if (m_hp_current < 1)  //< Soldier is dead!
		return false;
	if (!isOnBattlefield())
		return false;
	if (!m_battle)
		return true;
	return
		!m_battle->locked(ref_cast<Game, Editor_Game_Base>(owner().egbase()));
}

/**
 * Assign the soldier to a battle (may be zero).
 *
 * \note must only be called by the \ref Battle object
 */
void Soldier::setBattle(Game & game, Battle * const battle)
{
	if (m_battle != battle) {
		m_battle = battle;
		send_signal(game, "battle");
	}
}

/**
 * Set a fallback task.
 */
void Soldier::init_auto_task(Game & game) {
	if (get_current_hitpoints() < 1) {
		molog("[soldier] init_auto_task: die\n");
		return start_task_die(game);
	}

	return Worker::init_auto_task(game);
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

void Soldier::start_task_attack
	(Game & game, Building & building, uint32_t retreat)
{
	dynamic_cast<Attackable const &>(building);

	push_task(game, taskAttack);
	State & state  = top_state();
	state.objvar1  = &building;
	state.coords   = building.get_position();

	if (retreat) {
		assert(retreat < 101);
		state.ivar1    |= CF_RETREAT_WHEN_INJURED;
		state.ui32var3  = retreat * get_max_hitpoints() / 100;

		// Injured soldiers are not allowed to attack
		if (state.ui32var3 > get_current_hitpoints()) {
			state.ui32var3 = get_current_hitpoints();
			//send_signal(game, "injured");
		}
	}

}

void Soldier::attack_update(Game & game, State & state)
{
	std::string signal = get_signal();

	if (signal.size()) {
		if
			(signal == "blocked" || signal == "battle" || signal == "wakeup" ||
			 signal == "sleep")
			signal_handled();
		else if (signal == "fail") {
			signal_handled();
			if (state.objvar1.get(game)) {
				molog("[attack] failed to reach enemy\n");
				state.objvar1 = 0;
			} else {
				molog("[attack] unexpected fail\n");
				return pop_task(game);
			}
		} else {
			molog
				("[attack] cancelled by unexpected signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	//  We are at enemy building flag, and a defender is coming, sleep until he
	// "wake up"s me
	if (signal == "sleep") {
		return start_task_idle(game, get_animation("idle"), -1);
	}

	PlayerImmovable * const location = get_location(game);
	BaseImmovable * const imm = game.map()[get_position()].get_immovable();
	upcast(Building, enemy, state.objvar1.get(game));

	if (imm == location) {
		if (!enemy) {
			molog("[attack] returned home\n");
			return pop_task(game);
		}
		return start_task_leavebuilding(game, false);
	}

	if (m_battle)
		return start_task_battle(game);

	if (signal == "blocked")
		// Wait before we try again. Note that this must come *after*
		// we check for a battle
		return start_task_idle(game, get_animation("idle"), 250);

	if (!location) {
		molog("[attack] our location disappeared during a battle\n");
		return pop_task(game);
	}

	if
		(!enemy or
		 (state.ivar1 & CF_RETREAT_WHEN_INJURED and
		  state.ui32var3 > get_current_hitpoints()))
	{
		// Injured soldiers will try to return to safe site at home.
		if (state.ui32var3 > get_current_hitpoints()) {
			state.coords = Coords(0, 0);
			state.objvar1 = 0;
		}
		// The old militarysite gets replaced by a new one, so if "enemy" is not
		// valid anymore, we either "conquered" the new building, or it was
		// destroyed.
		BaseImmovable * const newimm = game.map()[state.coords].get_immovable();
		upcast(MilitarySite, newsite, newimm);
		if (newsite and (&newsite->owner() == &owner())) {
			if (upcast(SoldierControl, ctrl, newsite)) {
				state.objvar1 = 0;
				if
					(ctrl->stationedSoldiers().size() < ctrl->soldierCapacity() and
					 location->base_flag().get_position()
					 !=
					 newsite ->base_flag().get_position())
				{
					molog("[attack] enemy belongs to us now, move in\n");
					pop_task(game);
					set_location(newsite);
					newsite->update_soldier_request();
					return schedule_act(game, 10);
				}
			}
		}
		Flag & baseflag = location->base_flag();
		if (imm == &baseflag)
			return
				start_task_move
					(game,
					 WALK_NW,
					 &descr().get_right_walk_anims(does_carry_ware()),
					 true);

		if
			(start_task_movepath
			 	(game,
			 	 baseflag.get_position(),
			 	 0,
			 	 descr().get_right_walk_anims(does_carry_ware())))
			return;
		else {
			molog("[attack] failed to return home\n");
			return pop_task(game);
		}
	}

	// At this point, we know that the enemy building still stands,
	// and that we're outside in the plains.
	if (imm != &enemy->base_flag()) {
		if
			(start_task_movepath
			 	(game,
			 	 enemy->base_flag().get_position(),
			 	 2,
			 	 descr().get_right_walk_anims(does_carry_ware())))
			return;
		else {
			molog("[attack] failed to move towards building flag\n");
			return pop_task(game);
		}
	}

	upcast(Attackable, attackable, enemy);
	assert(attackable);

	molog("[attack] attacking target building\n");
	//  give the enemy soldier some time to act
	schedule_act(game, attackable->attack(*this) ? 1000 : 10);
}

void Soldier::attack_pop(Game & game, State &)
{
	if (m_battle)
		m_battle->cancel(game, *this);
}


struct FindSoldierAttackingPlayer : public FindBob {
	FindSoldierAttackingPlayer(Game & _game, Player & _player) :
		player(_player),
		game(_game) {}

	bool accept(Bob * const bob) const
	{
		if (upcast(Soldier, soldier, bob)) {
			return
				soldier->get_current_hitpoints() and
				soldier->is_attacking_player(game, player);
		}
		return false;
	}

	Player & player;
	Game & game;
};

/**
 * Soldiers with this task go out of his buildings. They will
 * try to find an enemy in his lands and go to hunt them down (signaling
 * "battle"). If no enemy was found inside our lands, but an enemy is found
 * outside our lands, then wait until the enemy goes inside or dissapear.
 * If no enemy is found, then return home.
 *
 * Variables used:
 * \li ivar1 used to store \c CombatFlags
 * \li ivar2 when CF_DEFEND_STAYHOME, 1 if it has reached the flag
//           when CF_RETREAT_WHEN_INJURED, the lesser HP before retreat
 */
Bob::Task Soldier::taskDefense = {
	"defense",
	static_cast<Bob::Ptr>(&Soldier::defense_update),
	0,
	static_cast<Bob::Ptr>(&Soldier::defense_pop)
};

void Soldier::start_task_defense
	(Game & game, bool stayhome, uint32_t retreat)
{
	molog("[defense] starting\n");
	push_task(game, taskDefense);
	State & state = top_state();

	state.ivar1 = 0;
	state.ivar2 = 0;

	// Here goes 'configuration'
	if (stayhome) {
		state.ivar1 |= CF_DEFEND_STAYHOME;
	} else {
		/* Flag defenders are not allowed to retreat, to avoid abuses */
		if (retreat) {
			state.ivar1 |= CF_RETREAT_WHEN_INJURED;
			state.ui32var3 = get_max_hitpoints() * retreat / 100;

			// Soldier must defend even if he starts injured
			if (state.ui32var3 < get_current_hitpoints())
				state.ui32var3 = get_current_hitpoints();
		}
	}
}

void Soldier::defense_update(Game & game, State & state)
{
	std::string signal = get_signal();

	if (signal.size()) {
		if (signal == "blocked" || signal == "battle" || signal == "wakeup") {
			signal_handled();
		} else {
			molog("[defense] cancelled by signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	PlayerImmovable * const location = get_location(game);
	Flag & baseflag = location->base_flag();
	BaseImmovable * const position = game.map()[get_position()].get_immovable();

	if (signal == "blocked")
		// Wait before we try again. Note that this must come *after*
		// we check for a battle
		return start_task_idle(game, get_animation("idle"), 250);

	// If we only are defending our home ...
	if (state.ivar1 & CF_DEFEND_STAYHOME) {
		if ((position == location) and (state.ivar2 == 1)) {
			molog("[defense] stayhome: returned home\n");
			return pop_task(game);
		}

		if (position == &baseflag) {
			state.ivar2 = 1;
			assert(state.ivar2 == 1);

			if (m_battle)
				return start_task_battle(game);

			// Check if any attacker is waiting us to fight
			std::vector<Bob *> soldiers;
			game.map().find_bobs
				(Area<FCoords>(get_position(), 0),
				 &soldiers,
				 FindBobEnemySoldier(*get_owner()));

			container_iterate_const(std::vector<Bob *>, soldiers, i) {
				if (upcast(Soldier, soldier, *i.current)) {
					if (soldier->canBeChallenged()) {
						new Battle(game, *this, *soldier);
						return start_task_battle(game);
					}
				}
			}

			if (state.ivar2 == 1) {
				molog("[defense] stayhome: return home\n");
				return start_task_return(game, false);
			}
		}

		molog("[defense] stayhome: leavebuilding\n");
		return start_task_leavebuilding(game, false);
	}


	// We are outside our building, get list of enemy soldiers attacking us
	std::vector<Bob *> soldiers;
	game.map().find_bobs
		(Area<FCoords>(get_position(), 10),
		 &soldiers,
		 FindSoldierAttackingPlayer(game, *get_owner()));

	if
		(soldiers.empty() or
		 ((state.ivar1 & CF_RETREAT_WHEN_INJURED) and
		  get_current_hitpoints() < state.ui32var3))
	{

		if (soldiers.empty())
			molog("[defense] no enemy soldiers found, ending task\n");
		else
			molog("[defense] I'm heavily injured!\n");

		// If no enemy was found, return home
		if (!location) {
			molog("[defense] location disappeared during battle\n");
			return pop_task(game);
		}

		if (position == location) {
			molog("[defense] returned home\n");
			return pop_task(game);
		}

		if (position == &baseflag) {
			return
				start_task_move
					(game,
					 WALK_NW,
					 &descr().get_right_walk_anims(does_carry_ware()),
					 true);
		}

		molog("[defense] return home\n");
		if
			(start_task_movepath
			 	(game,
			 	 baseflag.get_position(),
			 	 0,
			 	 descr().get_right_walk_anims(does_carry_ware())))
			return;

		molog("[defense] could not find way home\n");
		return pop_task(game);
	}

	// Go through soldiers
	Soldier * target = 0;
	uint32_t target_dist = 999;
	container_iterate_const(std::vector<Bob *>, soldiers, i) {

		// If enemy is in our land, then go after it!
		if (upcast(Soldier, soldier, *i.current)) {
			assert(soldier != this);
			Field const f = game.map().operator[](soldier->get_position());

			//  Check soldier, be sure that we can fight against soldier.
			// Only pursuers can go over enemy land when defending.
			if
				((soldier->canBeChallenged()) and
				 (f.get_owned_by() == get_owner()->player_number()))
			{
				uint32_t thisDist = game.map().calc_distance
					(get_position(), soldier->get_position());
				if (thisDist < target_dist) {
					target_dist = thisDist;
					target = soldier;
				}
			}
		}
	}

	if (target) {

		if (position == location)
			return start_task_leavebuilding(game, false);

		if (target_dist <= 1) {
			molog("[defense] starting battle with %u!\n", target->serial());
			new Battle(game, *this, *target);
			return start_task_battle(game);
		}

		// Move towards soldier
		if
			(start_task_movepath
			 	(game,
			 	 target->get_position(),
			 	 1,
			 	 descr().get_right_walk_anims(does_carry_ware()),
			 	 false,
			 	 1))
		{
			molog("[defense] move towards soldier %u\n", target->serial());
			return;
		} else {
			molog
				("[defense] failed to move towards attacking soldier %u\n",
				 target->serial());
			return pop_task(game);
		}
	}

	// If the enemy is not in our land, wait
	return start_task_idle(game, get_animation("idle"), 250);
}

void Soldier::defense_pop(Game & game, State &)
{
	if (m_battle)
		m_battle->cancel(game, *this);
}


Bob::Task Soldier::taskMoveInBattle = {
	"moveInBattle",
	static_cast<Bob::Ptr>(&Soldier::move_in_battle_update),
	0,
	0
};

void Soldier::start_task_move_in_battle(Game & game, CombatWalkingDir dir)
{
	int32_t mapdir = IDLE;

	switch (dir) {
		case CD_WALK_W:
		case CD_RETURN_E:
			mapdir = WALK_W;
			break;
		case CD_WALK_E:
		case CD_RETURN_W:
			mapdir = WALK_E;
			break;
		default:
			throw game_data_error(_("bad direction '%d'"), dir);
	}

	Map & map = game.map();
	int32_t const tdelta = (map.calc_cost(get_position(), mapdir)) / 2;
	molog("[move_in_battle] dir: (%d) tdelta: (%d)\n", dir, tdelta);
	m_combat_walking   = dir;
	m_combat_walkstart = game.get_gametime();
	m_combat_walkend   = m_combat_walkstart + tdelta;

	push_task(game, taskMoveInBattle);
	State & state = top_state();
	state.ivar1 = dir;
	set_animation
		(game, descr().get_animation(mapdir == WALK_E ? "walk_e" : "walk_w"));
}

void Soldier::move_in_battle_update(Game & game, State &)
{
	if (m_combat_walkend <= game.get_gametime()) {
		switch (m_combat_walking) {
			case CD_NONE:
				break;
			case CD_WALK_W:
				m_combat_walking = CD_COMBAT_W;
				break;
			case CD_WALK_E:
				m_combat_walking = CD_COMBAT_E;
				break;
			case CD_RETURN_W:
			case CD_RETURN_E:
			case CD_COMBAT_W:
			case CD_COMBAT_E:
				m_combat_walking = CD_NONE;
				break;
		}
		return pop_task(game);
	} else
		//  Only end the task once we've actually completed the step
		// Ignore signals until then
		return schedule_act(game, m_combat_walkend - game.get_gametime());
}

/**
 * \return \c true if the defending soldier should not stray from
 * his home flag.
 */
bool Soldier::stayHome()
{
	if (State const * const state = get_state(taskDefense))
		return state->ivar1 & CF_DEFEND_STAYHOME;
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

void Soldier::start_task_battle(Game & game)
{
	assert(m_battle);
	m_combat_walking = CD_NONE;

	push_task(game, taskBattle);
}

void Soldier::battle_update(Game & game, State &)
{
	std::string signal = get_signal();
	molog
		("[battle] update for player %u's soldier: signal = \"%s\"\n",
		 owner().player_number(), signal.c_str());

	if (signal.size()) {
		if (signal == "blocked") {
			signal_handled();
			return start_task_idle(game, get_animation("idle"), 5000);
		} else if
			(signal == "location" || signal == "battle" || signal == "wakeup")
			signal_handled();
		else {
			molog
				("[battle] interrupted by unexpected signal '%s'\n",
				 signal.c_str());
			return pop_task(game);
		}
	}

	if (!m_battle) {
		if (m_combat_walking == CD_COMBAT_W) {
			return start_task_move_in_battle(game, CD_RETURN_W);
		}
		if (m_combat_walking == CD_COMBAT_E) {
			return start_task_move_in_battle(game, CD_RETURN_E);
		}
		assert(m_combat_walking == CD_NONE);
		molog("[battle] is over\n");
		sendSpaceSignals(game);
		return pop_task(game);
	}

	if (stayHome()) {
		if (this == m_battle->first()) {
			molog("[battle] stayHome, so reverse roles\n");
			new Battle(game, *m_battle->second(), *m_battle->first());
			return skip_act(); //  we will get a signal via setBattle()
		} else {
			if (m_combat_walking != CD_COMBAT_E) {
				return start_task_move_in_battle(game, CD_WALK_E);
			}
		}
	} else {
		Soldier & opponent = *m_battle->opponent(*this);

		if (opponent.stayHome() and (this == m_battle->second())) {
			// Wait until correct roles are assigned
			new Battle(game, *m_battle->second(), *m_battle->first());
			return schedule_act(game, 10);
		}

		if (opponent.get_position() != get_position()) {
			Map & map = game.map();
			uint32_t const dist =
				map.calc_distance(get_position(), opponent.get_position());

			if (dist >= 2 || this == m_battle->first()) {
				//  Only make small steps at a time, so we can adjust to the
				//  opponent's change of position.
				Coords dest = opponent.get_position();
				if (upcast(Building, building, map[dest].get_immovable()))
					dest = building->base_flag().get_position();
				if
					(start_task_movepath
					 	(game,
					 	 dest,
					 	 0,
					 	 descr().get_right_walk_anims(does_carry_ware()),
					 	 false, (dist + 3) / 4))
				{
					molog
						("[battle] player %u's soldier started task_movepath\n",
						 owner().player_number());
					return;
				} else {
					BaseImmovable const * const immovable_position =
						get_position().field->get_immovable();
					BaseImmovable const * const immovable_dest     =
						map[dest]            .get_immovable();
					char buffer[2048];
					snprintf
						(buffer, sizeof(buffer),
						 _
						 	("The game engine has encountered a logic error. The %s "
						 	 "#%u of player %u could not find a way from (%i, %i) "
						 	 "(with %s immovable) to the opponent (%s #%u of player "
						 	 "%u) at (%i, %i) (with %s immovable). The %s will now "
						 	 "desert (but will not be executed). Strange things may "
						 	 "happen. No solution for this problem has been "
						 	 "implemented yet. (bug #1951113) (The game has been "
						 	 "paused.)"),
						 descname().c_str(), serial(), owner().player_number(),
						 get_position().x, get_position().y,
						 immovable_position ?
						 immovable_position->descr().descname().c_str() : _("no"),
						 opponent.descname().c_str(), opponent.serial(),
						 opponent.owner().player_number(),
						 dest.x, dest.y,
						 immovable_dest ?
						 immovable_dest->descr().descname().c_str() : _("no"),
						 descname().c_str());
					MessageQueue::add
						(owner(),
						 Message
						 	(_("Game engine"),
						 	 game.get_gametime(),
						 	 _("Logic error"),
						 	 Widelands::Coords(get_position()),
						 	 buffer));
					MessageQueue::add
						(opponent.owner(),
						 Message
						 	(_("Game engine"),
						 	 game.get_gametime(),
						 	 _("Logic error"),
						 	 Widelands::Coords(get_position()),
						 	 buffer));
					game.gameController()->setDesiredSpeed(0);
					return pop_task(game);
				}
			}
		} else {

			assert(opponent.get_position() == get_position());
			assert(m_battle == opponent.getBattle());

			if (opponent.is_walking()) {
				molog
					("[battle]: Opponent '%d' is walking, sleeping\n",
					 opponent.serial());
				return start_task_idle(game, descr().get_animation("idle"), 100);
			}

			if (m_battle->first()->serial() == serial()) {
				molog("[battle]: I am first: '%d'\n", m_combat_walking);
				if (m_combat_walking != CD_COMBAT_W) {
					start_task_move_in_battle(game, CD_WALK_W);
					return;
				}
			}

			if (m_battle->second()->serial() == serial()) {
				molog("[battle]: I am second: '%d'\n", m_combat_walking);
				if (m_combat_walking != CD_COMBAT_E) {
					start_task_move_in_battle(game, CD_WALK_E);
					return;
				}
			}
		}
	}

	m_battle->getBattleWork(game, *this);
}

void Soldier::battle_pop(Game & game, State &)
{
	if (m_battle)
		m_battle->cancel(game, *this);
}


Bob::Task Soldier::taskDie = {
	"die",
	static_cast<Bob::Ptr>(&Soldier::die_update),
	0,
	static_cast<Bob::Ptr>(&Soldier::die_pop)
};

void Soldier::start_task_die(Game & game)
{
	push_task(game, taskDie);
	top_state().ivar1 = game.get_gametime() + 1000;

	start_task_idle(game, descr().get_animation("idle"), 1000);
}

void Soldier::die_update(Game & game, State & state)
{
	std::string signal = get_signal();
	molog
		("[die] update for player %u's soldier: signal = \"%s\"\n",
		 owner().player_number(), signal.c_str());

	if (signal.size()) {
		signal_handled();
	}

	if (state.ivar1 > game.get_gametime())
		return schedule_act(game, state.ivar1 - game.get_gametime());

	// When task updated, dead is near!
	return pop_task(game);
}

void Soldier::die_pop(Game & game, State &)
{
	// Destroy the soldier!
	molog("[die] soldier %u has died\n", serial());
	schedule_destroy(game);
}


struct FindSoldierOnBattlefield : public FindBob {
	bool accept(Bob * const bob) const
	{
		if (upcast(Soldier, soldier, bob))
			return
				soldier->isOnBattlefield() and
				soldier->get_current_hitpoints();
		return false;
	}
};

/**
 * Override \ref Bob::checkNodeBlocked.
 *
 * As long as we're on the battlefield, check for other soldiers.
 */
bool Soldier::checkNodeBlocked
	(Game & game, FCoords const & field, bool const commit)
{
	if (!isOnBattlefield())
		return false;

	if (upcast(Building, building, get_location(game))) {
		if (field == building->get_position()) {
			if (commit)
				sendSpaceSignals(game);
			return false; // we can always walk home
		}
	}

	std::vector<Bob *> soldiers;
	game.map().find_bobs
		(Area<FCoords>(field, 0), &soldiers, FindSoldierOnBattlefield());

	if
		(soldiers.size() &&
		 (!m_battle ||
		  std::find(soldiers.begin(), soldiers.end(), m_battle->opponent(*this))
		  ==
		  soldiers.end()))
	{
		if (commit && soldiers.size() == 1) {
			Soldier & soldier = ref_cast<Soldier, Bob>(*soldiers[0]);
			if (soldier.get_owner() != get_owner() && soldier.canBeChallenged()) {
				molog("[checkNodeBlocked] attacking a soldier\n");
				new Battle(game, *this, soldier);
			}
		}

		/// Retreating soldiers don't check for blocked paths
		State * state = 0;

		if (get_state(taskAttack)) {
			state = get_state(taskAttack);
		}
		if (get_state(taskDefense)) {
			state = get_state(taskDefense);
		}
		if (state) {
			if
				(state->ivar1 & CF_RETREAT_WHEN_INJURED and
				 state->ui32var3 > get_current_hitpoints())
			{
				return false;
			}
		}
		return true;
	}

	if (commit)
		sendSpaceSignals(game);
	return false;
}


/**
 * Send a "wakeup" signal to all surrounding soldiers that are out in the open,
 * so that they may repeat pathfinding.
 */
void Soldier::sendSpaceSignals(Game & game)
{
	std::vector<Bob *> soldiers;

	game.map().find_bobs
		(Area<FCoords>(get_position(), 1),
		 &soldiers,
		 FindSoldierOnBattlefield());

	container_iterate_const(std::vector<Bob *>, soldiers, i)
		if (upcast(Soldier, soldier, *i.current))
			if (soldier != this)
				soldier->send_signal(game, "wakeup");

	Player_Number const land_owner = get_position().field->get_owned_by();
	if (land_owner != owner().player_number()) {
		std::vector<BaseImmovable *> attackables;
		game.map().find_reachable_immovables_unique
			(Area<FCoords>(get_position(), MaxProtectionRadius),
			 attackables,
			 CheckStepWalkOn(descr().movecaps(), false),
			 FindImmovableAttackable());

		container_iterate_const(std::vector<BaseImmovable *>, attackables, i)
			if
				(ref_cast<PlayerImmovable const, BaseImmovable const>(**i.current)
				 .get_owner()->player_number()
				 ==
				 land_owner)
				dynamic_cast<Attackable &>(**i.current).aggressor(*this);
	}
}


void Soldier::log_general_info(Editor_Game_Base const & egbase)
{
	Worker::log_general_info(egbase);
	molog("[Soldier]\n");
	molog
		("Levels: %d/%d/%d/%d\n",
		 m_hp_level, m_attack_level, m_defense_level, m_evade_level);
	molog ("HitPoints: %d/%d\n", m_hp_current, m_hp_max);
	molog ("Attack :  %d-%d\n", m_min_attack, m_max_attack);
	molog ("Defense : %d%%\n", m_defense);
	molog ("Evade:    %d%%\n", m_evade);
	molog ("CombatWalkingDir:   %i\n", m_combat_walking);
	molog ("CombatWalkingStart: %i\n", m_combat_walkstart);
	molog ("CombatWalkEnd:      %i\n", m_combat_walkend);
	molog ("HasBattle:   %s\n", m_battle ? "yes" : "no");
	if (m_battle)
		molog("BattleSerial: %u\n", m_battle->serial());
}

}
