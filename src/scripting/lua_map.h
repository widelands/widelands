/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_LUA_MAP_H
#define WL_SCRIPTING_LUA_MAP_H

#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/road.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/world/terrain_description.h"
#include "scripting/lua.h"
#include "scripting/luna.h"

namespace Widelands {
class BuildingDescr;
class Bob;
class ResourceDescription;
class WareDescr;
class TribeDescr;
}  // namespace Widelands

namespace LuaMaps {

// Used as return for parse_ware_workers_* functions
enum class RequestedWareWorker { kAll, kSingle, kList, kUndefined };

/*
 * Base class for all classes in wl.map
 */
class LuaMapModuleClass : public LunaClass {
public:
	const char* get_modulename() override {
		return "map";
	}
};

class LuaMap : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaMap);

	~LuaMap() override {
	}

	LuaMap() {
	}
	explicit LuaMap(lua_State* L) {
		report_error(L, "Cannot instantiate a 'Map' directly!");
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */
	int get_allows_seafaring(lua_State*);
	int get_number_of_port_spaces(lua_State*);
	int get_port_spaces(lua_State*);
	int get_width(lua_State*);
	int get_height(lua_State*);
	int get_player_slots(lua_State*);
	int get_waterway_max_length(lua_State*);

	/*
	 * Lua methods
	 */
	int count_conquerable_fields(lua_State*);
	int count_terrestrial_fields(lua_State*);
	int count_owned_valuable_fields(lua_State*);
	int place_immovable(lua_State*);
	int get_field(lua_State*);
	int recalculate(lua_State*);
	int recalculate_seafaring(lua_State*);
	int set_port_space(lua_State*);
	int sea_route_exists(lua_State*);
	int set_waterway_max_length(lua_State*);
	int find_ocean_fields(lua_State*);

	/*
	 * C methods
	 */
private:
};

class LuaTribeDescription : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaTribeDescription);

	~LuaTribeDescription() override {
	}

	LuaTribeDescription() : tribedescr_(nullptr) {
	}
	explicit LuaTribeDescription(const Widelands::TribeDescr* const tribedescr)
	   : tribedescr_(tribedescr) {
	}
	explicit LuaTribeDescription(lua_State* L) : tribedescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'LuaTribeDescription' directly!");
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */
	int get_buildings(lua_State*);
	int get_builder(lua_State* L);
	int get_carriers(lua_State*);
	int get_carrier(lua_State*);
	int get_carrier2(lua_State*);
	int get_ferry(lua_State*);
	int get_descname(lua_State*);
	int get_immovables(lua_State*);
	int get_resource_indicators(lua_State*);
	int get_geologist(lua_State*);
	int get_name(lua_State*);
	int get_port(lua_State*);
	int get_ship(lua_State*);
	int get_soldier(lua_State*);
	int get_wares(lua_State*);
	int get_workers(lua_State*);
	int get_directory(lua_State*);
	int get_collectors_points_table(lua_State*);

	/*
	 * Lua methods
	 */
	int has_building(lua_State*);
	int has_ware(lua_State*);
	int has_worker(lua_State*);

	/*
	 * C methods
	 */
protected:
	const Widelands::TribeDescr* get() const {
		assert(tribedescr_ != nullptr);
		return tribedescr_;
	}
	// For persistence.
	void set_description_pointer(const Widelands::TribeDescr* pointer) {
		tribedescr_ = pointer;
	}

private:
	const Widelands::TribeDescr* tribedescr_;
};

class LuaMapObjectDescription : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaMapObjectDescription);

	~LuaMapObjectDescription() override {
	}

	LuaMapObjectDescription() : mapobjectdescr_(nullptr) {
	}
	explicit LuaMapObjectDescription(const Widelands::MapObjectDescr* const mapobjectdescr)
	   : mapobjectdescr_(mapobjectdescr) {
	}
	explicit LuaMapObjectDescription(lua_State* L) : mapobjectdescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'MapObjectDescription' directly!");
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */
	int get_descname(lua_State*);
	int get_icon_name(lua_State*);
	int get_name(lua_State*);
	int get_type_name(lua_State*);

	/*
	 * Lua methods
	 */
	int helptexts(lua_State*);

	/*
	 * C methods
	 */
