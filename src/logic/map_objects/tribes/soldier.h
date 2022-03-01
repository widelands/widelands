/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_SOLDIER_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_SOLDIER_H

#include <memory>

#include "base/macros.h"
#include "logic/map_objects/tribes/training_attribute.h"
#include "logic/map_objects/tribes/worker.h"

namespace Widelands {

// Constants used to launch attacks
#define WEAKEST 0
#define STRONGEST 1

class EditorGameBase;
class Battle;

struct SoldierLevelRange {
	SoldierLevelRange();
	explicit SoldierLevelRange(const LuaTable&);
	SoldierLevelRange(const SoldierLevelRange&) = default;
	SoldierLevelRange& operator=(const SoldierLevelRange& other) = default;

	bool matches(const Soldier* soldier) const;
	bool matches(int32_t health, int32_t attack, int32_t defense, int32_t evade) const;

	bool operator==(const SoldierLevelRange& other) const {
		return min_health == other.min_health && min_attack == other.min_attack &&
		       min_defense == other.min_defense && min_evade == other.min_evade &&
		       max_health == other.max_health && max_attack == other.max_attack &&
		       max_defense == other.max_defense && max_evade == other.max_evade;
	}

	int32_t min_health;
	int32_t min_attack;
	int32_t min_defense;
	int32_t min_evade;
	int32_t max_health;
	int32_t max_attack;
	int32_t max_defense;
	int32_t max_evade;
};
using SoldierAnimationsList = std::map<std::string, SoldierLevelRange>;

class SoldierDescr : public WorkerDescr {
public:
	friend class Economy;

	SoldierDescr(const std::string& init_descname, const LuaTable& t, Descriptions& descriptions);
	~SoldierDescr() override {
	}

	uint32_t get_max_health_level() const {
		return health_.max_level;
	}
	uint32_t get_max_attack_level() const {
		return attack_.max_level;
	}
	uint32_t get_max_defense_level() const {
		return defense_.max_level;
	}
	uint32_t get_max_evade_level() const {
		return evade_.max_level;
	}

	uint32_t get_base_health() const {
		return health_.base;
	}
	uint32_t get_base_min_attack() const {
		return attack_.base;
	}
	uint32_t get_base_max_attack() const {
		return attack_.maximum;
	}
	uint32_t get_base_defense() const {
		return defense_.base;
	}
	uint32_t get_base_evade() const {
		return evade_.base;
	}

	uint32_t get_health_incr_per_level() const {
		return health_.increase;
	}
	uint32_t get_attack_incr_per_level() const {
		return attack_.increase;
	}
	uint32_t get_defense_incr_per_level() const {
		return defense_.increase;
	}
	uint32_t get_evade_incr_per_level() const {
		return evade_.increase;
	}

	const Image* get_health_level_pic(uint32_t const level) const {
		assert(level <= get_max_health_level());
		return health_.images[level];
	}
	const Image* get_attack_level_pic(uint32_t const level) const {
		assert(level <= get_max_attack_level());
		return attack_.images[level];
	}
	const Image* get_defense_level_pic(uint32_t const level) const {
		assert(level <= get_max_defense_level());
		return defense_.images[level];
	}
	const Image* get_evade_level_pic(uint32_t const level) const {
		assert(level <= get_max_evade_level());
		return evade_.images[level];
	}

	uint32_t get_rand_anim(Game& game, const std::string& name, const Soldier* soldier) const;

	const DirAnimations& get_right_walk_anims(bool const ware, Worker* w) const override;
	uint32_t get_animation(const std::string& anim, const MapObject* mo = nullptr) const override;

protected:
	Bob& create_object() const override;

private:
	// Health, Attack, Defense and Evade values.
	struct BattleAttribute {
		explicit BattleAttribute(std::unique_ptr<LuaTable> table);

		uint32_t base;                     // Base value
		uint32_t maximum;                  // Maximum value for randomizing attack values
		uint32_t increase;                 // Per level increase
		uint32_t max_level;                // Maximum level
		std::vector<const Image*> images;  // Level images
	};

