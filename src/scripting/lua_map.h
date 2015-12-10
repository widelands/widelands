/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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

#include <set>

#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/road.h"
#include "logic/constructionsite.h"
#include "logic/dismantlesite.h"
#include "logic/game.h"
#include "logic/militarysite.h"
#include "logic/productionsite.h"
#include "logic/ship.h"
#include "logic/soldier.h"
#include "logic/trainingsite.h"
#include "logic/warehouse.h"
#include "logic/worker.h"
#include "logic/world/terrain_description.h"
#include "scripting/lua.h"
#include "scripting/luna.h"


namespace Widelands {
	class SoldierDescr;
	class BuildingDescr;
	class Bob;
	class WareDescr;
	class WorkerDescr;
	class TerrainDescription;
	class TribeDescr;
}

namespace LuaMaps {

/*
 * Base class for all classes in wl.map
 */
class LuaMapModuleClass : public LunaClass {
	public:
		const char * get_modulename() override {return "map";}
};


class LuaMap : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaMap);

	virtual ~LuaMap() {}

	LuaMap() {}
	LuaMap(lua_State * L) {
		report_error(L, "Cannot instantiate a 'Map' directly!");
	}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_width(lua_State *);
	int get_height(lua_State *);
	int get_player_slots(lua_State *);

	/*
	 * Lua methods
	 */
	int place_immovable(lua_State *);
	int get_field(lua_State *);
	int recalculate(lua_State *);

	/*
	 * C methods
	 */
private:
};


class LuaTribeDescription : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaTribeDescription);

	virtual ~LuaTribeDescription() {}

	LuaTribeDescription() : tribedescr_(nullptr) {}
	LuaTribeDescription(const Widelands::TribeDescr* const tribedescr)
		: tribedescr_(tribedescr) {}
	LuaTribeDescription(lua_State* L) : tribedescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'LuaTribeDescription' directly!");
	}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_buildings(lua_State *);
	int get_carrier(lua_State *);
	int get_carrier2(lua_State *);
	int get_descname(lua_State *);
	int get_headquarters(lua_State *);
	int get_geologist(lua_State *);
	int get_name(lua_State *);
	int get_port(lua_State *);
	int get_ship(lua_State *);
	int get_soldier(lua_State *);
	int get_wares(lua_State *);
	int get_workers(lua_State *);

	/*
	 * Lua methods
	 */
	int has_building(lua_State *);
	int has_ware(lua_State *);
	int has_worker(lua_State *);

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

	virtual ~LuaMapObjectDescription() {}

	LuaMapObjectDescription() : mapobjectdescr_(nullptr) {}
	LuaMapObjectDescription(const Widelands::MapObjectDescr* const mapobjectdescr)
		: mapobjectdescr_(mapobjectdescr) {}
	LuaMapObjectDescription(lua_State* L) : mapobjectdescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'MapObjectDescription' directly!");
	}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_descname(lua_State *);
	int get_icon_name(lua_State*);
	int get_name(lua_State *);
	int get_type_name(lua_State *);
	int get_representative_image(lua_State *);


	/*
	 * Lua methods
	 */

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
		return static_cast<const Widelands::klass*>(LuaMapObjectDescription::get());                  \
	}

class LuaImmovableDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaImmovableDescription);

	virtual ~LuaImmovableDescription() {}

	LuaImmovableDescription() {}
	LuaImmovableDescription(const Widelands::ImmovableDescr* const immovabledescr)
		: LuaMapObjectDescription(immovabledescr) {
	}
	LuaImmovableDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_build_cost(lua_State *);
	int get_editor_category(lua_State *);
	int get_has_terrain_affinity(lua_State *);
	int get_pickiness(lua_State *);
	int get_preferred_fertility(lua_State *);
	int get_preferred_humidity(lua_State *);
	int get_preferred_temperature(lua_State *);
	int get_owner_type(lua_State *);
	int get_size(lua_State *);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(ImmovableDescr)
};


class LuaBuildingDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaBuildingDescription);

	virtual ~LuaBuildingDescription() {}

	LuaBuildingDescription() {}
	LuaBuildingDescription(const Widelands::BuildingDescr* const buildingdescr)
		: LuaMapObjectDescription(buildingdescr) {
	}
	LuaBuildingDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_build_cost(lua_State *);
	int get_buildable(lua_State *);
	int get_conquers(lua_State *);
	int get_destructible(lua_State *);
	int get_helptext_script(lua_State *);
	int get_enhanced(lua_State *);
	int get_enhanced_from(lua_State *);
	int get_enhancement_cost(lua_State *);
	int get_enhancement(lua_State *);
	int get_is_mine(lua_State *);
	int get_is_port(lua_State *);
	int get_isproductionsite(lua_State *);
	int get_returned_wares(lua_State *);
	int get_returned_wares_enhanced(lua_State *);
	int get_size(lua_State *);
	int get_vision_range(lua_State *);
	int get_workarea_radius(lua_State *);

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

	virtual ~LuaConstructionSiteDescription() {}

	LuaConstructionSiteDescription() {}
	LuaConstructionSiteDescription(const Widelands::ConstructionSiteDescr* const constructionsitedescr)
		: LuaBuildingDescription(constructionsitedescr) {
	}
	LuaConstructionSiteDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

private:
	CASTED_GET_DESCRIPTION(ConstructionSiteDescr)
};

class LuaDismantleSiteDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaDismantleSiteDescription);

	virtual ~LuaDismantleSiteDescription() {}

	LuaDismantleSiteDescription() {}
	LuaDismantleSiteDescription(const Widelands::DismantleSiteDescr* const dismantlesitedescr)
		: LuaBuildingDescription(dismantlesitedescr) {
	}
	LuaDismantleSiteDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

private:
	CASTED_GET_DESCRIPTION(DismantleSiteDescr)
};



class LuaProductionSiteDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaProductionSiteDescription);

	virtual ~LuaProductionSiteDescription() {}

	LuaProductionSiteDescription() {}
	LuaProductionSiteDescription(const Widelands::ProductionSiteDescr* const productionsitedescr)
		: LuaBuildingDescription(productionsitedescr) {
	}
	LuaProductionSiteDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_inputs(lua_State *);
	int get_output_ware_types(lua_State *);
	int get_output_worker_types(lua_State *);
	int get_production_programs(lua_State *);
	int get_working_positions(lua_State *);

	/*
	 * Lua methods
	 */

	int consumed_wares(lua_State *);
	int produced_wares(lua_State *);
	int recruited_workers(lua_State *);

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(ProductionSiteDescr)
};


class LuaMilitarySiteDescription : public LuaProductionSiteDescription {
public:
	LUNA_CLASS_HEAD(LuaMilitarySiteDescription);

	virtual ~LuaMilitarySiteDescription() {}

	LuaMilitarySiteDescription() {}
	LuaMilitarySiteDescription(const Widelands::ProductionSiteDescr* const militarysitedescr)
		: LuaProductionSiteDescription(militarysitedescr) {
	}
	LuaMilitarySiteDescription(lua_State* L) : LuaProductionSiteDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_heal_per_second(lua_State *);
	int get_max_number_of_soldiers(lua_State *);

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

	virtual ~LuaTrainingSiteDescription() {}

	LuaTrainingSiteDescription() {}
	LuaTrainingSiteDescription(const Widelands::ProductionSiteDescr* const trainingsitedescr)
		: LuaProductionSiteDescription(trainingsitedescr) {
	}
	LuaTrainingSiteDescription(lua_State* L) : LuaProductionSiteDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_food_attack(lua_State *);
	int get_food_defense(lua_State *);
	int get_food_evade(lua_State *);
	int get_food_hp(lua_State *);
	int get_max_attack(lua_State *);
	int get_max_defense(lua_State *);
	int get_max_evade(lua_State *);
	int get_max_hp(lua_State *);
	int get_max_number_of_soldiers(lua_State *);
	int get_min_attack(lua_State *);
	int get_min_defense(lua_State *);
	int get_min_evade(lua_State *);
	int get_min_hp(lua_State *);
	int get_weapons_attack(lua_State *);
	int get_weapons_defense(lua_State *);
	int get_weapons_evade(lua_State *);
	int get_weapons_hp(lua_State *);

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

	virtual ~LuaWarehouseDescription() {}

	LuaWarehouseDescription() {}
	LuaWarehouseDescription(const Widelands::WarehouseDescr* const warehousedescr)
		: LuaBuildingDescription(warehousedescr) {
	}
	LuaWarehouseDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_heal_per_second(lua_State *);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(WarehouseDescr)
};



class LuaWareDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaWareDescription);

	virtual ~LuaWareDescription() {}

	LuaWareDescription()  {}
	LuaWareDescription(const Widelands::WareDescr* const waredescr)
		: LuaMapObjectDescription(waredescr) {}
	LuaWareDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_consumers(lua_State *);
	int get_helptext_script(lua_State*);
	int get_producers(lua_State *);

	/*
	 * Lua methods
	 */
	int is_construction_material(lua_State *);

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(WareDescr)
};


class LuaWorkerDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaWorkerDescription);

	virtual ~LuaWorkerDescription() {}

	LuaWorkerDescription() {}
	LuaWorkerDescription(const Widelands::WorkerDescr* const workerdescr)
		: LuaMapObjectDescription(workerdescr) {
	}
	LuaWorkerDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_becomes(lua_State*);
	int get_buildcost(lua_State*);
	int get_helptext_script(lua_State*);
	int get_is_buildable(lua_State*);
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

#undef CASTED_GET_DESCRIPTION


class LuaTerrainDescription : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaTerrainDescription);

	virtual ~LuaTerrainDescription() {}

	LuaTerrainDescription() : terraindescr_(nullptr) {}
	LuaTerrainDescription(const Widelands::TerrainDescription* const terraindescr)
		: terraindescr_(terraindescr) {}
	LuaTerrainDescription(lua_State* L) : terraindescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'LuaTerrainDescription' directly!");
	}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_name(lua_State *);
	int get_descname(lua_State *);
	int get_representative_image(lua_State *);

	/*
	 * Lua methods
	 */
	int probability_to_grow(lua_State *);

	/*
	 * C methods
	 */
protected:
	const Widelands::TerrainDescription* get() const {
		assert(terraindescr_ != nullptr);
		return terraindescr_;
	}
	// For persistence.
	void set_description_pointer(const Widelands::TerrainDescription* pointer) {
		terraindescr_ = pointer;
	}

private:
	const Widelands::TerrainDescription* terraindescr_;
};