protected:
	const Widelands::MapObjectDescr* get() const {
		assert(mapobjectdescr_ != nullptr);
		return mapobjectdescr_;
	}
	// For persistence.
	void set_description_pointer(const Widelands::MapObjectDescr* pointer) {
		mapobjectdescr_ = pointer;
	}

private:
	const Widelands::MapObjectDescr* mapobjectdescr_;
};

#define CASTED_GET_DESCRIPTION(klass)                                                              \
	const Widelands::klass* get() const {                                                           \
		return static_cast<const Widelands::klass*>(LuaMapObjectDescription::get());                 \
	}

class LuaImmovableDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaImmovableDescription);

	~LuaImmovableDescription() override {
	}

	LuaImmovableDescription() {
	}
	explicit LuaImmovableDescription(const Widelands::ImmovableDescr* const immovabledescr)
	   : LuaMapObjectDescription(immovabledescr) {
	}
	explicit LuaImmovableDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */
	int get_species(lua_State*);
	int get_buildcost(lua_State*);
	int get_becomes(lua_State*);
	int get_terrain_affinity(lua_State*);
	int get_size(lua_State*);

	/*
	 * Lua methods
	 */
	int has_attribute(lua_State*);
	int probability_to_grow(lua_State*);

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(ImmovableDescr)
};

class LuaBuildingDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaBuildingDescription);

	~LuaBuildingDescription() override {
	}

	LuaBuildingDescription() {
	}
	explicit LuaBuildingDescription(const Widelands::BuildingDescr* const buildingdescr)
	   : LuaMapObjectDescription(buildingdescr) {
	}
	explicit LuaBuildingDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */
	int get_buildcost(lua_State*);
	int get_buildable(lua_State*);
	int get_conquers(lua_State*);
	int get_destructible(lua_State*);
	int get_enhanced(lua_State*);
	int get_enhanced_from(lua_State*);
	int get_enhancement_cost(lua_State*);
	int get_enhancement(lua_State*);
	int get_is_mine(lua_State*);
	int get_is_port(lua_State*);
	int get_size(lua_State*);
	int get_isproductionsite(lua_State*);
	int get_returns_on_dismantle(lua_State*);
	int get_enhancement_returns_on_dismantle(lua_State*);
	int get_vision_range(lua_State*);
	int get_workarea_radius(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(BuildingDescr)
};

class LuaConstructionSiteDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaConstructionSiteDescription);

	~LuaConstructionSiteDescription() override {
	}

	LuaConstructionSiteDescription() {
	}
	explicit LuaConstructionSiteDescription(
	   const Widelands::ConstructionSiteDescr* const constructionsitedescr)
	   : LuaBuildingDescription(constructionsitedescr) {
	}
	explicit LuaConstructionSiteDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

private:
	CASTED_GET_DESCRIPTION(ConstructionSiteDescr)
};

class LuaDismantleSiteDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaDismantleSiteDescription);

	~LuaDismantleSiteDescription() override {
	}

	LuaDismantleSiteDescription() {
	}
	explicit LuaDismantleSiteDescription(
	   const Widelands::DismantleSiteDescr* const dismantlesitedescr)
	   : LuaBuildingDescription(dismantlesitedescr) {
	}
	explicit LuaDismantleSiteDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

private:
	CASTED_GET_DESCRIPTION(DismantleSiteDescr)
};

class LuaProductionSiteDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaProductionSiteDescription);

	~LuaProductionSiteDescription() override {
	}

	LuaProductionSiteDescription() {
	}
	explicit LuaProductionSiteDescription(
	   const Widelands::ProductionSiteDescr* const productionsitedescr)
	   : LuaBuildingDescription(productionsitedescr) {
	}
	explicit LuaProductionSiteDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_inputs(lua_State*);
	int get_collected_bobs(lua_State*);
	int get_collected_immovables(lua_State*);
	int get_collected_resources(lua_State*);
	int get_created_bobs(lua_State*);
	int get_created_immovables(lua_State*);
	int get_created_resources(lua_State*);
	int get_output_ware_types(lua_State*);
	int get_output_worker_types(lua_State*);
	int get_production_programs(lua_State*);
	int get_supported_productionsites(lua_State*);
	int get_supported_by_productionsites(lua_State*);
	int get_working_positions(lua_State*);

	/*
	 * Lua methods
	 */

	int consumed_wares_workers(lua_State*);
	int produced_wares(lua_State*);
	int recruited_workers(lua_State*);

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(ProductionSiteDescr)
};

class LuaMilitarySiteDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaMilitarySiteDescription);

	~LuaMilitarySiteDescription() override {
	}

	LuaMilitarySiteDescription() {
	}
	explicit LuaMilitarySiteDescription(const Widelands::MilitarySiteDescr* const militarysitedescr)
	   : LuaBuildingDescription(militarysitedescr) {
	}
	explicit LuaMilitarySiteDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_heal_per_second(lua_State*);
	int get_max_number_of_soldiers(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(MilitarySiteDescr)
};

class LuaTrainingSiteDescription : public LuaProductionSiteDescription {
public:
	LUNA_CLASS_HEAD(LuaTrainingSiteDescription);

	~LuaTrainingSiteDescription() override {
	}

	LuaTrainingSiteDescription() {
	}
	explicit LuaTrainingSiteDescription(const Widelands::TrainingSiteDescr* const trainingsitedescr)
	   : LuaProductionSiteDescription(trainingsitedescr) {
	}
	explicit LuaTrainingSiteDescription(lua_State* L) : LuaProductionSiteDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_food_attack(lua_State*);
	int get_food_defense(lua_State*);
	int get_food_evade(lua_State*);
	int get_food_health(lua_State*);
	int get_max_attack(lua_State*);
	int get_max_defense(lua_State*);
	int get_max_evade(lua_State*);
	int get_max_health(lua_State*);
	int get_max_number_of_soldiers(lua_State*);
	int get_min_attack(lua_State*);
	int get_min_defense(lua_State*);
	int get_min_evade(lua_State*);
	int get_min_health(lua_State*);
	int get_weapons_attack(lua_State*);
	int get_weapons_defense(lua_State*);
	int get_weapons_evade(lua_State*);
	int get_weapons_health(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(TrainingSiteDescr)
};

class LuaWarehouseDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaWarehouseDescription);

	~LuaWarehouseDescription() override {
	}

	LuaWarehouseDescription() {
	}
	explicit LuaWarehouseDescription(const Widelands::WarehouseDescr* const warehousedescr)
	   : LuaBuildingDescription(warehousedescr) {
	}
	explicit LuaWarehouseDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_heal_per_second(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(WarehouseDescr)
};

class LuaMarketDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaMarketDescription);

	~LuaMarketDescription() override {
	}

	LuaMarketDescription() {
	}
	explicit LuaMarketDescription(const Widelands::MarketDescr* const warehousedescr)
	   : LuaBuildingDescription(warehousedescr) {
	}
	explicit LuaMarketDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(MarketDescr)
};

class LuaWareDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaWareDescription);

	~LuaWareDescription() override {
	}

	LuaWareDescription() {
	}
	explicit LuaWareDescription(const Widelands::WareDescr* const waredescr)
	   : LuaMapObjectDescription(waredescr) {
	}
	explicit LuaWareDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */
	int consumers(lua_State*);
	int is_construction_material(lua_State*);
	int producers(lua_State*);

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(WareDescr)
};

class LuaWorkerDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaWorkerDescription);

	~LuaWorkerDescription() override {
	}

	LuaWorkerDescription() {
	}
	explicit LuaWorkerDescription(const Widelands::WorkerDescr* const workerdescr)
	   : LuaMapObjectDescription(workerdescr) {
	}
	explicit LuaWorkerDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */
	int get_becomes(lua_State*);
	int get_buildcost(lua_State*);
	int get_employers(lua_State*);
	int get_buildable(lua_State*);
	int get_needed_experience(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(WorkerDescr)
};

class LuaSoldierDescription : public LuaWorkerDescription {
public:
	LUNA_CLASS_HEAD(LuaSoldierDescription);

	~LuaSoldierDescription() override {
	}

	LuaSoldierDescription() {
	}
	explicit LuaSoldierDescription(const Widelands::SoldierDescr* const soldierdescr)
	   : LuaWorkerDescription(soldierdescr) {
	}
	explicit LuaSoldierDescription(lua_State* L) : LuaWorkerDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_max_health_level(lua_State*);
	int get_max_attack_level(lua_State*);
	int get_max_defense_level(lua_State*);
	int get_max_evade_level(lua_State*);
	int get_base_health(lua_State*);
	int get_base_min_attack(lua_State*);
	int get_base_max_attack(lua_State*);
	int get_base_defense(lua_State*);
	int get_base_evade(lua_State*);
	int get_health_incr_per_level(lua_State*);
	int get_attack_incr_per_level(lua_State*);
	int get_defense_incr_per_level(lua_State*);
	int get_evade_incr_per_level(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(SoldierDescr)
};

class LuaShipDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaShipDescription);

	~LuaShipDescription() override {
	}

	LuaShipDescription() {
	}
	explicit LuaShipDescription(const Widelands::ShipDescr* const shipdescr)
	   : LuaMapObjectDescription(shipdescr) {
	}
	explicit LuaShipDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(ShipDescr)
};

