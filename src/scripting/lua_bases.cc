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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "scripting/lua_bases.h"

#include <boost/algorithm/string.hpp>

#include "economy/economy.h"
#include "economy/road.h"
#include "economy/waterway.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"
#include "scripting/factory.h"
#include "scripting/globals.h"
#include "scripting/lua_map.h"
#include "ui_basic/progresswindow.h"

using namespace Widelands;

namespace LuaBases {

/* RST
:mod:`wl.bases`
==================

.. module:: wl.bases
   :synopsis: Base functions for common features in Game and Editor

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.bases

The :mod:`wl.bases` module contains Base functions on which the classes
of :mod:`wl.game` and :mod:`wl.editor` are based. You will not need
to create any of the functions in this module directly, but you might
use their functionality via the child classes.
*/

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */

/* RST
Module Classes
^^^^^^^^^^^^^^^^

*/

/* RST
EditorGameBase
--------------

.. class:: EditorGameBase

   Common functionality between Editor and Game.
*/

const char LuaEditorGameBase::className[] = "EditorGameBase";
const MethodType<LuaEditorGameBase> LuaEditorGameBase::Methods[] = {
   METHOD(LuaEditorGameBase, get_immovable_description),
   METHOD(LuaEditorGameBase, tribe_immovable_exists),
   METHOD(LuaEditorGameBase, get_building_description),
   METHOD(LuaEditorGameBase, get_ship_description),
   METHOD(LuaEditorGameBase, get_tribe_description),
   METHOD(LuaEditorGameBase, get_ware_description),
   METHOD(LuaEditorGameBase, get_worker_description),
   METHOD(LuaEditorGameBase, get_resource_description),
   METHOD(LuaEditorGameBase, get_terrain_description),
   METHOD(LuaEditorGameBase, save_campaign_data),
   METHOD(LuaEditorGameBase, read_campaign_data),
   METHOD(LuaEditorGameBase, set_loading_message),
   {nullptr, nullptr},
};
const PropertyType<LuaEditorGameBase> LuaEditorGameBase::Properties[] = {
   PROP_RO(LuaEditorGameBase, map),
   PROP_RO(LuaEditorGameBase, players),
   {nullptr, nullptr, nullptr},
};

void LuaEditorGameBase::__persist(lua_State* /* L */) {
}
void LuaEditorGameBase::__unpersist(lua_State* /* L */) {
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: map

      (RO) The :class:`~wl.map.Map` the game is played on.
*/
int LuaEditorGameBase::get_map(lua_State* L) {
	to_lua<LuaMaps::LuaMap>(L, new LuaMaps::LuaMap());
	return 1;
}

/* RST
   .. attribute:: players

      (RO) An :class:`array` with the defined players. The players are
      either of type :class:`wl.game.Player` or :class:`wl.editor.Player`.

      In game, there might be less players then defined in
      :attr:`wl.map.Map.player_slots` because some slots might not be taken.
      Also note that for the same reason you cannot index this array with
      :attr:`wl.bases.PlayerBase.number`, but the players are ordered with
      increasing number in this array.

      The editor always creates all players that are defined by the map.
*/
int LuaEditorGameBase::get_players(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);

	lua_newtable(L);

	uint32_t idx = 1;
	for (PlayerNumber i = 1; i <= kMaxPlayers; i++) {
		Player* rv = egbase.get_player(i);
		if (!rv) {
			continue;
		}

		lua_pushuint32(L, idx++);
		get_factory(L).push_player(L, i);
		lua_settable(L, -3);
	}
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. function:: get_immovable_description(immovable_name)

      :arg immovable_name: the name of the immovable

      Returns the ImmovableDescription for the named object.

      (RO) The :class:`~wl.Game.Immovable_description`.
*/
int LuaEditorGameBase::get_immovable_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const std::string immovable_name = luaL_checkstring(L, 2);
	EditorGameBase& egbase = get_egbase(L);
	const World& world = egbase.world();
	DescriptionIndex idx = world.get_immovable_index(immovable_name);
	if (idx != INVALID_INDEX) {
		const ImmovableDescr* descr = world.get_immovable_descr(idx);
		return to_lua<LuaMaps::LuaImmovableDescription>(
		   L, new LuaMaps::LuaImmovableDescription(descr));
	}
	const Tribes& tribes = egbase.tribes();
	idx = tribes.immovable_index(immovable_name);
	if (!tribes.immovable_exists(idx)) {
		report_error(L, "Immovable %s does not exist", immovable_name.c_str());
	}
	const ImmovableDescr* descr = tribes.get_immovable_descr(idx);
	return to_lua<LuaMaps::LuaImmovableDescription>(L, new LuaMaps::LuaImmovableDescription(descr));
}

/* RST
   .. function:: tribe_immovable_exists(immovable_name)

      :arg immovable_name: the name of the tribe immovable

      Returns whether the tribes know about an ImmovableDescription for the named object.

      (RO) ``true`` if the named tribe immovable exists, ``false`` otherwise.
*/
int LuaEditorGameBase::tribe_immovable_exists(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const std::string immovable_name = luaL_checkstring(L, 2);
	lua_pushboolean(L, get_egbase(L).tribes().immovable_index(immovable_name) != INVALID_INDEX);
	return 1;
}

/* RST
   .. function:: get_building_description(building_description.name)

      :arg building_name: the name of the building

      Returns the description for the given building.

      (RO) The :class:`~wl.Game.Building_description`.
*/
int LuaEditorGameBase::get_building_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const Tribes& tribes = get_egbase(L).tribes();
	const std::string building_name = luaL_checkstring(L, 2);
	const DescriptionIndex building_index = tribes.building_index(building_name);
	if (!tribes.building_exists(building_index)) {
		report_error(L, "Building %s does not exist", building_name.c_str());
	}
	const BuildingDescr* building_description = tribes.get_building_descr(building_index);

