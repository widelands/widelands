/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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
#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_BATTLE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_BATTLE_H

#include "logic/map_objects/map_object.h"

namespace Widelands {
class Soldier;

class BattleDescr : public MapObjectDescr {
public:
	BattleDescr(char const* const init_name, char const* const init_descname)
	   : MapObjectDescr(MapObjectType::BATTLE, init_name, init_descname) {
	}
	~BattleDescr() override = default;

private:
	DISALLOW_COPY_AND_ASSIGN(BattleDescr);
};

/**
 * Manages the battle between two opposing soldiers.
 *
 * A \ref Battle object is created using the \ref create() function as soon as
 * a soldier decides he wants to attack someone else. A battle always has both
 * Soldiers defined, the battle object must be destroyed as soon as there is no
 * other Soldier to battle anymore.
 */
class Battle : public MapObject {
public:
	const BattleDescr& descr() const;

	Battle();                           //  for loading an existing battle from a savegame
	Battle(Game&, Soldier*, Soldier*);  //  to create a new battle in the game

	// Implements MapObject.
	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	bool has_new_save_support() override {
		return true;
	}
	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
	static MapObject::Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);

	// Cancel this battle immediately and schedule destruction.
	void cancel(Game&, Soldier&);

	// Returns true if the battle should not be interrupted.
	bool locked(const Game&);

	// The two soldiers involved in this fight.
	Soldier* first() {
		return first_;
	}
	Soldier* second() {
		return second_;
	}

	uint32_t get_pending_damage(const Soldier* for_whom) const;

	// Returns the other soldier involved in this battle. CHECKs that the given
	// soldier is participating in this battle. Can return nullptr, probably when the
	// opponent has died.
	Soldier* opponent(const Soldier&) const;

	// Called by the battling soldiers once they've met on a common node and are
	// idle.
	void get_battle_work(Game&, Soldier&);

private:
	struct Loader : public MapObject::Loader {
		virtual void load(FileRead&);
		void load_pointers() override;

		Serial first_;
		Serial second_;
	};

	void calculate_round(Game&);

	Soldier* first_ = nullptr;
	Soldier* second_ = nullptr;

	/**
	 * Gametime when the battle was created.
	 */
	Time creationtime_{0U};

	/**
	 * 1 if only the first soldier is ready, 2 if only the second soldier
	 * is ready, 3 if both are ready.
	 */
	uint8_t readyflags_{0U};

	/**
	 * Damage pending to apply. Damage is applied at end of round so animations
	 * can show current action.
	 */
	uint32_t damage_{0U};

	/**
	 * \c true if the first soldier is the next to strike.
	 */
	bool first_strikes_{true};

	/**
	 * \c true if the last turn attacker damaged his opponent
	 */
	bool last_attack_hits_{false};
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_BATTLE_H
