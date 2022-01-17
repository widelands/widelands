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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/map_objects/tribes/worker_descr.h"

#include <memory>

#include "base/i18n.h"
#include "base/vector.h"
#include "base/wexception.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/tribes/worker_program.h"

namespace Widelands {

WorkerDescr::WorkerDescr(const std::string& init_descname,
                         MapObjectType init_type,
                         const LuaTable& table,
                         Descriptions& descriptions)
   : BobDescr(init_descname, init_type, MapObjectDescr::OwnerType::kTribe, table),
     ware_hotspot_(table.has_key("ware_hotspot") ?
                      table.get_vector<std::string, int>("ware_hotspot") :
                      Vector2i(0, 15)),
     default_target_quantity_(kInvalidWare),
     buildable_(table.has_key("buildcost")),
     // Read what the worker can become and the needed experience. If one of the keys is there, the
     // other key must be there too. So, we cross the checks to trigger an exception if this is
     // violated.
     becomes_(INVALID_INDEX),
     needed_experience_(table.has_key("becomes") ? table.get_int("experience") : INVALID_INDEX),
     ai_hints_(new AI::WareWorkerHints()),
     descriptions_(descriptions) {
	if (table.has_key("experience")) {
		// This also checks that the other worker's type matches ours
		set_becomes(descriptions, table.get_string("becomes"));
	}

	if (icon_filename().empty()) {
		throw GameDataError("Worker %s has no menu icon", name().c_str());
	}

	std::unique_ptr<LuaTable> items_table;

	if (table.has_key("buildcost")) {
		items_table = table.get_table("buildcost");
		for (const std::string& key : items_table->keys<std::string>()) {
			try {
				// Check if ware/worker exists already and if not, try to load it. Will throw a
				// GameDataError on failure.
				descriptions.load_ware_or_worker(key);
				const int32_t value = items_table->get_int(key);
				if (value < 1) {
					throw GameDataError("Buildcost: Ware/Worker count needs to be > 0 in "
					                    "\"%s=%d\".\nEmpty buildcost tables are allowed if you wish to "
					                    "have an amount of 0.",
					                    key.c_str(), value);
				}
				if (value > 255) {
					throw GameDataError("Buildcost: Ware/Worker count needs to be <= 255 in \"%s=%d\".",
					                    key.c_str(), value);
				}

				buildcost_.insert(std::make_pair(key, value));
			} catch (const std::exception& e) {
				throw GameDataError("[buildcost] \"%s\": %s", key.c_str(), e.what());
			}
		}
	}

	// Read the walking animations
	assign_directional_animation(&walk_anims_, "walk");

	// Not all workers need a special walkload animation - for some the walk animation looks good
	// enough.
	if (is_animation_known("walkload_e")) {
		assign_directional_animation(&walkload_anims_, "walkload");
	} else {
		assign_directional_animation(&walkload_anims_, "walk");
	}

	// Read programs
	if (table.has_key("programs")) {
		std::unique_ptr<LuaTable> programs_table = table.get_table("programs");
		for (std::string program_name : programs_table->keys<std::string>()) {
			program_name = to_lower(program_name);
			if (programs_.count(program_name)) {
				throw GameDataError("Program '%s' has already been declared for worker '%s'",
				                    program_name.c_str(), name().c_str());
			}
			try {
				programs_[program_name] = std::unique_ptr<WorkerProgram>(new WorkerProgram(
				   program_name, *programs_table->get_table(program_name), *this, descriptions));
			} catch (const std::exception& e) {
				throw GameDataError("%s: Error in worker program %s: %s", name().c_str(),
				                    program_name.c_str(), e.what());
			}
		}
	}
}

WorkerDescr::WorkerDescr(const std::string& init_descname,
                         const LuaTable& table,
                         Descriptions& descriptions)
   : WorkerDescr(init_descname, MapObjectType::WORKER, table, descriptions) {
}

WorkerDescr::~WorkerDescr() {  // NOLINT
	                            // WorkerProgram needs this
}

void WorkerDescr::set_becomes(Descriptions& d, const std::string& become) {
	becomes_ = d.load_worker(become);
	assert(becomes_ != INVALID_INDEX);

	const WorkerDescr* other = d.get_worker_descr(becomes_);
	if (type() != other->type()) {
		throw GameDataError("Worker %s of type %s can not level up to worker %s of type %s",
		                    name().c_str(), to_string(type()).c_str(), become.c_str(),
		                    to_string(other->type()).c_str());
	}
}

void WorkerDescr::set_default_target_quantity(int quantity) {
	if (quantity < 0) {
		throw GameDataError(
		   "default_target_quantity %d for worker '%s' must be >=0", quantity, name().c_str());
	}
	default_target_quantity_ = quantity;
}

void WorkerDescr::set_preciousness(const std::string& tribename, int preciousness) {
	ai_hints_->set_preciousness(name(), tribename, preciousness);
}

/**
 * Get a program from the workers description.
 */
WorkerProgram const* WorkerDescr::get_program(const std::string& programname) const {
	Programs::const_iterator it = programs_.find(programname);

	if (it == programs_.end()) {
		throw GameDataError("%s has no program '%s'", name().c_str(), programname.c_str());
	}

	return it->second.get();
}

/**
 * Custom creation routing that accounts for the location.
 */
Worker& WorkerDescr::create(EditorGameBase& egbase,
                            Player* owner,
                            PlayerImmovable* const location,
                            Coords const coords) const {
	Worker& worker = dynamic_cast<Worker&>(create_object());
	worker.set_owner(owner);
	worker.set_location(location);
	worker.set_position(egbase, coords);
	worker.init(egbase);
	return worker;
}

uint32_t WorkerDescr::movecaps() const {
	return MOVECAPS_WALK;
}

/**
 * Create a generic worker of this type.
 */
Bob& WorkerDescr::create_object() const {
	return *new Worker(*this);
}

/**
 * check if worker can be substitute for a requested worker type
 */
bool WorkerDescr::can_act_as(DescriptionIndex const index) const {
	assert(descriptions_.worker_exists(index));
	if (index == worker_index()) {
		return true;
	}

	// if requested worker type can be promoted, compare with that type
	const WorkerDescr& descr = *descriptions_.get_worker_descr(index);
	DescriptionIndex const becomes_index = descr.becomes();
	return becomes_index != INVALID_INDEX ? can_act_as(becomes_index) : false;
}

DescriptionIndex WorkerDescr::worker_index() const {
	return descriptions_.worker_index(name());
}

void WorkerDescr::add_employer(const DescriptionIndex& building_index) {
	employers_.insert(building_index);
}

const std::set<DescriptionIndex>& WorkerDescr::employers() const {
	return employers_;
}

}  // namespace Widelands
