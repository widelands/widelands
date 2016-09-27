/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_DESCR_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_DESCR_H

#include <memory>

#include "base/macros.h"
#include "graphic/diranimations.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/immovable.h"
#include "scripting/lua_table.h"

class Image;

namespace Widelands {

// TODO(Antonio Trueba#1#): Get rid of forward class declaration
// (chicked-and-egg problem)
class Worker;
struct WorkerProgram;

class WorkerDescr : public BobDescr {
	friend class TribeDescr;
	friend class Warehouse;
	friend struct WorkerProgram;

public:
	using Buildcost = std::map<std::string, Quantity>;

	WorkerDescr(const std::string& init_descname,
	            MapObjectType type,
	            const LuaTable& table,
	            const EditorGameBase& egbase);
	WorkerDescr(const std::string& init_descname, const LuaTable& t, const EditorGameBase& egbase);
	~WorkerDescr() override;

	Bob& create_object() const override;

	bool is_buildable() const {
		return buildable_;
	}
	const Buildcost& buildcost() const {
		assert(is_buildable());
		return buildcost_;
	}

	Point get_ware_hotspot() const {
		return ware_hotspot_;
	}

	/// How much of the worker type that an economy should store in warehouses.
	/// The special value std::numeric_limits<uint32_t>::max() means that the
	/// the target quantity of this ware type will never be checked and should
	/// not be configurable.
	Quantity default_target_quantity() const {
		return default_target_quantity_;
	}

	bool has_demand_check() const {
		return default_target_quantity() != std::numeric_limits<uint32_t>::max();
	}

	/// Called when a demand check for this ware type is encountered during
	/// parsing. If there was no default target quantity set in the ware type's
	/// configuration, set the default value 1.
	void set_has_demand_check() {
		if (default_target_quantity_ == std::numeric_limits<uint32_t>::max())
			default_target_quantity_ = 1;
	}

	std::string helptext_script() const {
		return helptext_script_;
	}

	const DirAnimations& get_walk_anims() const {
		return walk_anims_;
	}
	const DirAnimations& get_right_walk_anims(bool const carries_ware) const {
		return carries_ware ? walkload_anims_ : walk_anims_;
	}
	WorkerProgram const* get_program(const std::string&) const;

	// For leveling
	int32_t get_needed_experience() const {
		return needed_experience_;
	}
	DescriptionIndex becomes() const {
		return becomes_;
	}
	DescriptionIndex worker_index() const;
	bool can_act_as(DescriptionIndex) const;

	// Add a building to the list of employers
	void add_employer(const DescriptionIndex& building_index);

	// The buildings where this worker can work
	const std::set<DescriptionIndex>& employers() const;

	Worker& create(EditorGameBase&, Player&, PlayerImmovable*, Coords) const;

	uint32_t movecaps() const override;

	using Programs = std::map<std::string, std::unique_ptr<WorkerProgram>>;
	const Programs& programs() const {
		return programs_;
	}

protected:
	Point ware_hotspot_;
	Quantity default_target_quantity_;
	std::string helptext_script_;  // The path and filename to the worker's helptext script
	DirAnimations walk_anims_;
	DirAnimations walkload_anims_;
	bool buildable_;
	Buildcost buildcost_;

	/**
	 * Number of experience points required for leveling up,
	 * or INVALID_INDEX if the worker cannot level up.
	 */
	int32_t needed_experience_;

	/**
	 * Type that this worker can become, i.e. level up to (or null).
	 */
	DescriptionIndex becomes_;
	Programs programs_;
	std::set<DescriptionIndex> employers_;  // Buildings where ths worker can work
private:
	const EditorGameBase& egbase_;
	DISALLOW_COPY_AND_ASSIGN(WorkerDescr);
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_DESCR_H
