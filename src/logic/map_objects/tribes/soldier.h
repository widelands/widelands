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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_SOLDIER_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_SOLDIER_H

#include <memory>

#include "base/macros.h"
#include "logic/map_objects/tribes/training_attribute.h"
#include "logic/map_objects/tribes/worker.h"

struct RGBColor;

namespace Widelands {

// Constants used to launch attacks
#define WEAKEST   0
#define STRONGEST 1

class EditorGameBase;
class Battle;

#define HP_FRAMECOLOR RGBColor(255, 255, 255)

class SoldierDescr : public WorkerDescr {
public:
	friend class Economy;

	SoldierDescr(const std::string& init_descname,
					 const LuaTable& t, const EditorGameBase& egbase);
	~SoldierDescr() override {}

	uint32_t get_max_hp_level          () const {return max_hp_level_;}
	uint32_t get_max_attack_level      () const {return max_attack_level_;}
	uint32_t get_max_defense_level     () const {return max_defense_level_;}
	uint32_t get_max_evade_level       () const {return max_evade_level_;}

	uint32_t get_base_hp        () const {return base_hp_;}
	uint32_t get_base_min_attack() const {return min_attack_;}
	uint32_t get_base_max_attack() const {return max_attack_;}
	uint32_t get_base_defense   () const {return defense_;}
	uint32_t get_base_evade     () const {return evade_;}

	uint32_t get_hp_incr_per_level     () const {return hp_incr_;}
	uint32_t get_attack_incr_per_level () const {return attack_incr_;}
	uint32_t get_defense_incr_per_level() const {return defense_incr_;}
	uint32_t get_evade_incr_per_level  () const {return evade_incr_;}

	const Image* get_hp_level_pic     (uint32_t const level) const {
		assert(level <= max_hp_level_);      return hp_pics_     [level];
	}
	const Image* get_attack_level_pic (uint32_t const level) const {
		assert(level <= max_attack_level_);  return attack_pics_ [level];
	}
	const Image* get_defense_level_pic(uint32_t const level) const {
		assert(level <= max_defense_level_); return defense_pics_[level];
	}
	const Image* get_evade_level_pic  (uint32_t const level) const {
		assert(level <= max_evade_level_);   return evade_pics_  [level];
	}

	uint32_t get_rand_anim(Game & game, const char * const name) const;

protected:
	Bob & create_object() const override;

	//  start values
	uint32_t base_hp_;
	uint32_t min_attack_;
	uint32_t max_attack_;
	uint32_t defense_;
	uint32_t evade_;

	//  per level increases
	uint32_t hp_incr_;
	uint32_t attack_incr_;
	uint32_t defense_incr_;
	uint32_t evade_incr_;

	//  max levels
	uint32_t max_hp_level_;
	uint32_t max_attack_level_;
	uint32_t max_defense_level_;
	uint32_t max_evade_level_;

	//  level pictures
	std::vector<const Image* >   hp_pics_;
	std::vector<const Image* >   attack_pics_;
	std::vector<const Image* >   evade_pics_;
	std::vector<const Image* >   defense_pics_;
	std::vector<std::string> hp_pics_fn_;
	std::vector<std::string> attack_pics_fn_;
	std::vector<std::string> evade_pics_fn_;
	std::vector<std::string> defense_pics_fn_;

	// animation names
	std::vector<std::string> attack_success_w_name_;
	std::vector<std::string> attack_failure_w_name_;
	std::vector<std::string> evade_success_w_name_;
	std::vector<std::string> evade_failure_w_name_;
	std::vector<std::string> die_w_name_;

	std::vector<std::string> attack_success_e_name_;
	std::vector<std::string> attack_failure_e_name_;
	std::vector<std::string> evade_success_e_name_;
	std::vector<std::string> evade_failure_e_name_;
	std::vector<std::string> die_e_name_;

private:
	// Reads list of animation names from the table and pushes them into result.
	void add_battle_animation(std::unique_ptr<LuaTable> table, std::vector<std::string>* result);

	DISALLOW_COPY_AND_ASSIGN(SoldierDescr);
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
	/// Soldier will wait enemies at his building flag. Only for defenders.
	CF_DEFEND_STAYHOME = 1,
	/// When current hitpoints goes under a fixed percentage, soldier will flee
	/// and heal inside military building
	CF_RETREAT_WHEN_INJURED = 2,
	/// Attackers would try avoid entering combat with others soldiers but 'flag
	/// defenders'.
	CF_AVOID_COMBAT = 4,
};


class Soldier : public Worker {
	friend struct MapBobdataPacket;
	MO_DESCR(SoldierDescr)

public:
	Soldier(const SoldierDescr &);

