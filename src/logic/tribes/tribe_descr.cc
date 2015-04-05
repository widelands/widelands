/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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

#include "logic/tribes/tribe_descr.h"

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
#include "logic/carrier.h"
#include "logic/constructionsite.h"
#include "logic/dismantlesite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/immovable.h"
#include "logic/militarysite.h"
#include "logic/ship.h"
#include "logic/soldier.h"
#include "logic/trainingsite.h"
#include "logic/warehouse.h"
#include "logic/worker.h"
#include "logic/world/resource_description.h"
#include "logic/world/world.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

namespace Widelands {

TribeDescr::TribeDescr
	(const LuaTable& table, const TribeBasicInfo& info, EditorGameBase& init_egbase)
	: name_(table.get_string("name")), descname_(info.descname), egbase_(init_egbase)
{
	// NOCOM(GunChleoc): Ware types listed in headquarters depend on starting conditions.
	// e.g. Barbarians have no wheat.
	// Carrier walk_load animation is empty
	// NOCOM grep for sc00_headquarters.lua -> headquarters.lua

	try {
		m_initializations = info.initializations;

		std::unique_ptr<LuaTable> items_table = table.get_table("animations");
		frontier_animation_id_ = g_gr->animations().load(*items_table->get_table("frontier"));
		flag_animation_id_ = g_gr->animations().load(*items_table->get_table("flag"));

		items_table = table.get_table("roads");
		const auto load_roads = [&items_table, this](
			const std::string& road_type, std::vector<std::string>* images) {
			std::vector<std::string> roads = items_table->get_table(road_type)->array_entries<std::string>();
			for (const std::string& filename : roads) {
				if (g_fs->file_exists(filename)) {
					images->push_back(filename);
				} else {
					throw GameDataError("File '%s' for %s road texture doesn't exist", filename.c_str(), road_type.c_str());
				}
			}
			if (images->empty()) {
				throw GameDataError("Tribe has no %s roads.", road_type.c_str());
			}
		};
		load_roads("normal", &normal_road_paths_);
		load_roads("busy", &busy_road_paths_);

		items_table = table.get_table("wares_order");
		wares_order_coords_.resize(egbase_.tribes().nrwares());
		int columnindex = 0;
		for (const int key : items_table->keys<int>()) {
			std::vector<WareIndex> column;
			std::vector<std::string> warenames = items_table->get_table(key)->array_entries<std::string>();
			for (size_t rowindex = 0; rowindex < warenames.size(); ++rowindex) {
				try {
					WareIndex wareindex = egbase_.tribes().safe_ware_index(warenames[rowindex]);
					if (has_ware(wareindex)) {
						throw GameDataError("Duplicate definition of ware '%s'", warenames[rowindex].c_str());
					}
					wares_.insert(wareindex);
					column.push_back(wareindex);
					wares_order_coords_[wareindex] = std::pair<uint32_t, uint32_t>(columnindex, rowindex);
				} catch (const WException& e) {
					throw GameDataError("Failed adding ware '%s: %s", warenames[rowindex].c_str(), e.what());
				}
			}
			if (!column.empty()) {
				wares_order_.push_back(column);
				++columnindex;
			}
		}

		items_table = table.get_table("workers_order");
		workers_order_coords_.resize(egbase_.tribes().nrworkers());
		columnindex = 0;
		for (const int key : items_table->keys<int>()) {
			std::vector<WareIndex> column;
			std::vector<std::string> workernames = items_table->get_table(key)->array_entries<std::string>();
			for (size_t rowindex = 0; rowindex < workernames.size(); ++rowindex) {
				try {
					WareIndex workerindex = egbase_.tribes().safe_worker_index(workernames[rowindex]);
					if (has_worker(workerindex)) {
						throw GameDataError("Duplicate definition of worker '%s'", workernames[rowindex].c_str());
					}
					workers_.insert(workerindex);
					column.push_back(workerindex);
					workers_order_coords_[workerindex] = std::pair<uint32_t, uint32_t>(columnindex, rowindex);

					if (egbase_.tribes().get_worker_descr(workerindex)->buildcost().size() < 1) {
						worker_types_without_cost_.push_back(workerindex);
					}
				} catch (const WException& e) {
					throw GameDataError("Failed adding worker '%s: %s", workernames[rowindex].c_str(), e.what());
				}
			}
			if (!column.empty()) {
				workers_order_.push_back(column);
				++columnindex;
			}
		}

		std::vector<std::string> immovables = table.get_table("immovables")->array_entries<std::string>();
		for (const std::string& immovablename : immovables) {
			try {
				int index = egbase_.tribes().safe_immovable_index(immovablename);
				if (immovables_.count(index) == 1) {
					throw GameDataError("Duplicate definition of immovable '%s'", immovablename.c_str());
				}
				immovables_.insert(index);
			} catch (const WException& e) {
				throw GameDataError("Failed adding immovable '%s': %s", immovablename.c_str(), e.what());
			}
		}

		for (const std::string& buildingname : table.get_table("buildings")->array_entries<std::string>()) {
			try {
				BuildingIndex index = egbase_.tribes().safe_building_index(buildingname);
				if (has_building(index)) {
					throw GameDataError("Duplicate definition of building '%s'", buildingname.c_str());
				}
				buildings_.push_back(index);

				// Register construction materials
				for (std::pair<WareIndex, uint8_t> build_cost : get_building_descr(index)->buildcost()) {
					if (!is_construction_material(build_cost.first)) {
						construction_materials_.emplace(build_cost.first);
					}
				}
				for (std::pair<WareIndex, uint8_t> enhancement_cost :
					  get_building_descr(index)->enhancement_cost()) {
					if (!is_construction_material(enhancement_cost.first)) {
						construction_materials_.emplace(enhancement_cost.first);
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
			ship_ = egbase_.tribes().safe_ship_index(shipname);
		} catch (const WException& e) {
			throw GameDataError("Failed adding ship '%s': %s", shipname.c_str(), e.what());
		}

		headquarters_ = add_special_building(table.get_string("headquarters"));
		port_ = add_special_building(table.get_string("port"));

	} catch (const GameDataError& e) {
		throw GameDataError("tribe %s: %s", name_.c_str(), e.what());
	}
}


/**
  * Access functions
  */

const std::string& TribeDescr::name() const {return name_;}
const std::string& TribeDescr::descname() const {return descname_;}

size_t TribeDescr::get_nrbuildings() const {return buildings_.size();}
size_t TribeDescr::get_nrwares() const {return wares_.size();}
size_t TribeDescr::get_nrworkers() const {return workers_.size();}

const std::vector<BuildingIndex> TribeDescr::buildings() const {return buildings_;}
const std::set<WareIndex> TribeDescr::wares() const {return wares_;}
const std::set<WareIndex> TribeDescr::workers() const {return workers_;}

bool TribeDescr::has_building(const BuildingIndex& index) const {
	return std::find(buildings_.begin(), buildings_.end(), index) != buildings_.end();
}
bool TribeDescr::has_ware(const WareIndex& index) const {
	return wares_.count(index) == 1;
}
bool TribeDescr::has_worker(const WareIndex& index) const {
	return workers_.count(index) == 1;
}
bool TribeDescr::is_construction_material(const WareIndex& index) const {
	return construction_materials_.count(index) == 1;
}

BuildingIndex TribeDescr::building_index(const std::string & buildingname) const {
	return egbase_.tribes().building_index(buildingname);
}

int TribeDescr::immovable_index(const std::string & immovablename) const {
	return egbase_.tribes().immovable_index(immovablename);
}
WareIndex TribeDescr::ware_index(const std::string & warename) const {
	return egbase_.tribes().ware_index(warename);
}
WareIndex TribeDescr::worker_index(const std::string & workername) const {
	return egbase_.tribes().worker_index(workername);
}

BuildingIndex TribeDescr::safe_building_index(const std::string& buildingname) const {
	return egbase_.tribes().safe_building_index(buildingname);
}

WareIndex TribeDescr::safe_ware_index(const std::string & warename) const {
	return egbase_.tribes().safe_ware_index(warename);
}
WareIndex TribeDescr::safe_worker_index(const std::string& workername) const {
	return egbase_.tribes().safe_worker_index(workername);
}

WareDescr const * TribeDescr::get_ware_descr(const WareIndex& index) const {
	return egbase_.tribes().get_ware_descr(index);
}
WorkerDescr const* TribeDescr::get_worker_descr(const WareIndex& index) const {
	return egbase_.tribes().get_worker_descr(index);
}

BuildingDescr const * TribeDescr::get_building_descr(const BuildingIndex& index) const {
	return egbase_.tribes().get_building_descr(index);
}
ImmovableDescr const * TribeDescr::get_immovable_descr(int index) const {
	return egbase_.tribes().get_immovable_descr(index);
}

WareIndex TribeDescr::builder() const {
	assert(egbase_.tribes().worker_exists(builder_));
	return builder_;
}
WareIndex TribeDescr::carrier() const {
	assert(egbase_.tribes().worker_exists(carrier_));
	return carrier_;
}
WareIndex TribeDescr::carrier2() const {
	assert(egbase_.tribes().worker_exists(carrier2_));
	return carrier2_;
}
WareIndex TribeDescr::geologist() const {
	assert(egbase_.tribes().worker_exists(geologist_));
	return geologist_;
}
WareIndex TribeDescr::soldier() const {
	assert(egbase_.tribes().worker_exists(soldier_));
	return soldier_;
}
int TribeDescr::ship() const {
	assert(egbase_.tribes().ship_exists(ship_));
	return ship_;
}
BuildingIndex TribeDescr::headquarters() const {
	assert(egbase_.tribes().building_exists(headquarters_));
	return headquarters_;
}
BuildingIndex TribeDescr::port() const {
	assert(egbase_.tribes().building_exists(port_));
	return port_;
}
const std::vector<WareIndex>& TribeDescr::worker_types_without_cost() const {
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

void TribeDescr::add_normal_road_texture(std::unique_ptr<Texture> texture) {
	road_textures_.add_normal_road_texture(std::move(texture));
}

void TribeDescr::add_busy_road_texture(std::unique_ptr<Texture> texture) {
	road_textures_.add_busy_road_texture(std::move(texture));
}

const RoadTextures& TribeDescr::road_textures() const {
	return road_textures_;
}

/*
==============
Find the best matching indicator for the given amount.
==============
*/
uint32_t TribeDescr::get_resource_indicator
	(ResourceDescription const * const res, uint32_t const amount) const
{
	if (!res || !amount) {
		int32_t idx = immovable_index("resi_none");
		// NOCOM(GunChleoc): use has_immovable
		if (idx == -1)
			throw GameDataError
				("tribe %s does not declare a resource indicator resi_none!",
				 name().c_str());
		return idx;
	}

	int32_t i = 1;
	int32_t num_indicators = 0;
	for (;;) {
		const std::string resi_filename = (boost::format("resi_%s%i") % res->name().c_str() % i).str();
		// NOCOM(GunChleoc): use has_immovable
		if (immovable_index(resi_filename) == -1)
			break;
		++i;
		++num_indicators;
	}

	if (!num_indicators)
		throw GameDataError
			("tribe %s does not declare a resource indicator for resource %s",
			 name().c_str(),
			 res->name().c_str());

	int32_t bestmatch =
		static_cast<int32_t>
			((static_cast<float>(amount) / res->max_amount())
			 *
			 num_indicators);
	if (bestmatch > num_indicators)
		throw GameDataError
			("Amount of %s is %i but max amount is %i",
			 res->name().c_str(),
			 amount,
			 res->max_amount());
	if (static_cast<int32_t>(amount) < res->max_amount())
		bestmatch += 1; // Resi start with 1, not 0

	return immovable_index((boost::format("resi_%s%i")
										 % res->name().c_str()
										 % bestmatch).str());
}


void TribeDescr::resize_ware_orders(size_t maxLength) {
	bool need_resize = false;

	//check if we actually need to resize
	for (WaresOrder::iterator it = wares_order_.begin(); it != wares_order_.end(); ++it) {
		if (it->size() > maxLength) {
			need_resize = true;
		  }
	 }

	//resize
	if (need_resize) {

		//build new smaller wares_order
		WaresOrder new_wares_order;
		for (WaresOrder::iterator it = wares_order_.begin(); it != wares_order_.end(); ++it) {
			new_wares_order.push_back(std::vector<Widelands::WareIndex>());
			for (std::vector<Widelands::WareIndex>::iterator it2 = it->begin(); it2 != it->end(); ++it2) {
				if (new_wares_order.rbegin()->size() >= maxLength) {
					new_wares_order.push_back(std::vector<Widelands::WareIndex>());
				}
				new_wares_order.rbegin()->push_back(*it2);
				wares_order_coords_[*it2].first = new_wares_order.size() - 1;
				wares_order_coords_[*it2].second = new_wares_order.rbegin()->size() - 1;
			}
		}

		//remove old array
		wares_order_.clear();
		wares_order_ = new_wares_order;
	}
}

WareIndex TribeDescr::add_special_worker(const std::string& workername) {
	try {
		WareIndex worker = egbase_.tribes().safe_worker_index(workername);
		if (!has_worker(worker)) {
			throw GameDataError("This tribe doesn't have the worker '%s'", workername.c_str());
		}
		return worker;
	} catch (const WException& e) {
		throw GameDataError("Failed adding special worker '%s': %s", workername.c_str(), e.what());
	}
}

BuildingIndex TribeDescr::add_special_building(const std::string& buildingname) {
	try {
		BuildingIndex building = egbase_.tribes().safe_building_index(buildingname);
		if (!has_building(building)) {
			throw GameDataError("This tribe doesn't have the building '%s'", buildingname.c_str());
		}
		return building;
	} catch (const WException& e) {
		throw GameDataError("Failed adding special building '%s': %s", buildingname.c_str(), e.what());
	}
}

}