	return LuaMaps::upcasted_map_object_descr_to_lua(L, building_description);
}

/* RST
   .. function:: get_ship_description(ship_description.name)

      :arg ship_name: the name of the ship

      Returns the description for the given ship.

      (RO) The :class:`~wl.Game.Ship_description`.
*/
int LuaEditorGameBase::get_ship_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const Tribes& tribes = get_egbase(L).tribes();
	const std::string ship_name = luaL_checkstring(L, 2);
	const DescriptionIndex ship_index = tribes.ship_index(ship_name);
	if (!tribes.ship_exists(ship_index)) {
		report_error(L, "Ship %s does not exist", ship_name.c_str());
	}
	const ShipDescr* ship_description = tribes.get_ship_descr(ship_index);

	return LuaMaps::upcasted_map_object_descr_to_lua(L, ship_description);
}
/* RST
   .. function:: get_tribe_description(tribe_name)

      :arg tribe_name: the name of the tribe

      Returns the tribe description of the given tribe.
      Loads the tribe if it hasn't been loaded yet.

      (RO) The :class:`~wl.Game.Tribe_description`.
*/
int LuaEditorGameBase::get_tribe_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}

	const std::string tribe_name = luaL_checkstring(L, 2);
	if (!Widelands::tribe_exists(tribe_name)) {
		report_error(L, "Tribe %s does not exist", tribe_name.c_str());
	}
	Notifications::publish(
	   NoteMapObjectDescription(tribe_name, NoteMapObjectDescription::LoadType::kObject));

	const Tribes& tribes = get_egbase(L).tribes();
	return to_lua<LuaMaps::LuaTribeDescription>(
	   L, new LuaMaps::LuaTribeDescription(tribes.get_tribe_descr(tribes.tribe_index(tribe_name))));
}

/* RST
   .. function:: get_ware_description(ware_description.name)

      :arg ware_name: the name of the ware

      Returns the ware description for the given ware.

      (RO) The :class:`~wl.Game.Ware_description`.
*/
int LuaEditorGameBase::get_ware_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const Tribes& tribes = get_egbase(L).tribes();
	const std::string ware_name = luaL_checkstring(L, 2);
	DescriptionIndex ware_index = tribes.ware_index(ware_name);
	if (!tribes.ware_exists(ware_index)) {
		report_error(L, "Ware %s does not exist", ware_name.c_str());
	}
	const WareDescr* ware_description = tribes.get_ware_descr(ware_index);
	return LuaMaps::upcasted_map_object_descr_to_lua(L, ware_description);
}

