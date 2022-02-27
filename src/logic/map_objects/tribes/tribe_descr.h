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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBE_DESCR_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBE_DESCR_H

#include <memory>

#include "base/macros.h"
#include "graphic/animation/animation.h"
#include "graphic/toolbar_imageset.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/road_textures.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/widelands.h"

namespace Widelands {

/*
 * Resource indicators:
 * A ResourceIndicatorSet maps the resource name to a ResourceIndicatorList.
 * A ResourceIndicatorList maps resource amounts to the DescriptionIndex of an immovable this player
 * uses.
 * Special case: The ResourceIndicatorList mapped to "" contains resis that will be used in
 * locations
 * without resources. If it has several entries, result is arbitrary.
 */
using ResourceIndicatorList = std::map<uint32_t, DescriptionIndex>;
using ResourceIndicatorSet = std::map<std::string, ResourceIndicatorList>;

/*
Tribes
------

Every player chooses a tribe. A tribe has distinct properties such as the
buildings it can build and the associated graphics.
Two players can choose the same tribe.
*/
class TribeDescr {
public:
	TribeDescr(const Widelands::TribeBasicInfo& info,
	           Descriptions& descriptions,
	           const LuaTable& table,
	           const LuaTable* scenario_table = nullptr);

	const std::string& name() const;
	const std::string& descname() const;
	const std::string& military_capacity_script() const;

	size_t get_nrwares() const;
	size_t get_nrworkers() const;

	const std::set<DescriptionIndex>& buildings() const;
	const std::set<DescriptionIndex>& wares() const;
	const std::set<DescriptionIndex>& workers() const;
	const std::set<DescriptionIndex>& immovables() const;
	const ResourceIndicatorSet& resource_indicators() const;

	std::set<DescriptionIndex>& mutable_wares();
	std::set<DescriptionIndex>& mutable_workers();
	std::set<DescriptionIndex>& mutable_buildings();
	std::set<DescriptionIndex>& mutable_immovables();

	bool has_building(const DescriptionIndex& index) const;
	bool has_ware(const DescriptionIndex& index) const;
	bool has_worker(const DescriptionIndex& index) const;
	bool has_immovable(const DescriptionIndex& index) const;

	// A ware is a construction material if it appears in a building's buildcost or enhancement cost
	bool is_construction_material(const DescriptionIndex& ware_index) const;

	DescriptionIndex building_index(const std::string& buildingname) const;
	DescriptionIndex immovable_index(const std::string& immovablename) const;
	DescriptionIndex ware_index(const std::string& warename) const;
	DescriptionIndex worker_index(const std::string& workername) const;

	/// Return the given building or die trying
	DescriptionIndex safe_building_index(const std::string& buildingname) const;
	/// Return the given ware or die trying
	DescriptionIndex safe_ware_index(const std::string& warename) const;
	/// Return the given worker or die trying
	DescriptionIndex safe_worker_index(const std::string& workername) const;

	BuildingDescr const* get_building_descr(const DescriptionIndex& index) const;
	ImmovableDescr const* get_immovable_descr(const DescriptionIndex& index) const;
	WareDescr const* get_ware_descr(const DescriptionIndex& index) const;
	WorkerDescr const* get_worker_descr(const DescriptionIndex& index) const;

	DescriptionIndex builder() const;
	DescriptionIndex geologist() const;
	DescriptionIndex scouts_house() const;
	DescriptionIndex soldier() const;
	DescriptionIndex ship() const;
	DescriptionIndex ferry() const;
	DescriptionIndex port() const;
	const std::vector<DescriptionIndex>& carriers() const {
		return carriers_;
	}

	const std::vector<DescriptionIndex>& trainingsites() const;
	const std::vector<DescriptionIndex>& worker_types_without_cost() const;

	uint32_t frontier_animation() const;
	uint32_t flag_animation() const;
	uint32_t bridge_animation(uint8_t dir, bool busy) const;

	// Bridge height in pixels at 1x scale, for drawing bobs walking over a bridge
	uint32_t bridge_height() const;

	// The road textures used for drawing roads and waterways.
	const RoadTextures& road_textures() const;

	DescriptionIndex get_resource_indicator(const ResourceDescription* const res,
	                                        const ResourceAmount amount) const;

	// Returns the initalization at 'index' (which must not be out of bounds).
	const Widelands::TribeBasicInfo::Initialization& initialization(const uint8_t index) const {
		return basic_info_.initializations.at(index);
	}

	const Widelands::TribeBasicInfo& basic_info() const {
		return basic_info_;
	}

	using WaresOrder = std::vector<std::vector<Widelands::DescriptionIndex>>;
	const WaresOrder& wares_order() const {
		return wares_order_;
	}
	WaresOrder& mutable_wares_order() {
		return wares_order_;
	}

	const WaresOrder& workers_order() const {
		return workers_order_;
	}
	WaresOrder& mutable_workers_order() {
		return workers_order_;
	}

