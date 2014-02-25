/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef SOLDIER_H
#define SOLDIER_H

#include "logic/tattribute.h"
#include "logic/worker.h"

#define SOLDIER_HP_BAR_WIDTH 13

struct RGBColor;

namespace Widelands {

// Constants used to launch attacks
#define WEAKEST   0
#define STRONGEST 1

class Editor_Game_Base;
class Battle;

#define HP_FRAMECOLOR RGBColor(255, 255, 255)

struct Soldier_Descr : public Worker_Descr {
	friend class Economy;
	Soldier_Descr
		(char const * const _name, char const * const _descname,
		 const std::string & directory, Profile &, Section & global_s,
		 const Tribe_Descr &);

	// NOTE we have to explicitly return Worker_Descr::SOLDIER, as SOLDIER is
	// NOTE as well defined in an enum in instances.h
	virtual Worker_Type get_worker_type() const override {return Worker_Descr::SOLDIER;}

	virtual void load_graphics() override;

	uint32_t get_max_hp_level          () const {return m_max_hp_level;}
	uint32_t get_max_attack_level      () const {return m_max_attack_level;}
	uint32_t get_max_defense_level     () const {return m_max_defense_level;}
	uint32_t get_max_evade_level       () const {return m_max_evade_level;}

	uint32_t get_base_hp        () const {return m_base_hp;}
	uint32_t get_base_min_attack() const {return m_min_attack;}
	uint32_t get_base_max_attack() const {return m_max_attack;}
	uint32_t get_base_defense   () const {return m_defense;}
	uint32_t get_base_evade     () const {return m_evade;}

	uint32_t get_hp_incr_per_level     () const {return m_hp_incr;}
	uint32_t get_attack_incr_per_level () const {return m_attack_incr;}
	uint32_t get_defense_incr_per_level() const {return m_defense_incr;}
	uint32_t get_evade_incr_per_level  () const {return m_evade_incr;}

	const Image* get_hp_level_pic     (uint32_t const level) const {
		assert(level <= m_max_hp_level);      return m_hp_pics     [level];
	}
	const Image* get_attack_level_pic (uint32_t const level) const {
		assert(level <= m_max_attack_level);  return m_attack_pics [level];
	}
	const Image* get_defense_level_pic(uint32_t const level) const {
		assert(level <= m_max_defense_level); return m_defense_pics[level];
	}
	const Image* get_evade_level_pic  (uint32_t const level) const {
		assert(level <= m_max_evade_level);   return m_evade_pics  [level];
	}



	uint32_t get_rand_anim(Game & game, const char * const name) const;

protected:
	virtual Bob & create_object() const override;

	//  start values
	uint32_t m_base_hp;
	uint32_t m_min_attack;
	uint32_t m_max_attack;
	uint32_t m_defense;
	uint32_t m_evade;

	//  per level increases
	uint32_t m_hp_incr;
	uint32_t m_attack_incr;
	uint32_t m_defense_incr;
	uint32_t m_evade_incr;

	//  max levels
	uint32_t m_max_hp_level;
	uint32_t m_max_attack_level;
	uint32_t m_max_defense_level;
	uint32_t m_max_evade_level;

	//  level pictures
	std::vector<const Image* >   m_hp_pics;
	std::vector<const Image* >   m_attack_pics;
	std::vector<const Image* >   m_evade_pics;
	std::vector<const Image* >   m_defense_pics;
	std::vector<std::string> m_hp_pics_fn;
	std::vector<std::string> m_attack_pics_fn;
	std::vector<std::string> m_evade_pics_fn;
	std::vector<std::string> m_defense_pics_fn;

	// animation names
	std::vector<std::string> m_attack_success_w_name;
	std::vector<std::string> m_attack_failure_w_name;
	std::vector<std::string> m_evade_success_w_name;
	std::vector<std::string> m_evade_failure_w_name;
	std::vector<std::string> m_die_w_name;

	std::vector<std::string> m_attack_success_e_name;
	std::vector<std::string> m_attack_failure_e_name;
	std::vector<std::string> m_evade_success_e_name;
	std::vector<std::string> m_evade_failure_e_name;
	std::vector<std::string> m_die_e_name;

	std::vector<std::string> load_animations_from_string
			(const std::string & directory, Profile & prof, Section & global_s,
			 const char * anim_name);

};

class Building;

enum CombatWalkingDir {
	CD_NONE = 0,     // Not in combat
	CD_WALK_W = 1,   // Going to west       (facing west)
	CD_WALK_E = 2,   // Going to east       (facing east)
	CD_COMBAT_W = 3, // Fighting at west    (facing east!!)
	CD_COMBAT_E = 4, // Fighting at east    (facing west!!)
	CD_RETURN_W = 5, // Returning from west (facing east!!)
	CD_RETURN_E = 6, // Returning from east (facing west!!)
};

enum CombatFlags {
	/**
	 * CF_DEFEND_STAY_HOME
	 *    Soldier will wait enemies at his building flag. Only for defenders.
	 */
	CF_DEFEND_STAYHOME = 1,
	/**
	 * CF_RETREAT_WHEN_INJURED
	 *    When current hitpoints goes under arbitrary value, soldier will flee
	 * and heal inside military building
	 */
	CF_RETREAT_WHEN_INJURED = 2,
	/**
	 * CF_AVOID_COMBAT
	 *    Attackers would try avoid entering combat with others soldiers but
	 * 'flag deffenders'.
	 */
	CF_AVOID_COMBAT = 4,

};


class Soldier : public Worker {
	friend struct Map_Bobdata_Data_Packet;
	MO_DESCR(Soldier_Descr);

public:
	Soldier(const Soldier_Descr &);