#undef CASTED_GET_DESCRIPTION

class LuaResourceDescription : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaResourceDescription);

	~LuaResourceDescription() override {
	}

	LuaResourceDescription() : resourcedescr_(nullptr) {
	}
	explicit LuaResourceDescription(const Widelands::ResourceDescription* const resourcedescr)
	   : resourcedescr_(resourcedescr) {
	}
	explicit LuaResourceDescription(lua_State* L) : resourcedescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'LuaResourceDescription' directly!");
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */
	int get_name(lua_State*);
	int get_descname(lua_State*);
	int get_is_detectable(lua_State*);
	int get_max_amount(lua_State*);
	int get_representative_image(lua_State*);

	/*
	 * Lua methods
	 */

	int editor_image(lua_State*);

	/*
	 * C methods
	 */
protected:
	const Widelands::ResourceDescription* get() const {
		assert(resourcedescr_ != nullptr);
		return resourcedescr_;
	}
	// For persistence.
	void set_description_pointer(const Widelands::ResourceDescription* pointer) {
		resourcedescr_ = pointer;
	}

private:
	const Widelands::ResourceDescription* resourcedescr_;
};

class LuaTerrainDescription : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaTerrainDescription);

	~LuaTerrainDescription() override {
	}

	LuaTerrainDescription() : terraindescr_(nullptr) {
	}
	explicit LuaTerrainDescription(const Widelands::TerrainDescription* const terraindescr)
	   : terraindescr_(terraindescr) {
	}
	explicit LuaTerrainDescription(lua_State* L) : terraindescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'LuaTerrainDescription' directly!");
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */
	int get_name(lua_State*);
	int get_descname(lua_State*);
	int get_default_resource(lua_State*);
	int get_default_resource_amount(lua_State*);
	int get_fertility(lua_State*);
	int get_humidity(lua_State*);
	int get_representative_image(lua_State*);
	int get_temperature(lua_State*);
	int get_valid_resources(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
	const Widelands::TerrainDescription* get() const {
		assert(terraindescr_ != nullptr);
		return terraindescr_;
	}

protected:
	// For persistence.
	void set_description_pointer(const Widelands::TerrainDescription* pointer) {
		terraindescr_ = pointer;
	}

private:
	const Widelands::TerrainDescription* terraindescr_;
};

class LuaEconomy : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaEconomy);

	~LuaEconomy() override {
	}

	LuaEconomy() : economy_(nullptr) {
	}
	explicit LuaEconomy(Widelands::Economy* economy) : economy_(economy) {
	}
	explicit LuaEconomy(lua_State* L) : economy_(nullptr) {
		report_error(L, "Cannot instantiate a 'LuaEconomy' directly!");
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */
	int target_quantity(lua_State*);
	int set_target_quantity(lua_State*);

	/*
	 * C methods
	 */

protected:
	Widelands::Economy* get() const {
		assert(economy_ != nullptr);
		return economy_;
	}
	// For persistence.
	void set_economy_pointer(Widelands::Economy* pointer) {
		economy_ = pointer;
	}

private:
	Widelands::Economy* economy_;
};

