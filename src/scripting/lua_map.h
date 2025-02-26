/*
 * Copyright (C) 2006-2025 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_LUA_MAP_H
#define WL_SCRIPTING_LUA_MAP_H

#include <map>

#include "logic/map.h"
#include "logic/map_objects/tribes/bill_of_materials.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "scripting/lua.h"
#include "scripting/luna.h"

namespace Widelands {
class MapObject;
class MapObjectDescr;
class ProductionSite;
struct RoadBase;
class Soldier;
class SoldierDescr;
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

struct SoldierMapDescr {
	SoldierMapDescr(uint8_t init_health,
	                uint8_t init_attack,
	                uint8_t init_defense,
	                uint8_t init_evade)
	   : health(init_health), attack(init_attack), defense(init_defense), evade(init_evade) {
	}
	SoldierMapDescr() = default;

	uint8_t health = 0;
	uint8_t attack = 0;
	uint8_t defense = 0;
	uint8_t evade = 0;

	bool operator<(const SoldierMapDescr& ot) const;
	bool operator==(const SoldierMapDescr& ot) const;
};

using InputMap =
   std::map<std::pair<Widelands::DescriptionIndex, Widelands::WareWorker>, Widelands::Quantity>;
using SoldiersMap = std::map<SoldierMapDescr, Widelands::Quantity>;
using WaresWorkersMap = std::map<Widelands::DescriptionIndex, Widelands::Quantity>;
using SoldierAmount = std::pair<SoldierMapDescr, Widelands::Quantity>;
using WorkerAmount = std::pair<Widelands::DescriptionIndex, Widelands::Quantity>;
using PlrInfluence = std::pair<Widelands::PlayerNumber, Widelands::MilitaryInfluence>;
using WaresSet = std::set<Widelands::DescriptionIndex>;
using InputSet = std::set<std::pair<Widelands::DescriptionIndex, Widelands::WareWorker>>;
using WorkersSet = std::set<Widelands::DescriptionIndex>;
using SoldiersList = std::vector<Widelands::Soldier*>;

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

// Sort functor to sort the owners claiming a field by their influence.
inline int sort_claimers(const PlrInfluence& first, const PlrInfluence& second) {
	return static_cast<int>(first.second > second.second);
}

// Checks if a field has the desired caps
bool check_has_caps(lua_State* L,
                    const std::string& query,
                    const Widelands::FCoords& f,
                    const Widelands::NodeCaps& caps,
                    const Widelands::Map& map);

int wares_or_workers_map_to_lua(lua_State* L,
                                const Widelands::Buildcost& ware_amount_map,
                                Widelands::MapObjectType type);

// Versions of the above macros which accept wares and workers
InputSet
parse_get_input_arguments(lua_State* L, const Widelands::TribeDescr& tribe, bool* return_number);
InputMap parse_set_input_arguments(lua_State* L, const Widelands::TribeDescr& tribe);

WaresWorkersMap count_wares_on_flag_(Widelands::Flag& f,
                                     const Widelands::Descriptions& descriptions);

// Does most of the work of get_workers for player immovables (buildings and roads mainly).
int do_get_workers(lua_State* L,
                   const Widelands::PlayerImmovable& pi,
                   const WaresWorkersMap& valid_workers);

int do_set_workers_for_road(lua_State* L,
                            Widelands::RoadBase* pi,
                            const WaresWorkersMap& valid_workers);
int do_set_workers_for_productionsite(lua_State* L,
                                      Widelands::ProductionSite* pi,
                                      const WaresWorkersMap& valid_workers);

// Return the valid workers for a Road.
WaresWorkersMap get_valid_workers_for(const Widelands::RoadBase& r);
// Returns the valid workers allowed in 'pi'.
WaresWorkersMap get_valid_workers_for(const Widelands::ProductionSite& ps);
// Translate the given Workers map into a (string, count) Lua table.
int workers_map_to_lua(lua_State* L, const WaresWorkersMap& valid_workers);
// Unpacks the Lua table of the form {health, attack, defense, evade} at the stack index
// 'table_index' into a SoldierMapDescr struct.
SoldierMapDescr
unbox_lua_soldier_description(lua_State* L, int table_index, const Widelands::SoldierDescr& sd);
// Parser the arguments of set_soldiers() into a setpoint. See the
// documentation in has_soldiers to understand the valid arguments.
SoldiersMap parse_set_soldiers_arguments(lua_State* L,
                                         const Widelands::SoldierDescr& soldier_descr);
// Does most of the work of get_soldiers for buildings.
int do_get_soldiers(lua_State* L,
                    const Widelands::SoldierControl& sc,
                    const Widelands::TribeDescr& tribe);
// Does most of the work of set_soldiers for buildings.
int do_set_soldiers(lua_State* L,
                    const Widelands::Coords& building_position,
                    Widelands::SoldierControl* sc,
                    Widelands::Player* owner);

// Parses a table of name/count pairs as given from Lua.
void parse_wares_workers(lua_State* L,
                         int table_index,
                         const Widelands::TribeDescr& tribe,
                         InputMap* ware_workers_list,
                         bool is_ware);
Widelands::BillOfMaterials
parse_wares_as_bill_of_material(lua_State* L, int table_index, const Widelands::TribeDescr& tribe);

const Widelands::TribeDescr* get_tribe_descr(lua_State* L, const std::string& tribename);

std::string soldier_preference_to_string(Widelands::SoldierPreference p);
Widelands::SoldierPreference string_to_soldier_preference(const std::string& p);

// Transforms the given warehouse policy to a string which is used by the lua code
void wh_policy_to_string(lua_State* L, Widelands::StockPolicy p);
// Transforms the given string from the lua code to a warehouse policy
Widelands::StockPolicy string_to_wh_policy(lua_State* L, uint32_t index);

Widelands::WarePriority string_to_priority(const std::string& p);
std::string priority_to_string(const Widelands::WarePriority& priority);

void luaopen_wlmap(lua_State*);

}  // namespace LuaMaps

#endif  // end of include guard: WL_SCRIPTING_LUA_MAP_H
