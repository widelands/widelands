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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_WARE_DESCR_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_WARE_DESCR_H

#include <memory>

#include "ai/ai_hints.h"
#include "base/macros.h"
#include "logic/map_objects/map_object.h"
#include "scripting/lua_table.h"

constexpr int kWareMenuPicWidth = 24;   //!< Default width for ware's menu icons
constexpr int kWareMenuPicHeight = 24;  //!< Default height for ware's menu icons

namespace Widelands {

/**
 * Wares can be stored in warehouses. They can be transferred across an
 * Economy. They can be traded.
 */
class WareDescr : public MapObjectDescr {
public:
	WareDescr(const std::string& init_descname, const LuaTable& t);
	~WareDescr() override {
	}

	/// AI hints for this ware type
	const AI::WareWorkerHints& ai_hints() const {
		return *ai_hints_;
	}

	/// How much of the ware type an economy should store in warehouses.
	/// The special value kInvalidWare means that the target quantity of this ware type will never be
	/// checked
	///  and should not be configurable.
	Quantity default_target_quantity(const std::string& tribename) const;
	/// Sets the default target quantity for the given tribe. Overwrites if it already exists.
	void set_default_target_quantity(const std::string& tribename, int quantity);

	/// This is an AI hint
	void set_preciousness(const std::string& tribename, int preciousness);

	bool has_demand_check(const std::string& tribename) const;

	/// Called when a demand check for this ware type is encountered during
	/// parsing. If there was no default target quantity set in the ware type's
	/// configuration for the 'tribename', sets the default value to 1.
	void set_has_demand_check(const std::string& tribename);

	// Add a building to the list of consumers
	void add_consumer(const DescriptionIndex& building_index);
	// Add a building to the list of producers
	void add_producer(const DescriptionIndex& building_index);

	// The buildings that consume this ware
	const std::set<DescriptionIndex>& consumers() const;
	// The buildings that produce this ware
	const std::set<DescriptionIndex>& producers() const;

private:
	// tribename, quantity. No default.
	std::unordered_map<std::string, Quantity> default_target_quantities_;

	// Hints for the AI
	std::unique_ptr<AI::WareWorkerHints> ai_hints_;

	std::set<DescriptionIndex> consumers_;  // Buildings that consume this ware
	std::set<DescriptionIndex> producers_;  // Buildings that produce this ware
	DISALLOW_COPY_AND_ASSIGN(WareDescr);
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WARE_DESCR_H