/* RST
   .. function:: get_worker_description(worker_description.name)

      :arg worker_name: the name of the worker

      Returs the worker desciption for the given worker.

      (RO) The :class:`~wl.Game.Worker_description`.
*/
int LuaEditorGameBase::get_worker_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const Tribes& tribes = get_egbase(L).tribes();
	const std::string worker_name = luaL_checkstring(L, 2);
	const DescriptionIndex worker_index = tribes.worker_index(worker_name);
	if (!tribes.worker_exists(worker_index)) {
		report_error(L, "Worker %s does not exist", worker_name.c_str());
	}
	const WorkerDescr* worker_description = tribes.get_worker_descr(worker_index);
	return LuaMaps::upcasted_map_object_descr_to_lua(L, worker_description);
}

/* RST
   .. function:: get_resource_description(resource_name)

      :arg resource_name: the name of the resource

      Returns the resource description for the given resource.

      (RO) The :class:`~wl.Game.Resource_description`.
*/
int LuaEditorGameBase::get_resource_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const std::string resource_name = luaL_checkstring(L, 2);
	const World& world = get_egbase(L).world();
	const DescriptionIndex idx = world.resource_index(resource_name.c_str());

	if (idx == INVALID_INDEX) {
		report_error(L, "Resource %s does not exist", resource_name.c_str());
	}

	const ResourceDescription* descr = world.get_resource(idx);
	return to_lua<LuaMaps::LuaResourceDescription>(L, new LuaMaps::LuaResourceDescription(descr));
}

/* RST
   .. function:: get_terrain_description(terrain_name)

      :arg terrain_name: the name of the terrain

      Returns a given terrain description for the given terrain.

      (RO) The :class:`~wl.Game.Terrain_description`.
*/
int LuaEditorGameBase::get_terrain_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const std::string terrain_name = luaL_checkstring(L, 2);
	const TerrainDescription* descr = get_egbase(L).world().terrain_descr(terrain_name);
	if (!descr) {
		report_error(L, "Terrain %s does not exist", terrain_name.c_str());
	}
	return to_lua<LuaMaps::LuaTerrainDescription>(L, new LuaMaps::LuaTerrainDescription(descr));
}

/* Helper function for save_campaign_data()

   This function reads the lua table from the stack and saves information about its
   keys, values and data types and its size to the provided maps.
   This function is recursive so subtables to any depth can be saved.
   Each value in the table (including all subtables) is uniquely identified by a key_key.
   The key_key is used as key in all the map.
   For the topmost table of size x, the key_keys are called '_0' through '_x-1'.
   For a subtable of size z at key_key '_y', the subtable's key_keys are called '_y_0' through
   '_y_z-1'.
   If a table is an array, the map 'keys' will contain no mappings for the array's key_keys.
*/
static void save_table_recursively(lua_State* L,
                                   const std::string& depth,
                                   std::map<std::string, const char*>* data,
                                   std::map<std::string, const char*>* keys,
                                   std::map<std::string, const char*>* type,
                                   std::map<std::string, uint32_t>* size) {
	lua_pushnil(L);
	uint32_t i = 0;
	while (lua_next(L, -2) != 0) {
		const std::string key_key = depth + "_" + std::to_string(i);

		// check the value's type
		const char* type_name = lua_typename(L, lua_type(L, -1));
		const std::string t = std::string(type_name);

		(*type)[key_key] = type_name;

		if (t == "number" || t == "string") {
			// numbers may be treated like strings here
			(*data)[key_key] = luaL_checkstring(L, -1);
		} else if (t == "boolean") {
			(*data)[key_key] = luaL_checkboolean(L, -1) ? "true" : "false";
		} else if (t == "table") {
			save_table_recursively(L, depth + "_" + std::to_string(i), data, keys, type, size);
		} else {
			report_error(
			   L, "A campaign data value may be a string, integer, boolean, or table; but not a %s!",
			   type_name);
		}

		++i;

		// put the key on the stack top
		lua_pop(L, 1);
		if (lua_type(L, -1) == LUA_TSTRING) {
			// this is a table
			(*keys)[key_key] = luaL_checkstring(L, -1);
		} else if (lua_type(L, -1) == LUA_TNUMBER) {
			// this is an array
			if (i != luaL_checkuint32(L, -1)) {
				// If we get here, the scripter must have set some array values to nil.
				// This is forbidden because it causes problems when trying to read the data later.
				report_error(L, "A campaign data array entry must not be nil!");
			}
			// otherwise, this is a normal array, so all is well
		} else {
			report_error(L, "A campaign data key may be a string or integer; but not a %s!",
			             lua_typename(L, lua_type(L, -1)));
		}
	}
	(*size)[depth] = i;
}