	void init(EditorGameBase &) override;
	void cleanup(EditorGameBase &) override;

	void set_level
		(uint32_t hp, uint32_t attack, uint32_t defense, uint32_t evade);
	void set_hp_level     (uint32_t);
	void set_attack_level (uint32_t);
	void set_defense_level(uint32_t);
	void set_evade_level  (uint32_t);
	uint32_t get_level (TrainingAttribute) const;
	uint32_t get_hp_level     () const {return hp_level_;}
	uint32_t get_attack_level () const {return attack_level_;}
	uint32_t get_defense_level() const {return defense_level_;}
	uint32_t get_evade_level  () const {return evade_level_;}
	uint32_t get_total_level () const {return hp_level_ + attack_level_ + defense_level_ + evade_level_;}

	/// Automatically select a task.
	void init_auto_task(Game &) override;

	Point calc_drawpos(const EditorGameBase &, Point) const;
	/// Draw this soldier
	void draw(const EditorGameBase&, RenderTarget&, const Point&) const override;

	static void calc_info_icon_size
		(const TribeDescr &, uint32_t & w, uint32_t & h);
	void draw_info_icon(RenderTarget &, Point, bool anchor_below) const;

	uint32_t get_current_hitpoints() const {return hp_current_;}
	uint32_t get_max_hitpoints() const;
	uint32_t get_min_attack() const;
	uint32_t get_max_attack() const;
	uint32_t get_defense() const;
	uint32_t get_evade() const;

	const Image* get_hp_level_pic     () const {
		return descr().get_hp_level_pic     (hp_level_);
	}
	const Image* get_attack_level_pic () const {
		return descr().get_attack_level_pic (attack_level_);
	}
	const Image* get_defense_level_pic() const {
		return descr().get_defense_level_pic(defense_level_);
	}
	const Image* get_evade_level_pic  () const {
		return descr().get_evade_level_pic  (evade_level_);
	}

	int32_t get_training_attribute(uint32_t attr) const override;

	/// Sets a random animation of desired type and start playing it.
	void start_animation
		(EditorGameBase &, char const * animname, uint32_t time);

	/// Heal quantity of hit points instantly
	void heal (uint32_t);
	void damage (uint32_t); /// Damage quantity of hit points

	void log_general_info(const EditorGameBase &) override;

	bool is_on_battlefield();
	bool is_attacking_player(Game &, Player &);
	Battle * get_battle();
	bool can_be_challenged();
	bool check_node_blocked(Game &, const FCoords &, bool commit) override;

	void set_battle(Game &, Battle *);

	void start_task_attack(Game & game, Building &);
	void start_task_defense(Game & game, bool stayhome);
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

	void send_space_signals(Game &);
	bool stay_home();

	// Pop the current task or, if challenged, start the fighting task.
	void pop_task_or_fight(Game &);

protected:
	static Task const taskAttack;
	static Task const taskDefense;
	static Task const taskBattle;
	static Task const taskMoveInBattle;
	// May be this can be moved this to bob when finished
	static Task const taskDie;

	bool is_evict_allowed() override;

private:
	uint32_t hp_current_;
	uint32_t hp_level_;
	uint32_t attack_level_;
	uint32_t defense_level_;
	uint32_t evade_level_;

	/// This is used to replicate walk for soldiers but only just before and
	/// just after figthing in a battle, to draw soldier at proper position.
	/// Maybe Bob.walking_ could be used, but then that variable should be
	/// protected instead of private, and some type of rework needed to allow
	/// the new states. I thought that it is cleaner to have this variable
	/// separate.
	CombatWalkingDir combat_walking_;
	uint32_t  combat_walkstart_;
	uint32_t  combat_walkend_;

	/**
	 * If the soldier is involved in a challenge, it is assigned a battle
	 * object.
	 */
	Battle * battle_;

	static constexpr uint8_t kSoldierHpBarWidth = 13;

	/// Number of consecutive blocked signals until the soldiers are considered permanently stuck
	static constexpr uint8_t kBockCountIsStuck = 10;

	// saving and loading
protected:
	struct Loader : public Worker::Loader {
	public:
		Loader();

		void load(FileRead &) override;
		void load_pointers() override;

	protected:
		const Task * get_task(const std::string & name) override;

	private:
		uint32_t battle_;
	};

	Loader * create_loader() override;

public:
	void do_save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
};

}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_SOLDIER_H
