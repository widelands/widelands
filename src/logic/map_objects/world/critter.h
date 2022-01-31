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

#ifndef WL_LOGIC_MAP_OBJECTS_WORLD_CRITTER_H
#define WL_LOGIC_MAP_OBJECTS_WORLD_CRITTER_H

#include <memory>
#include <set>

#include "base/macros.h"
#include "graphic/animation/diranimations.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/world/critter_program.h"

namespace Widelands {

//
// Description
//
class CritterDescr : public BobDescr {
public:
	CritterDescr(const std::string& init_descname,
	             const LuaTable&,
	             const std::vector<std::string>& attribs);
	~CritterDescr() override = default;

	Bob& create_object() const override;

	bool is_swimming() const;
	uint32_t movecaps() const override;
	const DirAnimations& get_walk_anims() const {
		return walk_anims_;
	}

	bool is_herbivore() const {
		return !food_plants_.empty();
	}
	bool is_carnivore() const {
		return carnivore_;
	}
	uint8_t get_size() const {
		return size_;
	}
	const std::set<uint32_t>& food_plants() const {
		return food_plants_;
	}
	uint8_t get_appetite() const {
		return appetite_;
	}
	uint8_t get_reproduction_rate() const {
		return reproduction_rate_;
	}

	CritterProgram const* get_program(const std::string&) const;

private:
	DirAnimations walk_anims_;
	using Programs = std::map<std::string, std::unique_ptr<const CritterProgram>>;
	Programs programs_;
	const uint8_t size_;
	const bool carnivore_;
	std::set<uint32_t> food_plants_;  // set of immovable attributes
	uint8_t appetite_;  // chance that we feel hungry when we encounter one food item, in %
	const uint8_t reproduction_rate_;  // reproduction adjustment factor, in %
	DISALLOW_COPY_AND_ASSIGN(CritterDescr);
};

class Critter : public Bob {
	friend struct MapBobdataPacket;
	friend struct CritterProgram;

	MO_DESCR(CritterDescr)

public:
	explicit Critter(const CritterDescr&);
	bool init(EditorGameBase&) override;

	void init_auto_task(Game&) override;

	void start_task_program(Game&, const std::string& name);

	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;

	static MapObject::Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);

protected:
	struct Loader : Bob::Loader {
		Loader() = default;

		const Task* get_task(const std::string& name) override;
		const MapObjectProgram* get_program(const std::string& name) override;
	};

private:
	void roam_update(Game&, State&);
	void program_update(Game&, State&);

	bool run_remove(Game&, State&, const CritterAction&);

	static Task const taskRoam;
	static Task const taskProgram;

	Time creation_time_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_WORLD_CRITTER_H