	BattleAttribute health_;
	BattleAttribute attack_;
	BattleAttribute defense_;
	BattleAttribute evade_;

	// Battle animation names
	SoldierAnimationsList attack_success_w_name_;
	SoldierAnimationsList attack_failure_w_name_;
	SoldierAnimationsList evade_success_w_name_;
	SoldierAnimationsList evade_failure_w_name_;
	SoldierAnimationsList die_w_name_;

	SoldierAnimationsList attack_success_e_name_;
	SoldierAnimationsList attack_failure_e_name_;
	SoldierAnimationsList evade_success_e_name_;
	SoldierAnimationsList evade_failure_e_name_;
	SoldierAnimationsList die_e_name_;

	// We can have per-level walking and idle anims
	// NOTE: I expect no soldier will ever agree to carry a ware, so we don't provide animations for
	// that. NOTE: All walking animations are expected to have the same set of ranges.
	SoldierAnimationsList idle_name_;
	std::unordered_map<std::unique_ptr<SoldierLevelRange>, std::map<uint8_t, std::string>>
	   walk_name_;

	// Reads list of animation names from the table and pushes them into result.
	void add_battle_animation(std::unique_ptr<LuaTable> table, SoldierAnimationsList* result);

	DISALLOW_COPY_AND_ASSIGN(SoldierDescr);
};

enum CombatWalkingDir {
	CD_NONE = 0,      // Not in combat
	CD_WALK_W = 1,    // Going to west       (facing west)
	CD_WALK_E = 2,    // Going to east       (facing east)
	CD_COMBAT_W = 3,  // Fighting at west    (facing east!!)
	CD_COMBAT_E = 4,  // Fighting at east    (facing west!!)
	CD_RETURN_W = 5,  // Returning from west (facing east!!)
	CD_RETURN_E = 6,  // Returning from east (facing west!!)
};

enum CombatFlags {
	/// Soldier will wait enemies at his building flag. Only for defenders.
	CF_DEFEND_STAYHOME = 1,
	/// When current health points drop below a fixed percentage, soldier will flee
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
	enum class InfoMode { kWalkingAround, kInBuilding };

	explicit Soldier(const SoldierDescr&);

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

	void set_level(uint32_t health, uint32_t attack, uint32_t defense, uint32_t evade);
	void set_health_level(uint32_t);
	void set_attack_level(uint32_t);
	void set_defense_level(uint32_t);
	void set_evade_level(uint32_t);
	void set_retreat_health(uint32_t);
	unsigned get_level(TrainingAttribute) const;
	unsigned get_health_level() const {
		return health_level_;
	}
	unsigned get_attack_level() const {
		return attack_level_;
	}
	unsigned get_defense_level() const {
		return defense_level_;
	}
	unsigned get_evade_level() const {
		return evade_level_;
	}
	unsigned get_total_level() const {
		return health_level_ + attack_level_ + defense_level_ + evade_level_;
	}

	/// Automatically select a task.
	void init_auto_task(Game&) override;

	Vector2f
	calc_drawpos(const EditorGameBase& game, const Vector2f& field_on_dst, const float scale) const;

	/// Draw this soldier
	void draw(const EditorGameBase&,
	          const InfoToDraw& info_to_draw,
	          const Vector2f& field_on_dst,
	          const Widelands::Coords& coords,
	          float scale,
	          RenderTarget* dst) const override;

	static void calc_info_icon_size(const TribeDescr&, int& w, int& h);

	// Draw the info icon containing health bar and levels. If 'anchor_below' is
	// true, the icon is drawn horizontally centered above Otherwise, the icon
	// is drawn below and right of 'draw_position'.
	void draw_info_icon(Vector2i draw_position,
	                    const float scale,
	                    const InfoMode draw_mode,
	                    const InfoToDraw info_to_draw,
	                    RenderTarget*) const;