#define CASTED_GET(klass)                                                                          \
	Widelands::klass* get(lua_State* L, Widelands::EditorGameBase& egbase) {                        \
		return static_cast<Widelands::klass*>(LuaMapObject::get(L, egbase, #klass));                 \
	}

class LuaMapObject : public LuaMapModuleClass {
	Widelands::ObjectPointer ptr_;

public:
	LUNA_CLASS_HEAD(LuaMapObject);

	LuaMapObject() : ptr_(nullptr) {
	}
	explicit LuaMapObject(Widelands::MapObject& mo) : ptr_(&mo) {
	}
	explicit LuaMapObject(lua_State* L) : ptr_(nullptr) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}
	~LuaMapObject() override {
		ptr_ = nullptr;
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * attributes
	 */
	int get___hash(lua_State*);
	int get_descr(lua_State*);
	int get_serial(lua_State*);

	/*
	 * Lua Methods
	 */
	int __eq(lua_State* L);
	int remove(lua_State* L);
	int destroy(lua_State* L);
	int has_attribute(lua_State* L);

	/*
	 * C Methods
	 */
	Widelands::MapObject*
	get(lua_State*, Widelands::EditorGameBase&, const std::string& = "MapObject");
	Widelands::MapObject* get_or_zero(const Widelands::EditorGameBase&);
};

class LuaBaseImmovable : public LuaMapObject {
public:
	LUNA_CLASS_HEAD(LuaBaseImmovable);

	LuaBaseImmovable() {
	}
	explicit LuaBaseImmovable(Widelands::BaseImmovable& mo) : LuaMapObject(mo) {
	}
	explicit LuaBaseImmovable(lua_State* L) : LuaMapObject(L) {
	}
	~LuaBaseImmovable() override {
	}

	/*
	 * Properties
	 */
	int get_fields(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	CASTED_GET(BaseImmovable)
};

class LuaPlayerImmovable : public LuaBaseImmovable {
public:
	LUNA_CLASS_HEAD(LuaPlayerImmovable);

	LuaPlayerImmovable() {
	}
	explicit LuaPlayerImmovable(Widelands::PlayerImmovable& mo) : LuaBaseImmovable(mo) {
	}
	explicit LuaPlayerImmovable(lua_State* L) : LuaBaseImmovable(L) {
	}
	~LuaPlayerImmovable() override {
	}

	/*
	 * Properties
	 */
	int get_owner(lua_State* L);
	int get_debug_ware_economy(lua_State* L);
	int get_debug_worker_economy(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	CASTED_GET(PlayerImmovable)
};

class LuaPortDock : public LuaPlayerImmovable {
public:
	LUNA_CLASS_HEAD(LuaPortDock);

	LuaPortDock() {
	}
	explicit LuaPortDock(Widelands::PortDock& mo) : LuaPlayerImmovable(mo) {
	}
	explicit LuaPortDock(lua_State* L) : LuaPlayerImmovable(L) {
	}
	~LuaPortDock() override {
	}

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
	CASTED_GET(PortDock)
};

class LuaBuilding : public LuaPlayerImmovable {
public:
	LUNA_CLASS_HEAD(LuaBuilding);

	LuaBuilding() {
	}
	explicit LuaBuilding(Widelands::Building& mo) : LuaPlayerImmovable(mo) {
	}
	explicit LuaBuilding(lua_State* L) : LuaPlayerImmovable(L) {
	}
	~LuaBuilding() override {
	}

	/*
	 * Properties
	 */
	int get_flag(lua_State* L);
	int get_destruction_blocked(lua_State* L);
	int set_destruction_blocked(lua_State* L);

	/*
	 * Lua Methods
	 */
	int dismantle(lua_State* L);
	int enhance(lua_State* L);

	/*
	 * C Methods
	 */
	CASTED_GET(Building)
};

class LuaFlag : public LuaPlayerImmovable {
public:
	LUNA_CLASS_HEAD(LuaFlag);

	LuaFlag() {
	}
	explicit LuaFlag(Widelands::Flag& mo) : LuaPlayerImmovable(mo) {
	}
	explicit LuaFlag(lua_State* L) : LuaPlayerImmovable(L) {
	}
	~LuaFlag() override {
	}

	/*
	 * Properties
	 */
	int get_ware_economy(lua_State* L);
	int get_worker_economy(lua_State* L);
	int get_roads(lua_State* L);
	int get_building(lua_State* L);
	/*
	 * Lua Methods
	 */
	int set_wares(lua_State*);
	int get_wares(lua_State*);
	int get_distance(lua_State*);
	int send_geologist(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(Flag)
};

class LuaRoad : public LuaPlayerImmovable {
public:
	LUNA_CLASS_HEAD(LuaRoad);

	LuaRoad() {
	}
	explicit LuaRoad(Widelands::RoadBase& mo) : LuaPlayerImmovable(mo) {
	}
	explicit LuaRoad(lua_State* L) : LuaPlayerImmovable(L) {
	}
	~LuaRoad() override {
	}

	/*
	 * Properties
	 */
	int get_end_flag(lua_State* L);
	int get_length(lua_State* L);
	int get_road_type(lua_State* L);
	int get_start_flag(lua_State* L);
	int get_valid_workers(lua_State* L);

	/*
	 * Lua Methods
	 */
	int get_workers(lua_State* L);
	int set_workers(lua_State* L);

	/*
	 * C Methods
	 */
	CASTED_GET(RoadBase)
	static bool create_new_worker(lua_State* L,
	                              Widelands::RoadBase& r,
	                              Widelands::EditorGameBase&,
	                              const Widelands::WorkerDescr*);
};

class LuaConstructionSite : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaConstructionSite);

	LuaConstructionSite() {
	}
	explicit LuaConstructionSite(Widelands::ConstructionSite& mo) : LuaBuilding(mo) {
	}
	explicit LuaConstructionSite(lua_State* L) : LuaBuilding(L) {
	}
	~LuaConstructionSite() override {
	}

	/*
	 * Properties
	 */
	int get_building(lua_State*);
	int get_has_builder(lua_State*);
	int set_has_builder(lua_State*);
	int get_setting_soldier_capacity(lua_State*);
	int set_setting_soldier_capacity(lua_State*);
	int get_setting_soldier_preference(lua_State*);
	int set_setting_soldier_preference(lua_State*);
	int get_setting_launch_expedition(lua_State*);
	int set_setting_launch_expedition(lua_State*);
	int get_setting_stopped(lua_State*);
	int set_setting_stopped(lua_State*);

	/*
	 * Lua Methods
	 */
	int get_priority(lua_State*);
	int set_priority(lua_State*);
	int get_desired_fill(lua_State*);
	int set_desired_fill(lua_State*);
	int get_setting_warehouse_policy(lua_State*);
	int set_setting_warehouse_policy(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(ConstructionSite)
};

class LuaDismantleSite : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaDismantleSite);

	LuaDismantleSite() {
	}
	explicit LuaDismantleSite(Widelands::DismantleSite& mo) : LuaBuilding(mo) {
	}
	explicit LuaDismantleSite(lua_State* L) : LuaBuilding(L) {
	}
	~LuaDismantleSite() override {
	}

	/*
	 * Properties
	 */
	int get_has_builder(lua_State*);
	int set_has_builder(lua_State*);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	CASTED_GET(DismantleSite)
};

class LuaWarehouse : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaWarehouse);

	LuaWarehouse() {
	}
	explicit LuaWarehouse(Widelands::Warehouse& mo) : LuaBuilding(mo) {
	}
	explicit LuaWarehouse(lua_State* L) : LuaBuilding(L) {
	}
	~LuaWarehouse() override {
	}

	/*
	 * Properties
	 */
	int get_portdock(lua_State* L);
	int get_expedition_in_progress(lua_State* L);

	/*
	 * Lua Methods
	 */
	int get_wares(lua_State*);
	int get_workers(lua_State*);
	int set_wares(lua_State*);
	int set_workers(lua_State*);
	int set_soldiers(lua_State*);
	int get_soldiers(lua_State*);
	int set_warehouse_policies(lua_State*);
	int get_warehouse_policies(lua_State*);
	int start_expedition(lua_State*);
	int cancel_expedition(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(Warehouse)
};

class LuaMarket : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaMarket);

	LuaMarket() {
	}
	explicit LuaMarket(Widelands::Market& mo) : LuaBuilding(mo) {
	}
	explicit LuaMarket(lua_State* L) : LuaBuilding(L) {
	}
	~LuaMarket() override {
	}

	/*
	 * Properties
	 */

	/*
	 * Lua Methods
	 */
	int propose_trade(lua_State* L);

	/*
	 * C Methods
	 */
	CASTED_GET(Market)
};