#define CASTED_GET(klass) \
Widelands:: klass * get(lua_State * L, Widelands::EditorGameBase & egbase) { \
	return static_cast<Widelands:: klass *> \
		(LuaMapObject::get(L, egbase, #klass)); \
}

class LuaMapObject : public LuaMapModuleClass {
	Widelands::ObjectPointer m_ptr;

public:
	LUNA_CLASS_HEAD(LuaMapObject);

	LuaMapObject() : m_ptr(nullptr) {}
	LuaMapObject(Widelands::MapObject & mo) {
		m_ptr = &mo;
	}
	LuaMapObject(lua_State * L) : m_ptr(nullptr) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}
	virtual ~LuaMapObject() {
		m_ptr = nullptr;
	}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * attributes
	 */
	int get___hash(lua_State *);
	int get_descr(lua_State *);
	int get_serial(lua_State *);

	/*
	 * Lua Methods
	 */
	int __eq(lua_State * L);
	int remove(lua_State * L);
	int destroy(lua_State * L);
	int has_attribute(lua_State * L);

	/*
	 * C Methods
	 */
	Widelands::MapObject * get
		(lua_State *, Widelands::EditorGameBase &, std::string = "MapObject");
	Widelands::MapObject * m_get_or_zero(Widelands::EditorGameBase &);
};


class LuaBaseImmovable : public LuaMapObject {
public:
	LUNA_CLASS_HEAD(LuaBaseImmovable);

	LuaBaseImmovable() {}
	LuaBaseImmovable(Widelands::BaseImmovable & mo) : LuaMapObject(mo) {}
	LuaBaseImmovable(lua_State * L) : LuaMapObject(L) {}
	virtual ~LuaBaseImmovable() {}

	/*
	 * Properties
	 */
	int get_size(lua_State * L);
	int get_fields(lua_State * L);

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

	LuaPlayerImmovable() {}
	LuaPlayerImmovable(Widelands::PlayerImmovable & mo) : LuaBaseImmovable(mo) {
	}
	LuaPlayerImmovable(lua_State * L) : LuaBaseImmovable(L) {}
	virtual ~LuaPlayerImmovable() {}

	/*
	 * Properties
	 */
	int get_owner(lua_State * L);
	int get_debug_economy(lua_State* L);

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

	LuaPortDock() {}
	LuaPortDock(Widelands::PortDock & mo) : LuaPlayerImmovable(mo) {
	}
	LuaPortDock(lua_State * L) : LuaPlayerImmovable(L) {}
	virtual ~LuaPortDock() {}

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

	LuaBuilding() {}
	LuaBuilding(Widelands::Building & mo) : LuaPlayerImmovable(mo) {
	}
	LuaBuilding(lua_State * L) : LuaPlayerImmovable(L) {}
	virtual ~LuaBuilding() {}

	/*
	 * Properties
	 */
	int get_flag(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	CASTED_GET(Building)
};

class LuaFlag : public LuaPlayerImmovable {
public:
	LUNA_CLASS_HEAD(LuaFlag);

	LuaFlag() {}
	LuaFlag(Widelands::Flag & mo) : LuaPlayerImmovable(mo) {
	}
	LuaFlag(lua_State * L) : LuaPlayerImmovable(L) {}
	virtual ~LuaFlag() {}

	/*
	 * Properties
	 */
	int get_roads(lua_State * L);
	int get_building(lua_State * L);
	/*
	 * Lua Methods
	 */
	int set_wares(lua_State *);
	int get_wares(lua_State *);


	/*
	 * C Methods
	 */
	CASTED_GET(Flag)
};

class LuaRoad : public LuaPlayerImmovable {
public:
	LUNA_CLASS_HEAD(LuaRoad);

	LuaRoad() {}
	LuaRoad(Widelands::Road & mo) : LuaPlayerImmovable(mo) {
	}
	LuaRoad(lua_State * L) : LuaPlayerImmovable(L) {}
	virtual ~LuaRoad() {}

	/*
	 * Properties
	 */
	int get_end_flag(lua_State * L);
	int get_length(lua_State * L);
	int get_road_type(lua_State * L);
	int get_start_flag(lua_State * L);
	int get_valid_workers(lua_State * L);

	/*
	 * Lua Methods
	 */
	int get_workers(lua_State* L);
	int set_workers(lua_State* L);

	/*
	 * C Methods
	 */
	CASTED_GET(Road)
	static int create_new_worker
			(Widelands::PlayerImmovable &,
			 Widelands::EditorGameBase &, const Widelands::WorkerDescr *);
};


class LuaConstructionSite : public LuaBuilding
{
public:
	LUNA_CLASS_HEAD(LuaConstructionSite);

	LuaConstructionSite() {}
	LuaConstructionSite(Widelands::ConstructionSite & mo) : LuaBuilding(mo) {
	}
	LuaConstructionSite(lua_State * L) : LuaBuilding(L) {}
	virtual ~LuaConstructionSite() {}

	/*
	 * Properties
	 */
	int get_building(lua_State *);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	CASTED_GET(ConstructionSite)
};


class LuaWarehouse : public LuaBuilding
{
public:
	LUNA_CLASS_HEAD(LuaWarehouse);

	LuaWarehouse() {}
	LuaWarehouse(Widelands::Warehouse & mo) : LuaBuilding(mo) {
	}
	LuaWarehouse(lua_State * L) : LuaBuilding(L) {}
	virtual ~LuaWarehouse() {}

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
	int start_expedition(lua_State*);
	int cancel_expedition(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(Warehouse)
};


class LuaProductionSite : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaProductionSite);

	LuaProductionSite() {}
	LuaProductionSite(Widelands::ProductionSite & mo) : LuaBuilding(mo) {
	}
	LuaProductionSite(lua_State * L) : LuaBuilding(L) {}
	virtual ~LuaProductionSite() {}

	/*
	 * Properties
	 */
	int get_valid_wares(lua_State * L);
	int get_valid_workers(lua_State* L);

	/*
	 * Lua Methods
	 */
	int get_wares(lua_State * L);
	int get_workers(lua_State* L);
	int set_wares(lua_State * L);
	int set_workers(lua_State* L);

	/*
	 * C Methods
	 */
	CASTED_GET(ProductionSite)
	static int create_new_worker
		(Widelands::PlayerImmovable &, Widelands::EditorGameBase &,
		 const Widelands::WorkerDescr *);
};

class LuaMilitarySite : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaMilitarySite);

	LuaMilitarySite() {}
	LuaMilitarySite(Widelands::MilitarySite & mo) : LuaBuilding(mo) {
	}
	LuaMilitarySite(lua_State * L) : LuaBuilding(L) {}
	virtual ~LuaMilitarySite() {}

	/*
	 * Properties
	 */
	int get_max_soldiers(lua_State*);

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

	LuaTrainingSite() {}
	LuaTrainingSite(Widelands::TrainingSite & mo) : LuaProductionSite(mo) {
	}
	LuaTrainingSite(lua_State * L) : LuaProductionSite(L) {}
	virtual ~LuaTrainingSite() {}

	/*
	 * Properties
	 */
	int get_max_soldiers(lua_State*);

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

	LuaBob() {}
	LuaBob(Widelands::Bob & mo) : LuaMapObject(mo) {}
	LuaBob(lua_State * L) : LuaMapObject(L) {}
	virtual ~LuaBob() {}

	/*
	 * Properties
	 */
	int get_field(lua_State *);
	int has_caps(lua_State *);

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

	LuaWorker() {}
	LuaWorker(Widelands::Worker & w) : LuaBob(w) {}
	LuaWorker(lua_State * L) : LuaBob(L) {}
	virtual ~LuaWorker() {}

	/*
	 * Properties
	 */
	int get_owner(lua_State * L);
	int get_location(lua_State *);

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

	LuaSoldier() {}
	LuaSoldier(Widelands::Soldier & w) : LuaWorker(w) {}
	LuaSoldier(lua_State * L) : LuaWorker(L) {}
	virtual ~LuaSoldier() {}

	/*
	 * Properties
	 */
	int get_attack_level(lua_State *);
	int get_defense_level(lua_State *);
	int get_hp_level(lua_State *);
	int get_evade_level(lua_State *);

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

	LuaShip() {}
	LuaShip(Widelands::Ship& s) : LuaBob(s) {}
	LuaShip(lua_State* L) : LuaBob(L) {}
	virtual ~LuaShip() {}

	/*
	 * Properties
	 */
	int get_debug_economy(lua_State * L);
	int get_last_portdock(lua_State* L);
	int get_destination(lua_State* L);
	int get_state(lua_State* L);
	int get_scouting_direction(lua_State* L);
	int set_scouting_direction(lua_State* L);
	int get_island_explore_direction(lua_State* L);
	int set_island_explore_direction(lua_State* L);
	/*
	 * Lua methods
	 */
	int get_wares(lua_State* L);
	int get_workers(lua_State* L);
	int build_colonization_port(lua_State* L);

	/*
	 * C methods
	 */
	CASTED_GET(Ship)
};
#undef CASTED_GET