	virtual void init(Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;

	void set_level
		(uint32_t hp, uint32_t attack, uint32_t defense, uint32_t evade);
	void set_hp_level     (uint32_t);
	void set_attack_level (uint32_t);
	void set_defense_level(uint32_t);
	void set_evade_level  (uint32_t);
	uint32_t get_level (tAttribute) const;
	uint32_t get_hp_level     () const {return m_hp_level;}
	uint32_t get_attack_level () const {return m_attack_level;}
	uint32_t get_defense_level() const {return m_defense_level;}
	uint32_t get_evade_level  () const {return m_evade_level;}

	/// Automatically select a task.
	void init_auto_task(Game &) override;

	Point calc_drawpos(const Editor_Game_Base &, Point) const;
	/// Draw this soldier
	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const Point&) const override;

	static void calc_info_icon_size
		(const Tribe_Descr &, uint32_t & w, uint32_t & h);
	void draw_info_icon(RenderTarget &, Point, bool anchor_below) const;

	//  Information function from description.
	uint32_t get_max_hp_level     () const {
		return descr().get_max_hp_level();
	}
	uint32_t get_max_attack_level () const {
		return descr().get_max_attack_level();
	}
	uint32_t get_max_defense_level() const {
		return descr().get_max_defense_level();
	}
	uint32_t get_max_evade_level  () const {
		return descr().get_max_evade_level();
	}

	uint32_t get_current_hitpoints() const {return m_hp_current;}
	uint32_t get_max_hitpoints() const;
	uint32_t get_min_attack() const;
	uint32_t get_max_attack() const;
	uint32_t get_defense() const;
	uint32_t get_evade() const;

	const Image* get_hp_level_pic     () const {
		return descr().get_hp_level_pic     (m_hp_level);
	}
	const Image* get_attack_level_pic () const {
		return descr().get_attack_level_pic (m_attack_level);
	}
	const Image* get_defense_level_pic() const {
		return descr().get_defense_level_pic(m_defense_level);
	}
	const Image* get_evade_level_pic  () const {
		return descr().get_evade_level_pic  (m_evade_level);
	}

	int32_t get_tattribute(uint32_t attr) const override;

	/// Sets a random animation of desired type and start playing it.
	void start_animation
		(Editor_Game_Base &, char const * animname, uint32_t time);

	/// Heal quantity of hit points instantly
	void heal (uint32_t);
	void damage (uint32_t); /// Damage quantity of hit points

	virtual void log_general_info(const Editor_Game_Base &) override;

	bool isOnBattlefield();
	bool is_attacking_player(Game &, Player &);
	Battle * getBattle();
	bool canBeChallenged();
	virtual bool checkNodeBlocked(Game &, const FCoords &, bool commit) override;

	void setBattle(Game &, Battle *);

	void start_task_attack(Game & game, Building &, uint8_t retreat);
	void start_task_defense(Game & game, bool stayhome, uint8_t retreat);
	void start_task_battle(Game &);
	void start_task_move_in_battle(Game &, CombatWalkingDir);
	void start_task_die(Game &);

private:
	void attack_update(Game &, State &);
	void attack_pop(Game &, State &);
	void defense_update(Game &, State &);
	void defense_pop(Game &, State &);
	void battle_update(Game &, State &);
	void battle_pop(Game &, State &);
	void move_in_battle_update(Game &, State &);
	void die_update(Game &, State &);
	void die_pop(Game &, State &);

	void sendSpaceSignals(Game &);
	bool stayHome();

	// Pop the current task or, if challenged, start the fighting task.
	void pop_task_or_fight(Game &);

protected:
	static Task const taskAttack;
	static Task const taskDefense;
	static Task const taskBattle;
	static Task const taskMoveInBattle;
	// May be this can be moved this to bob when finished
	static Task const taskDie;

	virtual bool is_evict_allowed() override;

private:
	uint32_t m_hp_current;
	uint32_t m_hp_level;
	uint32_t m_attack_level;
	uint32_t m_defense_level;
	uint32_t m_evade_level;

	/// This is used to replicate walk for soldiers but only just before and
	/// just after figthing in a battle, to draw soldier at proper position.
	/// Maybe Bob.m_walking could be used, but then that variable should be
	/// protected instead of private, and some type of rework needed to allow
	/// the new states. I thought that it is cleaner to have this variable
	/// separate.
	CombatWalkingDir m_combat_walking;
	int32_t  m_combat_walkstart;
	int32_t  m_combat_walkend;

	/**
	 * If the soldier is involved in a challenge, it is assigned a battle
	 * object.
	 */
	Battle * m_battle;

	// saving and loading
protected:
	struct Loader : public Worker::Loader {
	public:
		Loader();

		virtual void load(FileRead &) override;
		virtual void load_pointers() override;

	protected:
		virtual const Task * get_task(const std::string & name) override;

	private:
		uint32_t m_battle;
	};

	virtual Loader * create_loader() override;

public:
	virtual void do_save
		(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;
};

}

#endif