/* RST
   .. function:: save_campaign_data(campaign_name, scenario_name, data)

      :arg campaign_name: the name of the current campaign, e.g. "empiretut" or "frisians"
      :arg scenario_name: the name of the current scenario, e.g. "emp04" or "fri03"
      :arg data: a table of key-value pairs to save

      Saves information that can be read by other scenarios.

      If an array is used, the data will be saved in the correct order. Arrays may not contain nil
      values. If the table is not an array, all keys have to be strings. Tables may contain
      subtables of any depth. Cyclic dependencies will cause Widelands to crash. Only tables/arrays,
      strings, integer numbers and booleans may be used as values.
*/
int LuaEditorGameBase::save_campaign_data(lua_State* L) {

	const std::string campaign_name = luaL_checkstring(L, 2);
	const std::string scenario_name = luaL_checkstring(L, 3);
	luaL_checktype(L, 4, LUA_TTABLE);

	std::string dir = kCampaignDataDir + g_fs->file_separator() + campaign_name;
	boost::trim(dir);
	g_fs->ensure_directory_exists(dir);

	std::string complete_filename =
	   dir + g_fs->file_separator() + scenario_name + kCampaignDataExtension;
	boost::trim(complete_filename);

	std::map<std::string, const char*> data;
	std::map<std::string, const char*> keys;
	std::map<std::string, const char*> type;
	std::map<std::string, uint32_t> size;

	save_table_recursively(L, "", &data, &keys, &type, &size);

	Profile profile;
	Section& data_section = profile.create_section("data");
	for (const auto& p : data) {
		data_section.set_string(p.first.c_str(), p.second);
	}
	Section& keys_section = profile.create_section("keys");
	for (const auto& p : keys) {
		keys_section.set_string(p.first.c_str(), p.second);
	}
	Section& type_section = profile.create_section("type");
	for (const auto& p : type) {
		type_section.set_string(p.first.c_str(), p.second);
	}
	Section& size_section = profile.create_section("size");
	for (const auto& p : size) {
		size_section.set_natural(p.first.c_str(), p.second);
	}

	profile.write(complete_filename.c_str(), false);

	return 0;
}

/* Helper function for read_campaign_data()

   This function reads the campaign data file and re-creates the table the data was created from.
   This function is recursive so subtables to any depth can be created.
   For information on section structure and key_keys, see the comment for save_table_recursively().
   This function first newly creates the table to write data to, and the number of items in the
   table is read.
   For each item, the unique key_key is created. If the 'keys' section doesn't contain an entry for
   that key_key,
   it must be because this table is supposed to be an array. Then the data type is checked
   and the key-value pair is written to the table as the correct type.
*/
static void push_table_recursively(lua_State* L,
                                   const std::string& depth,
                                   Section* data_section,
                                   Section* keys_section,
                                   Section* type_section,
                                   Section* size_section) {
	const uint32_t size = size_section->get_natural(depth.c_str());
	lua_newtable(L);
	for (uint32_t i = 0; i < size; i++) {
		const std::string key_key_str(depth + '_' + std::to_string(i));
		const char* key_key = key_key_str.c_str();
		if (keys_section->has_val(key_key)) {
			// This is a table
			lua_pushstring(L, keys_section->get_string(key_key));
		} else {
			// This must be an array
			lua_pushinteger(L, i + 1);
		}

		// check the data type and push the value
		const std::string type = type_section->get_string(key_key);

		if (type == "boolean") {
			lua_pushboolean(L, data_section->get_bool(key_key));
		} else if (type == "number") {
			lua_pushinteger(L, data_section->get_int(key_key));
		} else if (type == "string") {
			lua_pushstring(L, data_section->get_string(key_key));
		} else if (type == "table") {
			// creates a new (sub-)table at the stacktop, populated with its own key-value-pairs
			push_table_recursively(L, depth + "_" + std::to_string(i), data_section, keys_section,
			                       type_section, size_section);
		} else {
			// this code should not be reached unless the user manually edited the .wcd file
			log("Illegal data type %s in campaign data file, setting key %s to nil\n", type.c_str(),
			    luaL_checkstring(L, -1));
			lua_pushnil(L);
		}
		lua_settable(L, -3);
	}
}

