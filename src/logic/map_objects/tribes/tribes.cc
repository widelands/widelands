/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "graphic/graphic.h"
#include "logic/game_data_error.h"

namespace Widelands {

Tribes::Tribes() :
	buildings_(new DescriptionMaintainer<BuildingDescr>()),
	immovables_(new DescriptionMaintainer<ImmovableDescr>()),
	ships_(new DescriptionMaintainer<ShipDescr>()),
	wares_(new DescriptionMaintainer<WareDescr>()),
	workers_(new DescriptionMaintainer<WorkerDescr>()),
	tribes_(new DescriptionMaintainer<TribeDescr>()) {
}

std::vector<std::string> Tribes::get_all_tribenames() {
	std::vector<std::string> tribenames;
	LuaInterface lua;
	std::unique_ptr<LuaTable> table(lua.run_script("tribes/preload.lua"));
	for (const int key : table->keys<int>()) {
		std::unique_ptr<LuaTable> info = table->get_table(key);
		info->do_not_warn_about_unaccessed_keys();
		tribenames.push_back(info->get_string("name"));
	}
	return tribenames;
}

std::vector<TribeBasicInfo> Tribes::get_all_tribeinfos() {
	std::vector<TribeBasicInfo> tribeinfos;
	LuaInterface lua;
	std::unique_ptr<LuaTable> table(lua.run_script("tribes/preload.lua"));
	for (const int key : table->keys<int>()) {
		tribeinfos.push_back(TribeBasicInfo(table->get_table(key)));
	}
	return tribeinfos;
}

TribeBasicInfo Tribes::tribeinfo(const std::string& tribename) {
	if (tribe_exists(tribename)) {
		for (const TribeBasicInfo& info : get_all_tribeinfos()) {
			if (info.name == tribename) {
				return info;
			}
		}
	}
	throw GameDataError("The tribe '%s'' does not exist.", tribename.c_str());
	assert(false); // A TribeBasicInfo should have been found
}

bool Tribes::tribe_exists(const std::string& tribename) {
	for (const std::string& name : get_all_tribenames()) {
		if (name == tribename) {
			return true;
		}
	}
	return false;
}

void Tribes::add_constructionsite_type(const LuaTable& table, const EditorGameBase& egbase) {
	i18n::Textdomain td("tribes");
	buildings_->add(
				new ConstructionSiteDescr(
					pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
					table,
					egbase));
}

void Tribes::add_dismantlesite_type(const LuaTable& table, const EditorGameBase& egbase) {
	i18n::Textdomain td("tribes");
	buildings_->add(
				new DismantleSiteDescr(
					pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
					table,
					egbase));
}

void Tribes::add_militarysite_type(const LuaTable& table, const EditorGameBase& egbase) {
	i18n::Textdomain td("tribes");
	buildings_->add(
				new MilitarySiteDescr(
					pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
					table,
					egbase));
}

void Tribes::add_productionsite_type(const LuaTable& table, const EditorGameBase& egbase) {
	i18n::Textdomain td("tribes");
	const char* msgctxt = table.get_string("msgctxt").c_str();
	buildings_->add(
				new ProductionSiteDescr(
					pgettext_expr(msgctxt, table.get_string("descname").c_str()),
					msgctxt,
					table,
					egbase));
}

void Tribes::add_trainingsite_type(const LuaTable& table, const EditorGameBase& egbase) {
	i18n::Textdomain td("tribes");
	buildings_->add(
				new TrainingSiteDescr(
					pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
					table,
					egbase));
}

void Tribes::add_warehouse_type(const LuaTable& table, const EditorGameBase& egbase) {
	i18n::Textdomain td("tribes");
	buildings_->add(
				new WarehouseDescr(
					pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
					table,
					egbase));
}

void Tribes::add_immovable_type(const LuaTable& table) {
	i18n::Textdomain td("tribes");
	immovables_->add(
				new ImmovableDescr(
					pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
					table,
					*this));
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

void Tribes::add_carrier_type(const LuaTable& table, const EditorGameBase& egbase) {
	i18n::Textdomain td("tribes");
	workers_->add(new CarrierDescr(
						  pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
						  table,
						  egbase));
}

void Tribes::add_soldier_type(const LuaTable& table, const EditorGameBase& egbase) {
	i18n::Textdomain td("tribes");
	workers_->add(
				new SoldierDescr(
					pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
					table,
					egbase));
}

void Tribes::add_worker_type(const LuaTable& table, const EditorGameBase& egbase) {
	i18n::Textdomain td("tribes");
	workers_->add(
				new WorkerDescr(
					pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
					table,
					egbase));
}

void Tribes::add_tribe(const LuaTable& table, const EditorGameBase& egbase) {
	const std::string name = table.get_string("name");
	if (tribe_exists(name)) {
		tribes_->add(new TribeDescr(table, Tribes::tribeinfo(name), egbase.tribes()));
	} else {
		throw GameDataError("The tribe '%s'' has no preload file.", name.c_str());
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

bool Tribes::ware_exists(const DescriptionIndex& index) const {
	return wares_->get_mutable(index) != nullptr;
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
	const DescriptionIndex result = building_index(buildingname);
	if (!building_exists(result)) {
		throw GameDataError("Unknown building type \"%s\"", buildingname.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_immovable_index(const std::string& immovablename) const {
	const DescriptionIndex result = immovable_index(immovablename);
	if (!immovable_exists(result)) {
		throw GameDataError("Unknown immovable type \"%s\"", immovablename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_ship_index(const std::string& shipname) const {
	const DescriptionIndex result = ship_index(shipname);
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
	const DescriptionIndex result = ware_index(warename);
	if (!ware_exists(result)) {
		throw GameDataError("Unknown ware type \"%s\"", warename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_worker_index(const std::string& workername) const {
	const DescriptionIndex result = worker_index(workername);
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

void Tribes::set_ware_type_has_demand_check(const DescriptionIndex& wareindex,
														  const std::string& tribename) const {
	wares_->get_mutable(wareindex)->set_has_demand_check(tribename);
}

void Tribes::set_worker_type_has_demand_check(const DescriptionIndex& workerindex) const {
	workers_->get_mutable(workerindex)->set_has_demand_check();
}


void Tribes::load_graphics()
{
	// These will be deleted at the end of the method.
	std::vector<std::unique_ptr<Texture>> individual_textures_;
	for (size_t tribeindex = 0; tribeindex < nrtribes(); ++tribeindex) {
		TribeDescr* tribe = tribes_->get_mutable(tribeindex);
		for (const std::string& texture_path : tribe->normal_road_paths()) {
			tribe->add_normal_road_texture(g_gr->images().get(texture_path));
		}
		for (const std::string& texture_path : tribe->busy_road_paths()) {
			tribe->add_busy_road_texture(g_gr->images().get(texture_path));
		}
	}
}

void Tribes::postload() {
	for (DescriptionIndex i = 0; i < buildings_->size(); ++i) {
		BuildingDescr& building_descr = *buildings_->get_mutable(i);

		// Add consumers and producers to wares.
		if (upcast(ProductionSiteDescr, de, &building_descr)) {
			for (const WareAmount& ware_amount : de->inputs()) {
				wares_->get_mutable(ware_amount.first)->add_consumer(i);
			}
			for (const DescriptionIndex& wareindex : de->output_ware_types()) {
				wares_->get_mutable(wareindex)->add_producer(i);
			}
		}

		// Register which buildings buildings can have been enhanced from
		const DescriptionIndex& enhancement = building_descr.enhancement();
		if (building_exists(enhancement)) {
			buildings_->get_mutable(enhancement)->set_enhanced_from(i);
		}
	}
	// Resize the configuration of our wares if they won't fit in the current window (12 = info label size)
	int number = (g_gr->get_yres() - 290) / (WARE_MENU_PIC_HEIGHT + WARE_MENU_PIC_PAD_Y + 12);
	for (DescriptionIndex i = 0; i < tribes_->size(); ++i) {
		tribes_->get_mutable(i)->resize_ware_orders(number);
	}
}

} // namespace Widelands
