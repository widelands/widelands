/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "logic/map_objects/tribes/tribes.h"

#include <memory>

#include "base/wexception.h"
#include "graphic/graphic.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/ferry.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"

namespace Widelands {
Tribes::Tribes()
   : buildings_(new DescriptionMaintainer<BuildingDescr>()),
     immovables_(new DescriptionMaintainer<ImmovableDescr>()),
     ships_(new DescriptionMaintainer<ShipDescr>()),
     wares_(new DescriptionMaintainer<WareDescr>()),
     workers_(new DescriptionMaintainer<WorkerDescr>()),
     tribes_(new DescriptionMaintainer<TribeDescr>()),
     legacy_lookup_table_(new TribesLegacyLookupTable()),
     largest_workarea_(0) {
}

void Tribes::add_constructionsite_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	buildings_->add(new ConstructionSiteDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table, *this));
}

void Tribes::add_dismantlesite_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	buildings_->add(new DismantleSiteDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table, *this));
}

void Tribes::add_militarysite_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	buildings_->add(new MilitarySiteDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table, *this));
}

void Tribes::add_productionsite_type(const LuaTable& table, const World& world) {
	i18n::Textdomain td("tribes");
	const std::string msgctxt = table.get_string("msgctxt");
	buildings_->add(
	   new ProductionSiteDescr(pgettext_expr(msgctxt.c_str(), table.get_string("descname").c_str()),
	                           msgctxt, table, *this, world));
}

void Tribes::add_trainingsite_type(const LuaTable& table, const World& world) {
	i18n::Textdomain td("tribes");
	const std::string msgctxt = table.get_string("msgctxt");
	buildings_->add(
	   new TrainingSiteDescr(pgettext_expr(msgctxt.c_str(), table.get_string("descname").c_str()),
	                         msgctxt, table, *this, world));
}

void Tribes::add_warehouse_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	buildings_->add(new WarehouseDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table, *this));
}

void Tribes::add_market_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	buildings_->add(new MarketDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table, *this));
}

void Tribes::add_immovable_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	immovables_->add(new ImmovableDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table, *this));
}

void Tribes::add_ship_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	ships_->add(new ShipDescr(_(table.get_string("descname")), table));
}

void Tribes::add_ware_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	wares_->add(new WareDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table));
}

void Tribes::add_carrier_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	workers_->add(new CarrierDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table, *this));
}

void Tribes::add_ferry_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	workers_->add(new FerryDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table, *this));
}

void Tribes::add_soldier_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	workers_->add(new SoldierDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table, *this));
}

void Tribes::add_worker_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	workers_->add(new WorkerDescr(
	   pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
	   table, *this));
}

void Tribes::add_tribe(const LuaTable& table, const World& world) {
	const std::string name = table.get_string("name");
	if (Widelands::tribe_exists(name)) {
		tribes_->add(new TribeDescr(table, Widelands::get_tribeinfo(name), world, *this));
	} else {
		throw GameDataError("The tribe '%s'' has no preload file.", name.c_str());
	}
}

void Tribes::add_custom_building(const LuaTable& table) {
	const std::string tribename = table.get_string("tribename");
	if (Widelands::tribe_exists(tribename)) {
		TribeDescr* descr = tribes_->get_mutable(tribe_index(tribename));
		const std::string buildingname = table.get_string("buildingname");
		descr->add_building(buildingname);
	} else {
		throw GameDataError("The tribe '%s'' has no preload file.", tribename.c_str());
	}
}

void Tribes::add_custom_worker(const LuaTable& table) {
	const std::string tribename = table.get_string("tribename");
	if (Widelands::tribe_exists(tribename)) {
		TribeDescr* descr = tribes_->get_mutable(tribe_index(tribename));
		descr->add_worker(table.get_string("workername"));
	} else {
		throw GameDataError("The tribe '%s'' has no preload file.", tribename.c_str());
	}
}

size_t Tribes::nrbuildings() const {
	return buildings_->size();
}

size_t Tribes::nrtribes() const {
	return tribes_->size();
}