class LuaProductionSite : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaProductionSite);

	LuaProductionSite() {
	}
	explicit LuaProductionSite(Widelands::ProductionSite& mo) : LuaBuilding(mo) {
	}
	explicit LuaProductionSite(lua_State* L) : LuaBuilding(L) {
	}
	~LuaProductionSite() override {
	}

	/*
	 * Properties
	 */
	int get_valid_inputs(lua_State* L);
	int get_valid_workers(lua_State* L);
	int get_is_stopped(lua_State* L);
	int get_productivity(lua_State* L);

	/*
	 * Lua Methods
	 */
	int get_inputs(lua_State* L);
	int get_workers(lua_State* L);
	int set_inputs(lua_State* L);
	int set_workers(lua_State* L);
	int toggle_start_stop(lua_State* L);
	int get_priority(lua_State*);
	int set_priority(lua_State*);
	int get_desired_fill(lua_State*);
	int set_desired_fill(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(ProductionSite)
	static bool create_new_worker(lua_State* L,
	                              Widelands::ProductionSite& ps,
	                              Widelands::EditorGameBase&,
	                              const Widelands::WorkerDescr*);
};

class LuaMilitarySite : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaMilitarySite);

	LuaMilitarySite() {
	}
	explicit LuaMilitarySite(Widelands::MilitarySite& mo) : LuaBuilding(mo) {
	}
	explicit LuaMilitarySite(lua_State* L) : LuaBuilding(L) {
	}
	~LuaMilitarySite() override {
	}

	/*
	 * Properties
	 */
	int get_max_soldiers(lua_State*);
	int get_soldier_preference(lua_State*);
	int set_soldier_preference(lua_State*);
	int get_capacity(lua_State*);
	int set_capacity(lua_State*);

	/*
	 * Lua Methods
	 */
	int set_soldiers(lua_State*);
	int get_soldiers(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(MilitarySite)
};