class LuaField : public LuaMapModuleClass {
	Widelands::Coords m_c;
public:
	LUNA_CLASS_HEAD(LuaField);

	LuaField() {}
	LuaField (int16_t x, int16_t y) :
		m_c(Widelands::Coords(x, y)) {}
	LuaField (Widelands::Coords c) : m_c(c) {}
	LuaField(lua_State * L) {
		report_error(L, "Cannot instantiate a 'Field' directly!");
	}
	virtual ~LuaField() {}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get___hash(lua_State *);
	int get_x(lua_State * L);
	int get_y(lua_State * L);
	int get_viewpoint_x(lua_State * L);
	int get_viewpoint_y(lua_State * L);
	int get_height(lua_State * L);
	int set_height(lua_State * L);
	int get_raw_height(lua_State * L);
	int set_raw_height(lua_State * L);
	int get_immovable(lua_State * L);
	int get_bobs(lua_State * L);
	int get_terr(lua_State * L);
	int set_terr(lua_State * L);
	int get_terd(lua_State * L);
	int set_terd(lua_State * L);
	int get_rn(lua_State *);
	int get_ln(lua_State *);
	int get_trn(lua_State *);
	int get_tln(lua_State *);
	int get_bln(lua_State *);
	int get_brn(lua_State *);
	int get_resource(lua_State *);
	int set_resource(lua_State *);
	int get_resource_amount(lua_State *);
	int set_resource_amount(lua_State *);
	int get_initial_resource_amount(lua_State *);
	int get_claimers(lua_State *);
	int get_owner(lua_State *);

	/*
	 * Lua methods
	 */
	int __tostring(lua_State * L);
	int __eq(lua_State * L);
	int region(lua_State * L);
	int has_caps(lua_State *);

	/*
	 * C methods
	 */
	inline const Widelands::Coords & coords() {return m_c;}
	const Widelands::FCoords fcoords(lua_State * L);

private:
	int m_region(lua_State * L, uint32_t radius);
	int m_hollow_region(lua_State * L, uint32_t radius, uint32_t inner_radius);
};

class LuaPlayerSlot : public LuaMapModuleClass {
	Widelands::PlayerNumber m_plr;

public:
	LUNA_CLASS_HEAD(LuaPlayerSlot);

	LuaPlayerSlot() : m_plr(0) {}
	LuaPlayerSlot(Widelands::PlayerNumber plr) : m_plr(plr) {}
	LuaPlayerSlot(lua_State * L) : m_plr(0) {
		report_error(L, "Cannot instantiate a 'PlayerSlot' directly!");
	}
	virtual ~LuaPlayerSlot() {}

	void __persist(lua_State * L) override;
	void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_tribe_name(lua_State *);
	int get_name(lua_State *);
	int get_starting_field(lua_State *);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
};

int upcasted_map_object_descr_to_lua(lua_State* L, const Widelands::MapObjectDescr* descr);
int upcasted_map_object_to_lua(lua_State * L, Widelands::MapObject * mo);

void luaopen_wlmap(lua_State *);

}  // namespace LuaMap

#endif  // end of include guard: WL_SCRIPTING_LUA_MAP_H