	unsigned get_current_health() const {
		return current_health_;
	}
	void set_current_health(const unsigned h) {
		current_health_ = h;
	}

	unsigned get_retreat_health() const {
		return retreat_health_;
	}
	unsigned get_max_health() const;
	unsigned get_min_attack() const;
	unsigned get_max_attack() const;
	unsigned get_defense() const;
	unsigned get_evade() const;

	const Image* get_health_level_pic() const {
		return descr().get_health_level_pic(health_level_);
	}
	const Image* get_attack_level_pic() const {
		return descr().get_attack_level_pic(attack_level_);
	}
	const Image* get_defense_level_pic() const {
		return descr().get_defense_level_pic(defense_level_);
	}
	const Image* get_evade_level_pic() const {
		return descr().get_evade_level_pic(evade_level_);
	}

	int32_t get_training_attribute(TrainingAttribute attr) const override;

	/// Sets a random animation of desired type and start playing it.
	void start_animation(EditorGameBase&, const std::string& animname, const Duration& time);

	/// Heal quantity of health points instantly
	void heal(unsigned);
	void damage(unsigned);  /// Damage quantity of health points

	void log_general_info(const EditorGameBase&) const override;

	bool is_on_battlefield();
	bool is_attacking_player(Game&, Player&);
	Battle* get_battle() const;
	bool can_be_challenged();
	bool check_node_blocked(Game&, const FCoords&, bool commit) override;

	void set_battle(Game&, Battle*);

	void start_task_attack(Game& game, Building&);
	void start_task_defense(Game& game, bool stayhome);
	void start_task_battle(Game&);
	void start_task_move_in_battle(Game&, CombatWalkingDir);
	void start_task_die(Game&);

	std::pair<std::unique_ptr<SoldierLevelRange>, std::unique_ptr<DirAnimations>>&
	get_walking_animations_cache() {
		return walking_animations_cache_;
	}

private:
	void attack_update(Game&, State&);
	void attack_pop(Game&, State&);
	void defense_update(Game&, State&);
	void defense_pop(Game&, State&);
	void battle_update(Game&, State&);
	void battle_pop(Game&, State&);
	void move_in_battle_update(Game&, State&);
	void die_update(Game&, State&);
	void die_pop(Game&, State&);

	void send_space_signals(Game&);
	bool stay_home();

	// Pop the current task or, if challenged, start the fighting task.
	void pop_task_or_fight(Game&);

protected:
	static Task const taskAttack;
	static Task const taskDefense;
	static Task const taskBattle;
	static Task const taskMoveInBattle;
	// May be this can be moved this to bob when finished
	static Task const taskDie;

	bool is_evict_allowed() override;

private:
	unsigned current_health_;
	unsigned health_level_;
	unsigned attack_level_;
	unsigned defense_level_;
	unsigned evade_level_;
	unsigned retreat_health_;

	/// This is used to replicate walk for soldiers but only just before and
	/// just after figthing in a battle, to draw soldier at proper position.
	/// Maybe Bob.walking_ could be used, but then that variable should be
	/// protected instead of private, and some type of rework needed to allow
	/// the new states. I thought that it is cleaner to have this variable
	/// separate.
	CombatWalkingDir combat_walking_;
	Time combat_walkstart_;
	Time combat_walkend_;

	/**
	 * If the soldier is involved in a challenge, it is assigned a battle
	 * object.
	 */
	Battle* battle_;

	std::pair<std::unique_ptr<SoldierLevelRange>, std::unique_ptr<DirAnimations>>
	   walking_animations_cache_;

	/// Number of consecutive blocked signals until the soldiers are considered permanently stuck
	static constexpr uint8_t kBockCountIsStuck = 10;

	// saving and loading
protected:
	struct Loader : public Worker::Loader {
	public:
		Loader();

		void load(FileRead&) override;
		void load_pointers() override;

	protected:
		const Task* get_task(const std::string& name) override;

	private:
		uint32_t battle_;
	};

	Loader* create_loader() override;

public:
	void do_save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_SOLDIER_H
