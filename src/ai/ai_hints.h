/*
 * Copyright (C) 2004-2019 by the Widelands Development Team
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

#ifndef WL_AI_AI_HINTS_H
#define WL_AI_AI_HINTS_H

#include <memory>
#include <unordered_map>

#include "base/log.h"
#include "base/macros.h"
#include "logic/widelands.h"
#include "scripting/lua_table.h"

namespace Widelands {
enum class AiType : uint8_t { kVeryWeak, kWeak, kNormal };
}

/// This struct is used to read out the data given in [aihints] section of a
/// buildings conf file. It is used to tell the computer player about the
/// special properties of a building.
struct BuildingHints {
	explicit BuildingHints(std::unique_ptr<LuaTable>);
	~BuildingHints() {
	}

	std::set<std::string> supported_production() const {
		return supported_production_;
	}

	bool has_mines() const {
		return !mines_.empty();
	}

	char const* get_mines() const {
		return mines_.c_str();
	}

	bool get_needs_water() const {
		return needs_water_;
	}

	bool is_space_consumer() const {
		return space_consumer_;
	}
	bool is_expansion_type() const {
		return expansion_;
	}
	bool is_fighting_type() const {
		return fighting_;
	}
	bool is_mountain_conqueror() const {
		return mountain_conqueror_;
	}

	bool requires_supporters() const {
		return requires_supporters_;
	}

	bool is_shipyard() const {
		return shipyard_;
	}

	bool supports_seafaring() const {
		return supports_seafaring_;
	}

	const std::string& collects_ware_from_map() const {
		return collects_ware_from_map_;
	}

	uint32_t get_prohibited_till() const {
		return prohibited_till_;
	}

	uint32_t basic_amount() const {
		return basic_amount_;
	}

	uint32_t get_forced_after() const {
		return forced_after_;
	}

	uint8_t get_mines_percent() const {
		return mines_percent_;
	}

	int16_t get_ai_limit(Widelands::AiType) const;

	void set_trainingsites_max_percent(int percent);

	uint8_t trainingsites_max_percent() const;

private:
	const std::string mines_;
	const bool needs_water_;
	const bool space_consumer_;
	const bool expansion_;
	const bool fighting_;
	const bool mountain_conqueror_;
	const bool shipyard_;
	const bool supports_seafaring_;
	const std::string collects_ware_from_map_;
	const int32_t prohibited_till_;
	const uint32_t basic_amount_;
	const int32_t forced_after_;
	const int8_t mines_percent_;
	const int16_t very_weak_ai_limit_;
	const int16_t weak_ai_limit_;
	const int16_t normal_ai_limit_;
	const bool requires_supporters_;
	int trainingsites_max_percent_;
	std::set<std::string> supported_production_;

	DISALLOW_COPY_AND_ASSIGN(BuildingHints);
};

/// Hints common to wares and workers
struct WareWorkerHints {
	WareWorkerHints() = default;

	/// Returns the preciousness of the ware/worker, or kInvalidWare if the tribe doesn't use the
	/// ware/worker or the worker has no preciousness defined for the tribe.
	int preciousness(const std::string& tribename) const;

protected:
	void read_preciousness(const std::string& name, const LuaTable& table);

private:
	// tribename, preciousness. No default.
	std::unordered_map<std::string, int> preciousnesses_;
};

/// Hints for wares
struct WareHints : WareWorkerHints {
	explicit WareHints(const std::string& ware_name, const LuaTable& table);
};

/// Hints for workers
struct WorkerHints : WareWorkerHints {
	explicit WorkerHints(const std::string& worker_name, const LuaTable& table);
};

#endif  // end of include guard: WL_AI_AI_HINTS_H