class LuaTrainingSite : public LuaProductionSite {
public:
	LUNA_CLASS_HEAD(LuaTrainingSite);

	LuaTrainingSite() {
	}
	explicit LuaTrainingSite(Widelands::TrainingSite& mo) : LuaProductionSite(mo) {
	}
	explicit LuaTrainingSite(lua_State* L) : LuaProductionSite(L) {
	}
	~LuaTrainingSite() override {
	}

	/*
	 * Properties
	 */
	int get_max_soldiers(lua_State*);
	int get_capacity(lua_State*);
	int set_capacity(lua_State*);

	/*
	 * Lua Methods
	 */
	int set_soldiers(lua_State*);
	int get_soldiers(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(TrainingSite)
};

class LuaBob : public LuaMapObject {
public:
	LUNA_CLASS_HEAD(LuaBob);

	LuaBob() {
	}
	explicit LuaBob(Widelands::Bob& mo) : LuaMapObject(mo) {
	}
	explicit LuaBob(lua_State* L) : LuaMapObject(L) {
	}
	~LuaBob() override {
	}

	/*
	 * Properties
	 */
	int get_field(lua_State*);
	int has_caps(lua_State*);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	CASTED_GET(Bob)
};

class LuaWorker : public LuaBob {
public:
	LUNA_CLASS_HEAD(LuaWorker);

	LuaWorker() {
	}
	explicit LuaWorker(Widelands::Worker& w) : LuaBob(w) {
	}
	explicit LuaWorker(lua_State* L) : LuaBob(L) {
	}
	~LuaWorker() override {
	}

	/*
	 * Properties
	 */
	int get_owner(lua_State* L);
	int get_location(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
	CASTED_GET(Worker)
};

class LuaSoldier : public LuaWorker {
public:
	LUNA_CLASS_HEAD(LuaSoldier);

	LuaSoldier() {
	}
	explicit LuaSoldier(Widelands::Soldier& w) : LuaWorker(w) {
	}
	explicit LuaSoldier(lua_State* L) : LuaWorker(L) {
	}
	~LuaSoldier() override {
	}

	/*
	 * Properties
	 */
	int get_attack_level(lua_State*);
	int get_defense_level(lua_State*);
	int get_health_level(lua_State*);
	int get_evade_level(lua_State*);
	int get_current_health(lua_State*);
	int set_current_health(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
	CASTED_GET(Soldier)
};

class LuaShip : public LuaBob {
public:
	LUNA_CLASS_HEAD(LuaShip);

	LuaShip() {
	}
	explicit LuaShip(Widelands::Ship& s) : LuaBob(s) {
	}
	explicit LuaShip(lua_State* L) : LuaBob(L) {
	}
	~LuaShip() override {
	}

	/*
	 * Properties
	 */
	int get_debug_ware_economy(lua_State* L);
	int get_debug_worker_economy(lua_State* L);
	int get_last_portdock(lua_State* L);
	int get_destination(lua_State* L);
	int get_state(lua_State* L);
	int get_scouting_direction(lua_State* L);
	int set_scouting_direction(lua_State* L);
	int get_island_explore_direction(lua_State* L);
	int set_island_explore_direction(lua_State* L);
	int get_shipname(lua_State* L);
	int get_capacity(lua_State* L);
	int set_capacity(lua_State* L);
	/*
	 * Lua methods
	 */
	int get_wares(lua_State* L);
	int get_workers(lua_State* L);
	int build_colonization_port(lua_State* L);
	int make_expedition(lua_State* L);

