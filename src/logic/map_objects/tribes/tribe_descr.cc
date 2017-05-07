/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "logic/map_objects/tribes/tribe_descr.h"

#include <algorithm>
#include <iostream>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

namespace Widelands {

/**
  * The contents of 'table' are documented in
  * /data/tribes/atlanteans.lua
  */
TribeDescr::TribeDescr(const LuaTable& table, const TribeBasicInfo& info, const Tribes& init_tribes)
   : name_(table.get_string("name")), descname_(info.descname), tribes_(init_tribes) {

	try {
		initializations_ = info.initializations;

		std::unique_ptr<LuaTable> items_table = table.get_table("animations");
		frontier_animation_id_ = g_gr->animations().load(*items_table->get_table("frontier"));
		flag_animation_id_ = g_gr->animations().load(*items_table->get_table("flag"));

		items_table = table.get_table("roads");
		const auto load_roads = [&items_table, this](
		   const std::string& road_type, std::vector<std::string>* images) {
			std::vector<std::string> roads =
			   items_table->get_table(road_type)->array_entries<std::string>();
			for (const std::string& filename : roads) {
				if (g_fs->file_exists(filename)) {
					images->push_back(filename);
				} else {
					throw GameDataError("File '%s' for %s road texture doesn't exist", filename.c_str(),
					                    road_type.c_str());
				}
			}
			if (images->empty()) {
				throw GameDataError("Tribe has no %s roads.", road_type.c_str());
			}
		};
		load_roads("normal", &normal_road_paths_);
		load_roads("busy", &busy_road_paths_);

		items_table = table.get_table("wares_order");
		wares_order_coords_.resize(tribes_.nrwares());
		int columnindex = 0;
		for (const int key : items_table->keys<int>()) {
			std::vector<DescriptionIndex> column;
			std::vector<std::string> warenames =
			   items_table->get_table(key)->array_entries<std::string>();
			for (size_t rowindex = 0; rowindex < warenames.size(); ++rowindex) {
				try {
					DescriptionIndex wareindex = tribes_.safe_ware_index(warenames[rowindex]);
					if (has_ware(wareindex)) {
						throw GameDataError(
						   "Duplicate definition of ware '%s'", warenames[rowindex].c_str());
					}
					wares_.insert(wareindex);
					column.push_back(wareindex);
					wares_order_coords_[wareindex] = std::make_pair(columnindex, rowindex);
				} catch (const WException& e) {
					throw GameDataError(
					   "Failed adding ware '%s: %s", warenames[rowindex].c_str(), e.what());
				}
			}
			if (!column.empty()) {
				wares_order_.push_back(column);
				++columnindex;
			}
		}

		items_table = table.get_table("workers_order");
		workers_order_coords_.resize(tribes_.nrworkers());
		columnindex = 0;
		for (const int key : items_table->keys<int>()) {
			std::vector<DescriptionIndex> column;
			std::vector<std::string> workernames =
			   items_table->get_table(key)->array_entries<std::string>();
			for (size_t rowindex = 0; rowindex < workernames.size(); ++rowindex) {
				try {
					DescriptionIndex workerindex = tribes_.safe_worker_index(workernames[rowindex]);
					if (has_worker(workerindex)) {
						throw GameDataError(
						   "Duplicate definition of worker '%s'", workernames[rowindex].c_str());
					}
					workers_.insert(workerindex);
					column.push_back(workerindex);
					workers_order_coords_[workerindex] = std::make_pair(columnindex, rowindex);

					const WorkerDescr& worker_descr = *tribes_.get_worker_descr(workerindex);
					if (worker_descr.is_buildable() && worker_descr.buildcost().empty()) {
						worker_types_without_cost_.push_back(workerindex);
					}
				} catch (const WException& e) {
					throw GameDataError(
					   "Failed adding worker '%s: %s", workernames[rowindex].c_str(), e.what());
				}
			}
			if (!column.empty()) {
				workers_order_.push_back(column);
				++columnindex;
			}
		}

		std::vector<std::string> immovables =
		   table.get_table("immovables")->array_entries<std::string>();
		for (const std::string& immovablename : immovables) {
			try {
				DescriptionIndex index = tribes_.safe_immovable_index(immovablename);
				if (immovables_.count(index) == 1) {
					throw GameDataError("Duplicate definition of immovable '%s'", immovablename.c_str());
				}
				immovables_.insert(index);
			} catch (const WException& e) {
				throw GameDataError(
				   "Failed adding immovable '%s': %s", immovablename.c_str(), e.what());
			}
		}

		ship_names_ = table.get_table("ship_names")->array_entries<std::string>();

		for (const std::string& buildingname :
		     table.get_table("buildings")->array_entries<std::string>()) {
			try {
				DescriptionIndex index = tribes_.safe_building_index(buildingname);
				if (has_building(index)) {
					throw GameDataError("Duplicate definition of building '%s'", buildingname.c_str());
				}
				buildings_.push_back(index);

				// Register construction materials
				for (const auto& build_cost : get_building_descr(index)->buildcost()) {
					if (!is_construction_material(build_cost.first)) {
						construction_materials_.insert(build_cost.first);
					}
				}
				for (const auto& enhancement_cost : get_building_descr(index)->enhancement_cost()) {
					if (!is_construction_material(enhancement_cost.first)) {
						construction_materials_.insert(enhancement_cost.first);
					}
				}
			} catch (const WException& e) {
				throw GameDataError("Failed adding building '%s': %s", buildingname.c_str(), e.what());
			}
		}

		// Special types
		builder_ = add_special_worker(table.get_string("builder"));
		carrier_ = add_special_worker(table.get_string("carrier"));
		carrier2_ = add_special_worker(table.get_string("carrier2"));
		geologist_ = add_special_worker(table.get_string("geologist"));
		soldier_ = add_special_worker(table.get_string("soldier"));

		const std::string shipname = table.get_string("ship");
		try {
			ship_ = tribes_.safe_ship_index(shipname);
		} catch (const WException& e) {
			throw GameDataError("Failed adding ship '%s': %s", shipname.c_str(), e.what());
		}

		headquarters_ = add_special_building(table.get_string("headquarters"));
		port_ = add_special_building(table.get_string("port"));
		barracks_ = add_special_building(table.get_string("barracks"));

	} catch (const GameDataError& e) {
		throw GameDataError("tribe %s: %s", name_.c_str(), e.what());
	}
}

/**
  * Access functions
  */

const std::string& TribeDescr::name() const {
	return name_;
}
const std::string& TribeDescr::descname() const {
	return descname_;
}

size_t TribeDescr::get_nrbuildings() const {
	return buildings_.size();
}
size_t TribeDescr::get_nrwares() const {
	return wares_.size();
}
size_t TribeDescr::get_nrworkers() const {
	return workers_.size();
}

const std::vector<DescriptionIndex>& TribeDescr::buildings() const {
	return buildings_;
}
const std::set<DescriptionIndex>& TribeDescr::wares() const {
	return wares_;
}
const std::set<DescriptionIndex>& TribeDescr::workers() const {
	return workers_;
}

bool TribeDescr::has_building(const DescriptionIndex& index) const {
	return std::find(buildings_.begin(), buildings_.end(), index) != buildings_.end();
}
bool TribeDescr::has_ware(const DescriptionIndex& index) const {
	return wares_.count(index) == 1;
}
bool TribeDescr::has_worker(const DescriptionIndex& index) const {
	return workers_.count(index) == 1;
}
bool TribeDescr::has_immovable(const DescriptionIndex& index) const {
	return immovables_.count(index) == 1;
}
bool TribeDescr::is_construction_material(const DescriptionIndex& index) const {
	return construction_materials_.count(index) == 1;
}

DescriptionIndex TribeDescr::building_index(const std::string& buildingname) const {
	return tribes_.building_index(buildingname);
}

DescriptionIndex TribeDescr::immovable_index(const std::string& immovablename) const {
	return tribes_.immovable_index(immovablename);
}
DescriptionIndex TribeDescr::ware_index(const std::string& warename) const {
	return tribes_.ware_index(warename);
}
DescriptionIndex TribeDescr::worker_index(const std::string& workername) const {
	return tribes_.worker_index(workername);
}

DescriptionIndex TribeDescr::safe_building_index(const std::string& buildingname) const {
	return tribes_.safe_building_index(buildingname);
}

DescriptionIndex TribeDescr::safe_ware_index(const std::string& warename) const {
	return tribes_.safe_ware_index(warename);
}
DescriptionIndex TribeDescr::safe_worker_index(const std::string& workername) const {
	return tribes_.safe_worker_index(workername);
}

WareDescr const* TribeDescr::get_ware_descr(const DescriptionIndex& index) const {
	return tribes_.get_ware_descr(index);
}
WorkerDescr const* TribeDescr::get_worker_descr(const DescriptionIndex& index) const {
	return tribes_.get_worker_descr(index);
}

BuildingDescr const* TribeDescr::get_building_descr(const DescriptionIndex& index) const {
	return tribes_.get_building_descr(index);
}
ImmovableDescr const* TribeDescr::get_immovable_descr(const DescriptionIndex& index) const {
	return tribes_.get_immovable_descr(index);
}

DescriptionIndex TribeDescr::builder() const {
	assert(tribes_.worker_exists(builder_));
	return builder_;
}
DescriptionIndex TribeDescr::carrier() const {
	assert(tribes_.worker_exists(carrier_));
	return carrier_;
}
DescriptionIndex TribeDescr::carrier2() const {
	assert(tribes_.worker_exists(carrier2_));
	return carrier2_;
}
DescriptionIndex TribeDescr::geologist() const {
	assert(tribes_.worker_exists(geologist_));
	return geologist_;
}
DescriptionIndex TribeDescr::soldier() const {
	assert(tribes_.worker_exists(soldier_));
	return soldier_;
}
DescriptionIndex TribeDescr::ship() const {
	assert(tribes_.ship_exists(ship_));
	return ship_;
}
DescriptionIndex TribeDescr::headquarters() const {
	assert(tribes_.building_exists(headquarters_));
	return headquarters_;
}
DescriptionIndex TribeDescr::port() const {
	assert(tribes_.building_exists(port_));
	return port_;
}
DescriptionIndex TribeDescr::barracks() const {
	assert(tribes_.building_exists(barracks_));
	return barracks_;
}
const std::vector<DescriptionIndex>& TribeDescr::worker_types_without_cost() const {
	return worker_types_without_cost_;
}

uint32_t TribeDescr::frontier_animation() const {
	return frontier_animation_id_;
}

uint32_t TribeDescr::flag_animation() const {
	return flag_animation_id_;
}

const std::vector<std::string>& TribeDescr::normal_road_paths() const {
	return normal_road_paths_;
}

const std::vector<std::string>& TribeDescr::busy_road_paths() const {
	return busy_road_paths_;
}

void TribeDescr::add_normal_road_texture(const Image* texture) {
	road_textures_.add_normal_road_texture(texture);
}

void TribeDescr::add_busy_road_texture(const Image* texture) {
	road_textures_.add_busy_road_texture(texture);
}

const RoadTextures& TribeDescr::road_textures() const {
	return road_textures_;
}

/*
==============
Find the best matching indicator for the given amount.
==============
*/
DescriptionIndex TribeDescr::get_resource_indicator(ResourceDescription const* const res,
                                                    const ResourceAmount amount) const {
	if (!res || !amount) {
		DescriptionIndex idx = immovable_index("resi_none");
		if (!has_immovable(idx)) {
			throw GameDataError("There is no resource indicator for resi_none!");
		}
		return idx;
	}

	int32_t i = 1;
	int32_t num_indicators = 0;
	for (;;) {
		const std::string resi_filename =
		   (boost::format("resi_%s%i") % res->name().c_str() % i).str();
		if (!has_immovable(immovable_index(resi_filename))) {
			break;
		}
		++i;
		++num_indicators;
	}

	if (!num_indicators) {
		throw GameDataError("There is no resource indicator for resource %s", res->name().c_str());
	}

	int32_t bestmatch =
	   static_cast<int32_t>((static_cast<float>(amount) / res->max_amount()) * num_indicators);
	if (bestmatch > num_indicators) {
		throw GameDataError("Amount of %s is %i but max amount is %i", res->name().c_str(),
		                    static_cast<unsigned int>(amount),
		                    static_cast<unsigned int>(res->max_amount()));
	}
	if (amount < res->max_amount()) {
		bestmatch += 1;  // Resi start with 1, not 0
	}

	return immovable_index((boost::format("resi_%s%i") % res->name().c_str() % bestmatch).str());
}

void TribeDescr::resize_ware_orders(size_t maxLength) {
	bool need_resize = false;

	// Check if we actually need to resize.
	for (WaresOrder::iterator it = wares_order_.begin(); it != wares_order_.end(); ++it) {
		if (it->size() > maxLength) {
			need_resize = true;
		}
	}

	// Build new smaller wares_order.
	if (need_resize) {
		WaresOrder new_wares_order;
		for (WaresOrder::iterator it = wares_order_.begin(); it != wares_order_.end(); ++it) {
			new_wares_order.push_back(std::vector<Widelands::DescriptionIndex>());
			for (std::vector<Widelands::DescriptionIndex>::iterator it2 = it->begin();
			     it2 != it->end(); ++it2) {
				if (new_wares_order.rbegin()->size() >= maxLength) {
					new_wares_order.push_back(std::vector<Widelands::DescriptionIndex>());
				}
				new_wares_order.rbegin()->push_back(*it2);
				wares_order_coords_[*it2].first = new_wares_order.size() - 1;
				wares_order_coords_[*it2].second = new_wares_order.rbegin()->size() - 1;
			}
		}

		// Remove old array.
		wares_order_.clear();
		wares_order_ = new_wares_order;
	}
}

/**
  * Helper functions
  */

DescriptionIndex TribeDescr::add_special_worker(const std::string& workername) {
	try {
		DescriptionIndex worker = tribes_.safe_worker_index(workername);
		if (!has_worker(worker)) {
			throw GameDataError("This tribe doesn't have the worker '%s'", workername.c_str());
		}
		return worker;
	} catch (const WException& e) {
		throw GameDataError("Failed adding special worker '%s': %s", workername.c_str(), e.what());
	}
}

DescriptionIndex TribeDescr::add_special_building(const std::string& buildingname) {
	try {
		DescriptionIndex building = tribes_.safe_building_index(buildingname);
		if (!has_building(building)) {
			throw GameDataError("This tribe doesn't have the building '%s'", buildingname.c_str());
		}
		return building;
	} catch (const WException& e) {
		throw GameDataError(
		   "Failed adding special building '%s': %s", buildingname.c_str(), e.what());
	}
}
}