size_t Tribes::nrwares() const {
	return wares_->size();
}

size_t Tribes::nrworkers() const {
	return workers_->size();
}

bool Tribes::ware_exists(const std::string& warename) const {
	return wares_->exists(warename) != nullptr;
}
bool Tribes::ware_exists(const DescriptionIndex& index) const {
	return wares_->get_mutable(index) != nullptr;
}
bool Tribes::worker_exists(const std::string& workername) const {
	return workers_->exists(workername) != nullptr;
}
bool Tribes::worker_exists(const DescriptionIndex& index) const {
	return workers_->get_mutable(index) != nullptr;
}
bool Tribes::building_exists(const std::string& buildingname) const {
	return buildings_->exists(buildingname) != nullptr;
}
bool Tribes::building_exists(const DescriptionIndex& index) const {
	return buildings_->get_mutable(index) != nullptr;
}
bool Tribes::immovable_exists(DescriptionIndex index) const {
	return immovables_->get_mutable(index) != nullptr;
}
bool Tribes::ship_exists(DescriptionIndex index) const {
	return ships_->get_mutable(index) != nullptr;
}
bool Tribes::tribe_exists(DescriptionIndex index) const {
	return tribes_->get_mutable(index) != nullptr;
}

DescriptionIndex Tribes::safe_building_index(const std::string& buildingname) const {
	const DescriptionIndex result =
	   building_index(legacy_lookup_table_.get()->lookup_building(buildingname));
	if (!building_exists(result)) {
		throw GameDataError("Unknown building type \"%s\"", buildingname.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_immovable_index(const std::string& immovablename) const {
	const DescriptionIndex result =
	   immovable_index(legacy_lookup_table_.get()->lookup_immovable(immovablename));
	if (!immovable_exists(result)) {
		throw GameDataError("Unknown immovable type \"%s\"", immovablename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_ship_index(const std::string& shipname) const {
	const DescriptionIndex result = ship_index(legacy_lookup_table_.get()->lookup_ship(shipname));
	if (!ship_exists(result)) {
		throw GameDataError("Unknown ship type \"%s\"", shipname.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_tribe_index(const std::string& tribename) const {
	const DescriptionIndex result = tribe_index(tribename);
	if (!tribe_exists(result)) {
		throw GameDataError("Unknown tribe \"%s\"", tribename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_ware_index(const std::string& warename) const {
	const DescriptionIndex result = ware_index(legacy_lookup_table_.get()->lookup_ware(warename));
	if (!ware_exists(result)) {
		throw GameDataError("Unknown ware type \"%s\"", warename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_worker_index(const std::string& workername) const {
	const DescriptionIndex result =
	   worker_index(legacy_lookup_table_.get()->lookup_worker(workername));
	if (!worker_exists(result)) {
		throw GameDataError("Unknown worker type \"%s\"", workername.c_str());
	}
	return result;
}

DescriptionIndex Tribes::building_index(const std::string& buildingname) const {
	return buildings_->get_index(buildingname);
}

DescriptionIndex Tribes::immovable_index(const std::string& immovablename) const {
	return immovables_->get_index(immovablename);
}

DescriptionIndex Tribes::ship_index(const std::string& shipname) const {
	return ships_->get_index(shipname);
}

DescriptionIndex Tribes::tribe_index(const std::string& tribename) const {
	return tribes_->get_index(tribename);
}

DescriptionIndex Tribes::ware_index(const std::string& warename) const {
	return wares_->get_index(warename);
}

DescriptionIndex Tribes::worker_index(const std::string& workername) const {
	return workers_->get_index(workername);
}

const BuildingDescr* Tribes::get_building_descr(DescriptionIndex buildingindex) const {
	return buildings_->get_mutable(buildingindex);
}

BuildingDescr* Tribes::get_mutable_building_descr(DescriptionIndex buildingindex) const {
	return buildings_->get_mutable(buildingindex);
}

const ImmovableDescr* Tribes::get_immovable_descr(DescriptionIndex immovableindex) const {
	return immovables_->get_mutable(immovableindex);
}

const ShipDescr* Tribes::get_ship_descr(DescriptionIndex shipindex) const {
	return ships_->get_mutable(shipindex);
}

const WareDescr* Tribes::get_ware_descr(DescriptionIndex wareindex) const {
	return wares_->get_mutable(wareindex);
}

const WorkerDescr* Tribes::get_worker_descr(DescriptionIndex workerindex) const {
	return workers_->get_mutable(workerindex);
}

const TribeDescr* Tribes::get_tribe_descr(DescriptionIndex tribeindex) const {
	return tribes_->get_mutable(tribeindex);
}

void Tribes::load_graphics() {
	for (size_t tribeindex = 0; tribeindex < nrtribes(); ++tribeindex) {
		TribeDescr* tribe = tribes_->get_mutable(tribeindex);
		for (const std::string& texture_path : tribe->normal_road_paths()) {
			tribe->add_normal_road_texture(g_gr->images().get(texture_path));
		}
		for (const std::string& texture_path : tribe->busy_road_paths()) {
			tribe->add_busy_road_texture(g_gr->images().get(texture_path));
		}
		for (const std::string& texture_path : tribe->waterway_paths()) {
			tribe->add_waterway_texture(g_gr->images().get(texture_path));
		}
	}
}

void Tribes::postload() {
	largest_workarea_ = 0;
	for (DescriptionIndex i = 0; i < buildings_->size(); ++i) {
		BuildingDescr& building_descr = *buildings_->get_mutable(i);

		// Calculate largest possible workarea radius
		for (const auto& pair : building_descr.workarea_info()) {
			largest_workarea_ = std::max(largest_workarea_, pair.first);
		}

		// Add consumers and producers to wares.
		if (upcast(ProductionSiteDescr, de, &building_descr)) {
			for (const auto& ware_amount : de->input_wares()) {
				wares_->get_mutable(ware_amount.first)->add_consumer(i);
			}
			for (const DescriptionIndex& wareindex : de->output_ware_types()) {
				wares_->get_mutable(wareindex)->add_producer(i);
			}
			for (const auto& job : de->working_positions()) {
				workers_->get_mutable(job.first)->add_employer(i);
			}
		}

		// Register which buildings buildings can have been enhanced from
		const DescriptionIndex& enhancement = building_descr.enhancement();
		if (building_exists(enhancement)) {
			buildings_->get_mutable(enhancement)->set_enhanced_from(i);
		}
	}

	// Calculate the trainingsites proportions.
	postload_calculate_trainingsites_proportions();

	// Some final checks on the gamedata
	for (DescriptionIndex i = 0; i < tribes_->size(); ++i) {
		TribeDescr* tribe_descr = tribes_->get_mutable(i);

		// Register which wares and workers have economy demand checks for each tribe
		for (const DescriptionIndex bi : tribe_descr->buildings()) {
			postload_register_economy_demand_checks(*buildings_->get_mutable(bi), *tribe_descr);
		}
		// Verify that the preciousness has been set for all of the tribe's wares
		for (const DescriptionIndex wi : tribe_descr->wares()) {
			if (tribe_descr->get_ware_descr(wi)->ai_hints().preciousness(tribe_descr->name()) ==
			    kInvalidWare) {
				throw GameDataError("The ware '%s' needs to define a preciousness for tribe '%s'",
				                    tribe_descr->get_ware_descr(wi)->name().c_str(),
				                    tribe_descr->name().c_str());
			}
		}
	}
}

/// Register wares and workers that have economy demand checks for a building
void Tribes::postload_register_economy_demand_checks(BuildingDescr& building_descr,
                                                     const TribeDescr& tribe_descr) {
	if (upcast(ProductionSiteDescr, prodsite, &building_descr)) {
		// This function can be called only once per loading of tribes
		assert(prodsite->ware_demand_checks() != nullptr);

		for (const DescriptionIndex wi : *prodsite->ware_demand_checks()) {
			if (!tribe_descr.has_ware(wi)) {
				throw GameDataError("Productionsite '%s' for tribe '%s' has an economy demand check "
				                    "for ware '%s', but the tribe does not use this ware",
				                    prodsite->name().c_str(), tribe_descr.name().c_str(),
				                    get_ware_descr(wi)->name().c_str());
			}
			wares_->get_mutable(wi)->set_has_demand_check(tribe_descr.name());
		}
		for (const DescriptionIndex wi : *prodsite->worker_demand_checks()) {
			if (!tribe_descr.has_worker(wi)) {
				throw GameDataError("Productionsite '%s' for tribe '%s' has an economy demand check "
				                    "for worker '%s', but the tribe does not use this worker",
				                    prodsite->name().c_str(), tribe_descr.name().c_str(),
				                    get_worker_descr(wi)->name().c_str());
			}
			workers_->get_mutable(wi)->set_has_demand_check();
		}
		prodsite->clear_demand_checks();
	}
}

/// Set default trainingsites proportions for AI. Make sure that we get a sum of ca. 100
void Tribes::postload_calculate_trainingsites_proportions() {
	for (DescriptionIndex i = 0; i < tribes_->size(); ++i) {
		TribeDescr* tribe_descr = tribes_->get_mutable(i);
		unsigned int trainingsites_without_percent = 0;
		int used_percent = 0;
		std::vector<BuildingDescr*> traingsites_with_percent;
		for (const DescriptionIndex& index : tribe_descr->trainingsites()) {
			BuildingDescr* descr = get_mutable_building_descr(index);
			if (descr->hints().trainingsites_max_percent() == 0) {
				++trainingsites_without_percent;
			} else {
				used_percent += descr->hints().trainingsites_max_percent();
				traingsites_with_percent.push_back(descr);
			}
		}

		log("%s trainingsites: We have used up %d%% on %" PRIuS " sites, there are %d without\n",
		    tribe_descr->name().c_str(), used_percent, traingsites_with_percent.size(),
		    trainingsites_without_percent);

		// Adjust used_percent if we don't have at least 5% for each remaining trainingsite
		const float limit = 100 - trainingsites_without_percent * 5;
		if (used_percent > limit) {
			const int deductme = (used_percent - limit) / traingsites_with_percent.size();
			used_percent = 0;
			for (BuildingDescr* descr : traingsites_with_percent) {
				descr->set_hints_trainingsites_max_percent(descr->hints().trainingsites_max_percent() -
				                                           deductme);
				used_percent += descr->hints().trainingsites_max_percent();
			}
			log("%s trainingsites: Used percent was adjusted to %d%%\n", tribe_descr->name().c_str(),
			    used_percent);
		}

		// Now adjust for trainingsites that didn't have their max_percent set
		if (trainingsites_without_percent > 0) {
			int percent_to_use = std::ceil((100 - used_percent) / trainingsites_without_percent);
			// We sometimes get below 100% in spite of the ceil call above.
			// A total sum a bit above 100% is fine though, so we increment until it's big enough.
			while ((used_percent + percent_to_use * trainingsites_without_percent) < 100) {
				++percent_to_use;
			}
			log("%s trainingsites: Assigning %d%% to each of the remaining %d sites\n",
			    tribe_descr->name().c_str(), percent_to_use, trainingsites_without_percent);
			if (percent_to_use < 1) {
				throw GameDataError(
				   "%s: Training sites without predefined proportions add up to < 1%% and "
				   "will never be built: %d",
				   tribe_descr->name().c_str(), used_percent);
			}
			for (const DescriptionIndex& index : tribe_descr->trainingsites()) {
				BuildingDescr* descr = get_mutable_building_descr(index);
				if (descr->hints().trainingsites_max_percent() == 0) {
					descr->set_hints_trainingsites_max_percent(percent_to_use);
					used_percent += percent_to_use;
				}
			}
		}
		if (used_percent < 100) {
			throw GameDataError("%s: Final training sites proportions add up to < 100%%: %d",
			                    tribe_descr->name().c_str(), used_percent);
		}
	}
}

uint32_t Tribes::get_largest_workarea() const {
	return largest_workarea_;
}
}  // namespace Widelands