	bool uses_resource(const std::string& name) const {
		return used_resources_.count(name);
	}
	// Warning: Do not use pointer arithmetics in logic code!
	const std::set<const BuildingDescr*>& buildings_built_over_immovables() const {
		return buildings_built_over_immovables_;
	}

	const std::vector<std::pair<std::string, int>>& collectors_points_table() const {
		return collectors_points_table_;
	}

	const std::string& get_productionsite_worker_missing_string() const {
		return productionsite_worker_missing_;
	}
	const std::string& get_productionsite_worker_coming_string() const {
		return productionsite_worker_coming_;
	}
	const std::string& get_productionsite_workers_missing_string() const {
		return productionsite_workers_missing_;
	}
	const std::string& get_productionsite_workers_coming_string() const {
		return productionsite_workers_coming_;
	}

	// The custom toolbar imageset if any. Can be nullptr.
	ToolbarImageset* toolbar_image_set() const;

	const std::map<std::string /* key */, std::string /* building */>& fastplace_defaults() const {
		return fastplace_defaults_;
	}

	// Read helptext from Lua table
	void load_helptexts(MapObjectDescr* descr, const LuaTable& table) const;

	// Make sure that everything is there and that dependencies are calculated.
	// This needs to be called exactly once during postloading.
	void finalize_loading(Descriptions& descriptions);

private:
	// Helper functions for loading everything in the constructor
	void load_frontiers_flags_roads(const LuaTable& table);
	void load_ships(const LuaTable& table, Descriptions& descriptions);
	void load_wares(const LuaTable& table, Descriptions& descriptions);
	void load_immovables(const LuaTable& table, Descriptions& descriptions);
	void load_workers(const LuaTable& table, Descriptions& descriptions);
	void load_buildings(const LuaTable& table, Descriptions& descriptions);

	// Helper function for adding a special worker type (carriers etc.)
	DescriptionIndex add_special_worker(const std::string& workername,
	                                    Descriptions& descriptions) const;
	// Helper function for adding a special building type (port etc.)
	DescriptionIndex add_special_building(const std::string& buildingname,
	                                      Descriptions& descriptions) const;
	// Helper function to calculate trainingsites proportions for the AI
	void calculate_trainingsites_proportions(const Descriptions& descriptions) const;

	void process_productionsites(Descriptions& descriptions);

	const std::string name_;
	const std::string descname_;
	const std::string military_capacity_script_;
	const Descriptions& descriptions_;

	uint32_t frontier_animation_id_;
	uint32_t flag_animation_id_;
	struct BridgeAnimationIDs {
		uint32_t e;
		uint32_t se;
		uint32_t sw;
	};
	BridgeAnimationIDs bridges_normal_;
	BridgeAnimationIDs bridges_busy_;
	uint32_t bridge_height_;

	// A container of all texture images that can be used for drawing a
	// (normal|busy) road or a waterway. The images are guaranteed to exist.
	RoadTextures road_textures_;

	std::set<DescriptionIndex> buildings_;
	std::set<DescriptionIndex> immovables_;  // The player immovables
	std::set<DescriptionIndex> workers_;
	std::set<DescriptionIndex> wares_;
	std::set<const BuildingDescr*> buildings_built_over_immovables_;
	std::set<std::string> used_resources_;
	ResourceIndicatorSet resource_indicators_;
	// The wares that are used by construction sites
	std::set<DescriptionIndex> construction_materials_;
	// Special units. Some of them are used by the engine, some are only used by the AI.
	DescriptionIndex builder_;                // The builder for this tribe
	std::vector<DescriptionIndex> carriers_;  // The basic and advanced carriers for this tribe
	DescriptionIndex geologist_;              // This tribe's geologist worker
	DescriptionIndex soldier_;                // The soldier that this tribe uses
	DescriptionIndex ship_;                   // The ship that this tribe uses
	DescriptionIndex ferry_;                  // The ferry that this tribe uses
	DescriptionIndex port_;                   // The port that this tribe uses
	DescriptionIndex scouts_house_;           // This tribe's scout's house/hut
	std::vector<DescriptionIndex> worker_types_without_cost_;
	std::vector<DescriptionIndex> trainingsites_;
	// Order and positioning of wares in the warehouse display
	WaresOrder wares_order_;
	WaresOrder workers_order_;
	std::vector<std::pair<std::string, int>> collectors_points_table_;

	std::string productionsite_worker_missing_, productionsite_workers_missing_,
	   productionsite_worker_coming_, productionsite_workers_coming_;

	// An optional custom imageset for the in-game menu toolbar
	std::unique_ptr<ToolbarImageset> toolbar_image_set_;

	std::map<std::string, std::string> fastplace_defaults_;

	Widelands::TribeBasicInfo basic_info_;

	DISALLOW_COPY_AND_ASSIGN(TribeDescr);
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBE_DESCR_H