/* RST
   .. function:: read_campaign_data(campaign_name, scenario_name)

      :arg campaign_name: the name of the campaign, e.g. "empiretut" or "frisians"
      :arg scenario_name: the name of the scenario that saved the data, e.g. "emp04" or "fri03"

      Reads information that was saved by another scenario.
      The data is returned as a table of key-value pairs.
      The table is not guaranteed to be in any particular order, unless it is an array,
      in which case it will be returned in the same order as it was saved.
      This function returns :const:`nil` if the file cannot be opened for reading.
*/
int LuaEditorGameBase::read_campaign_data(lua_State* L) {
	const std::string campaign_name = luaL_checkstring(L, 2);
	const std::string scenario_name = luaL_checkstring(L, 3);

	std::string complete_filename = kCampaignDataDir + g_fs->file_separator() + campaign_name +
	                                g_fs->file_separator() + scenario_name + kCampaignDataExtension;
	boost::trim(complete_filename);

	Profile profile;
	profile.read(complete_filename.c_str());
	Section* data_section = profile.get_section("data");
	Section* keys_section = profile.get_section("keys");
	Section* type_section = profile.get_section("type");
	Section* size_section = profile.get_section("size");
	if (data_section == nullptr || keys_section == nullptr || type_section == nullptr ||
	    size_section == nullptr) {
		log("Unable to read campaign data file, returning nil\n");
		lua_pushnil(L);
	} else {
		push_table_recursively(L, "", data_section, keys_section, type_section, size_section);
	}

	return 1;
}