	/*
	 * C methods
	 */
	CASTED_GET(Ship)
};
#undef CASTED_GET

class LuaField : public LuaMapModuleClass {
	Widelands::Coords coords_;

public:
	LUNA_CLASS_HEAD(LuaField);

	LuaField() {
	}
	explicit LuaField(int16_t x, int16_t y) : coords_(Widelands::Coords(x, y)) {
	}
	explicit LuaField(Widelands::Coords c) : coords_(c) {
	}
	explicit LuaField(lua_State* L) {
		report_error(L, "Cannot instantiate a 'Field' directly!");
	}
	~LuaField() override {
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */
	int get___hash(lua_State*);
	int get_x(lua_State* L);
	int get_y(lua_State* L);
	int get_viewpoint_x(lua_State* L);
	int get_viewpoint_y(lua_State* L);
	int get_height(lua_State* L);
	int set_height(lua_State* L);
	int get_raw_height(lua_State* L);
	int set_raw_height(lua_State* L);
	int get_immovable(lua_State* L);
	int get_bobs(lua_State* L);
	int get_terr(lua_State* L);
	int set_terr(lua_State* L);
	int get_terd(lua_State* L);
	int set_terd(lua_State* L);
	int get_rn(lua_State*);
	int get_ln(lua_State*);
	int get_trn(lua_State*);
	int get_tln(lua_State*);
	int get_bln(lua_State*);
	int get_brn(lua_State*);
	int get_resource(lua_State*);
	int set_resource(lua_State*);
	int get_resource_amount(lua_State*);
	int set_resource_amount(lua_State*);
	int get_initial_resource_amount(lua_State*);
	int get_claimers(lua_State*);
	int get_owner(lua_State*);
	int get_buildable(lua_State*);
	int get_has_roads(lua_State* L);

	/*
	 * Lua methods
	 */
	int __tostring(lua_State* L);
	int __eq(lua_State* L);
	int region(lua_State* L);
	int has_caps(lua_State*);
	int has_max_caps(lua_State*);
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	int indicate(lua_State* L);
#endif

	/*
	 * C methods
	 */
	inline const Widelands::Coords& coords() {
		return coords_;
	}
	const Widelands::FCoords fcoords(lua_State* L);

private:
	int region(lua_State* L, uint32_t radius);
	int hollow_region(lua_State* L, uint32_t radius, uint32_t inner_radius);
};

class LuaPlayerSlot : public LuaMapModuleClass {
	Widelands::PlayerNumber player_number_;

public:
	LUNA_CLASS_HEAD(LuaPlayerSlot);

	LuaPlayerSlot() : player_number_(0) {
	}
	explicit LuaPlayerSlot(Widelands::PlayerNumber plr) : player_number_(plr) {
	}
	explicit LuaPlayerSlot(lua_State* L) : player_number_(0) {
		report_error(L, "Cannot instantiate a 'PlayerSlot' directly!");
	}
	~LuaPlayerSlot() override {
	}

	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;

	/*
	 * Properties
	 */
	int get_tribe_name(lua_State*);
	int get_name(lua_State*);
	int get_starting_field(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
};

using InputMap =
   std::map<std::pair<Widelands::DescriptionIndex, Widelands::WareWorker>, Widelands::Quantity>;

int upcasted_map_object_descr_to_lua(lua_State* L, const Widelands::MapObjectDescr* descr);
int upcasted_map_object_to_lua(lua_State* L, Widelands::MapObject* mo);
RequestedWareWorker parse_wares_workers_list(lua_State*,
                                             const Widelands::TribeDescr&,
                                             Widelands::DescriptionIndex*,
                                             std::vector<Widelands::DescriptionIndex>*,
                                             bool is_ware);
RequestedWareWorker parse_wares_workers_counted(lua_State*,
                                                const Widelands::TribeDescr&,
                                                LuaMaps::InputMap*,
                                                bool is_ware);
void luaopen_wlmap(lua_State*);

}  // namespace LuaMaps

#endif  // end of include guard: WL_SCRIPTING_LUA_MAP_H