/* RST
   .. function:: set_loading_message(text)

      :arg text: the text to display

      Change the progress message on the loading screen.
      May be used from the init.lua files for tribe/world loading only.
*/
int LuaEditorGameBase::set_loading_message(lua_State* L) {
	Notifications::publish(UI::NoteLoadingMessage(luaL_checkstring(L, 2)));
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
PlayerBase
----------

.. class:: PlayerBase

   The Base class for the Player objects in Editor and Game
*/

const char LuaPlayerBase::className[] = "PlayerBase";
const MethodType<LuaPlayerBase> LuaPlayerBase::Methods[] = {
   METHOD(LuaPlayerBase, __eq),        METHOD(LuaPlayerBase, __tostring),
   METHOD(LuaPlayerBase, conquer),     METHOD(LuaPlayerBase, get_wares),
   METHOD(LuaPlayerBase, get_workers), METHOD(LuaPlayerBase, place_building),
   METHOD(LuaPlayerBase, place_flag),  METHOD(LuaPlayerBase, place_road),
   METHOD(LuaPlayerBase, place_ship),  {nullptr, nullptr},
};
const PropertyType<LuaPlayerBase> LuaPlayerBase::Properties[] = {
   PROP_RO(LuaPlayerBase, number),
   PROP_RO(LuaPlayerBase, tribe_name),
   {nullptr, nullptr, nullptr},
};

void LuaPlayerBase::__persist(lua_State* L) {
	PERS_UINT32("player", player_number_);
}
void LuaPlayerBase::__unpersist(lua_State* L) {
	UNPERS_UINT32("player", player_number_)
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: number

      (RO) The number of this Player.
*/
int LuaPlayerBase::get_number(lua_State* L) {
	lua_pushuint32(L, player_number_);
	return 1;
}

/* RST
   .. attribute:: tribe_name

      (RO) The name of the tribe of this player.
*/
int LuaPlayerBase::get_tribe_name(lua_State* L) {
	lua_pushstring(L, get(L, get_egbase(L)).tribe().name());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaPlayerBase::__eq(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	const Player& me = get(L, egbase);
	const Player& you = (*get_base_user_class<LuaPlayerBase>(L, 2))->get(L, egbase);

	lua_pushboolean(L, (me.player_number() == you.player_number()));
	return 1;
}

int LuaPlayerBase::__tostring(lua_State* L) {
	const std::string pushme = (boost::format("Player(%i)") %
	                            static_cast<unsigned int>(get(L, get_egbase(L)).player_number()))
	                              .str();
	lua_pushstring(L, pushme.c_str());
	return 1;
}
/* RST
   .. function:: place_flag(field[, force])

      Builds a flag on a given field if it is legal to do so. If not,
      reports an error

      :arg field: where the flag should be created
      :type field: :class:`wl.map.Field`
      :arg force: If this is :const:`true` then the map is created with
         pure force:

            * if there is an immovable on this field, it will be
              removed
            * if there are flags too close by to this field, they will be
              ripped
            * if the player does not own the territory, it is conquered
              for him.
      :type force: :class:`boolean`
      :returns: :class:`wl.map.Flag` object created or :const:`nil`.
*/
int LuaPlayerBase::place_flag(lua_State* L) {
	uint32_t n = lua_gettop(L);
	LuaMaps::LuaField* c = *get_user_class<LuaMaps::LuaField>(L, 2);
	bool force = false;
	if (n > 2) {
		force = luaL_checkboolean(L, 3);
	}

	Flag* f;
	if (!force) {
		f = get(L, get_egbase(L)).build_flag(c->fcoords(L));
		if (!f) {
			report_error(L, "Couldn't build flag!");
		}
	} else {
		f = &get(L, get_egbase(L)).force_flag(c->fcoords(L));
	}

	return to_lua<LuaMaps::LuaFlag>(L, new LuaMaps::LuaFlag(*f));
}

/* RST
   .. method:: place_road(roadtype, f1, dir1, dir2, ...[, force=false])

      Start a road or waterway at the given field, then walk the directions
      given. Places a flag at the last field.

      If the last argument to this function is :const:`true` the road will
      be created by force: all immovables in the way are removed and land
      is conquered.

      :arg roadtype: 'normal', 'busy', or 'waterway'
      :type roadtype: :class:`string`
      :arg f1: fields to connect with this road
      :type f1: :class:`wl.map.Field`
      :arg dirs: direction, can be either ("r", "l", "br", "bl", "tr", "tl") or
         ("e", "w", "ne", "nw", "se", "sw").
      :type dirs: :class:`string`

      :returns: the road created
*/
int LuaPlayerBase::place_road(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	const Map& map = egbase.map();

	const std::string roadtype = luaL_checkstring(L, 2);
	Flag* starting_flag = (*get_user_class<LuaMaps::LuaFlag>(L, 3))->get(L, egbase);
	Coords current = starting_flag->get_position();
	Path path(current);

	bool force_road = false;
	if (lua_isboolean(L, -1)) {
		force_road = luaL_checkboolean(L, -1);
		lua_pop(L, 1);
	}

	// Construct the path
	CheckStepLimited cstep;
	for (int32_t i = 4; i <= lua_gettop(L); i++) {
		std::string d = luaL_checkstring(L, i);

		if (d == "ne" || d == "tr") {
			path.append(map, 1);
			map.get_trn(current, &current);
		} else if (d == "e" || d == "r") {
			path.append(map, 2);
			map.get_rn(current, &current);
		} else if (d == "se" || d == "br") {
			path.append(map, 3);
			map.get_brn(current, &current);
		} else if (d == "sw" || d == "bl") {
			path.append(map, 4);
			map.get_bln(current, &current);
		} else if (d == "w" || d == "l") {
			path.append(map, 5);
			map.get_ln(current, &current);
		} else if (d == "nw" || d == "tl") {
			path.append(map, 6);
			map.get_tln(current, &current);
		} else {
			report_error(L, "Illegal direction: %s", d.c_str());
		}

		cstep.add_allowed_location(current);
	}

	// Make sure that the road cannot cross itself
	Path optimal_path;
	map.findpath(path.get_start(), path.get_end(), 0, optimal_path, cstep, Map::fpBidiCost);
	if (optimal_path.get_nsteps() != path.get_nsteps()) {
		report_error(L, "Cannot build a road that crosses itself!");
	}

	RoadBase* r = nullptr;
	if (force_road) {
		if (roadtype == "waterway") {
			r = &get(L, egbase).force_waterway(path);
		} else {
			Road& road = get(L, egbase).force_road(path);
			if (roadtype == "busy") {
				road.set_busy(egbase, true);
			} else if (roadtype != "normal") {
				report_error(
				   L, "Invalid road type '%s' (permitted values are 'normal', 'busy', and 'waterway'",
				   roadtype.c_str());
			}
			r = &road;
		}
	} else {
		BaseImmovable* bi = map.get_immovable(current);
		if (!bi || bi->descr().type() != MapObjectType::FLAG) {
			if (!get(L, egbase).build_flag(current)) {
				report_error(L, "Could not place end flag!");
			}
		}
		if (bi && bi == starting_flag) {
			report_error(L, "Cannot build a closed loop!");
		}

		if (roadtype == "waterway") {
			r = get(L, egbase).build_waterway(path);
		} else {
			Road* road = get(L, egbase).build_road(path);
			if (roadtype == "busy") {
				if (road) {
					road->set_busy(egbase, true);
				}
			} else if (roadtype != "normal") {
				report_error(
				   L, "Invalid road type '%s' (permitted values are 'normal', 'busy', and 'waterway'",
				   roadtype.c_str());
			}
			r = road;
		}
	}

	if (!r) {
		report_error(L, "Error while creating Road. May be: something is in "
		                "the way or you do not own the territory where you want to build "
		                "the road");
	}

	return to_lua<LuaMaps::LuaRoad>(L, new LuaMaps::LuaRoad(*r));
}

/* RST
   .. method:: place_building(name, field[, cs = false, force = false])

      Immediately creates a building on the given field. The building starts
      out completely empty. If :const:`cs` is set, the building
      is not created directly, instead a constructionsite for this building is
      placed.

      If the :const:`force` argument is set, the building is forced into
      existence: the same action is taken as for :meth:`place_flag` when force
      is :const:`true`. Additionally, all buildings that are too close to the
      new one are ripped.

      :returns: a subclass of :class:`wl.map.Building`
*/
int LuaPlayerBase::place_building(lua_State* L) {
	const std::string& name = luaL_checkstring(L, 2);
	LuaMaps::LuaField* c = *get_user_class<LuaMaps::LuaField>(L, 3);
	bool constructionsite = false;
	bool force = false;

	if (lua_gettop(L) >= 4) {
		constructionsite = luaL_checkboolean(L, 4);
	}
	if (lua_gettop(L) >= 5) {
		force = luaL_checkboolean(L, 5);
	}

	EditorGameBase& egbase = get_egbase(L);
	const Tribes& tribes = egbase.tribes();
	Player& player = get(L, egbase);

	// If the building belongs to a tribe that no player is playing, we need to load it now
	Notifications::publish(
	   NoteMapObjectDescription(name, NoteMapObjectDescription::LoadType::kObject));

	const DescriptionIndex building_index = tribes.building_index(name);

	// Ensure that the loaded object was indeed a building
	if (!tribes.building_exists(building_index)) {
		report_error(L, "Unknown Building: '%s'", name.c_str());
	}

	if (!player.tribe().has_building(building_index) &&
	    tribes.get_building_descr(building_index)->type() !=
	       Widelands::MapObjectType::MILITARYSITE) {
		report_error(L, "Building: '%s' is not available for Player %d's tribe '%s'", name.c_str(),
		             player.player_number(), player.tribe().name().c_str());
	}

	FormerBuildings former_buildings;
	find_former_buildings(tribes, building_index, &former_buildings);
	if (constructionsite) {
		former_buildings.pop_back();
	}

	Building* b = nullptr;
	if (force) {
		if (constructionsite) {
			b = &player.force_csite(c->coords(), building_index, former_buildings);
		} else {
			b = &player.force_building(c->coords(), former_buildings);
		}
	} else {
		b = player.build(c->coords(), building_index, constructionsite, former_buildings);
	}
	if (!b) {
		const std::string tempname(
		   force ? constructionsite ? "force constructionsite" : "force building" : "place building");
		report_error(L, "Couldn't %s '%s' at (%d, %d)!", tempname.c_str(), name.c_str(),
		             c->coords().x, c->coords().y);
	}

	LuaMaps::upcasted_map_object_to_lua(L, b);
	return 1;
}

/* RST
   .. method:: place_ship(field)

      Places a ship for the player's tribe, which will be
      owned by the player.

      :arg field: where the ship should be placed.
      :type field: :class:`wl.map.Field`

      :returns: The new ship that was created.
*/
// UNTESTED
int LuaPlayerBase::place_ship(lua_State* L) {
	LuaMaps::LuaField* c = *get_user_class<LuaMaps::LuaField>(L, 2);

	EditorGameBase& egbase = get_egbase(L);
	Player& player = get(L, egbase);

	const ShipDescr* descr = egbase.tribes().get_ship_descr(player.tribe().ship());
	Bob& ship = egbase.create_ship(c->coords(), descr->name(), &player);

	LuaMaps::upcasted_map_object_to_lua(L, &ship);

	return 1;
}

/* RST
   .. method:: conquer(f[, radius=1])

      Conquer this area around the given field if it does not belong to the
      player already. This will conquer the fields no matter who owns it at the
      moment.

      :arg f: center field for conquering
      :type f: :class:`wl.map.Field`
      :arg radius: radius to conquer around. Default value makes this call
         conquer 7 fields
      :type radius: :class:`integer`
      :returns: :const:`nil`
*/
int LuaPlayerBase::conquer(lua_State* L) {
	uint32_t radius = 1;
	if (lua_gettop(L) > 2) {
		radius = luaL_checkuint32(L, 3);
	}

	get_egbase(L).conquer_area_no_building(PlayerArea<Area<FCoords>>(
	   player_number_,
	   Area<FCoords>((*get_user_class<LuaMaps::LuaField>(L, 2))->fcoords(L), radius)));
	return 0;
}

/* RST
   .. method:: get_workers(name)

      Returns the number of workers of this type in the players stock. This does not implement
      everything that :class:`HasWorkers` offers.

      :arg name: name of the worker to get
      :type name: :class:`string`.
      :returns: the number of workers
*/
// UNTESTED
int LuaPlayerBase::get_workers(lua_State* L) {
	Player& player = get(L, get_egbase(L));
	const std::string workername = luaL_checkstring(L, -1);

	const DescriptionIndex worker = player.tribe().worker_index(workername);

	uint32_t nworkers = 0;
	for (const auto& economy : player.economies()) {
		if (economy.second->type() == Widelands::wwWORKER) {
			nworkers += economy.second->stock_ware_or_worker(worker);
		}
	}
	lua_pushuint32(L, nworkers);
	return 1;
}

/* RST
   .. method:: get_wares(name)

      Returns the number of wares of this type in the players stock. This does not implement
      everything that :class:`HasWorkers` offers.

      :arg name: name of the worker to get
      :type name: :class:`string`.
      :returns: the number of wares
*/
// UNTESTED
int LuaPlayerBase::get_wares(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	Player& player = get(L, egbase);
	const std::string warename = luaL_checkstring(L, -1);

	const DescriptionIndex ware = egbase.tribes().ware_index(warename);

	uint32_t nwares = 0;
	for (const auto& economy : player.economies()) {
		if (economy.second->type() == Widelands::wwWARE) {
			nwares += economy.second->stock_ware_or_worker(ware);
		}
	}
	lua_pushuint32(L, nwares);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Player& LuaPlayerBase::get(lua_State* L, Widelands::EditorGameBase& egbase) {
	if (player_number_ > kMaxPlayers) {
		report_error(L, "Illegal player number %i", player_number_);
	}
	Player* rv = egbase.get_player(player_number_);
	if (!rv) {
		report_error(L, "Player with the number %i does not exist", player_number_);
	}
	return *rv;
}

/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */

const static struct luaL_Reg wlbases[] = {{nullptr, nullptr}};

void luaopen_wlbases(lua_State* const L) {
	lua_getglobal(L, "wl");      // S: wl_table
	lua_pushstring(L, "bases");  // S: wl_table "bases"
	luaL_newlib(L, wlbases);     // S: wl_table "bases" wl.bases_table
	lua_settable(L, -3);         // S: wl_table
	lua_pop(L, 1);               // S:

	register_class<LuaEditorGameBase>(L, "bases");
	register_class<LuaPlayerBase>(L, "bases");
}
}  // namespace LuaBases
