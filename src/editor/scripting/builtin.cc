/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License), or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not), write to the Free Software
 * Foundation), Inc., 51 Franklin Street), Fifth Floor), Boston), MA  02110-1301, USA.
 *
 */

#include "editor/scripting/builtin.h"

#include "editor/scripting/function.h"

const BuiltinFunctionInfo& builtin_f(const std::string& name) {
	try {
		return *kBuiltinFunctions.at(name);
	} catch (...) {
		throw wexception("Unknown builtin function %s", name.c_str());
	}
}

std::string builtin_f(const FunctionBase& f) {
	for (const auto& pair : kBuiltinFunctions) {
		if (pair.second->function.get() == &f) {
			return pair.first;
		}
	}
	return "";
}

const BuiltinPropertyInfo& builtin_p(const std::string& name) {
	try {
		return *kBuiltinProperties.at(name);
	} catch (...) {
		throw wexception("Unknown builtin property %s", name.c_str());
	}
}

std::string builtin_p(const Property& p) {
	for (const auto& pair : kBuiltinProperties) {
		if (pair.second->property.get() == &p) {
			return pair.first;
		}
	}
	return "";
}

/************************************************************
                     Builtin functions
************************************************************/

// The _() function is not contained here – access it instead via
// `ConstexprString`'s `translatable` attribute.
const std::map<std::string, BuiltinFunctionInfo*> kBuiltinFunctions = {

   // Real Lua builtins

   {"print", new BuiltinFunctionInfo(
                []() { return _("Prints debug information to the stdandard output."); },
                new FunctionBase("print",
                                 VariableType(VariableTypeID::Nil),  // call on
                                 VariableType(VariableTypeID::Nil),  // returns
                                 {std::make_pair("text", VariableType(VariableTypeID::String))}))},
   {"random_1",
    new BuiltinFunctionInfo(
       /** TRANSLATORS: max is the name of a function parameter */
       []() { return _("Returns a random value between 1 and max."); },
       new FunctionBase("math.random",
                        VariableType(VariableTypeID::Nil),      // call on
                        VariableType(VariableTypeID::Integer),  // returns
                        {std::make_pair("max", VariableType(VariableTypeID::Integer))}))},
   {"random_2",
    new BuiltinFunctionInfo(
       /** TRANSLATORS: min and max are names of function parameters */
       []() { return _("Returns a random value between min and max."); },
       new FunctionBase("math.random",
                        VariableType(VariableTypeID::Nil),      // call on
                        VariableType(VariableTypeID::Integer),  // returns
                        {std::make_pair("min", VariableType(VariableTypeID::Integer)),
                         std::make_pair("max", VariableType(VariableTypeID::Integer))}))},

   // Shipped Lua functions

   {"sleep",
    new BuiltinFunctionInfo(
       []() { return _("Pauses the current thread for the given number of milliseconds."); },
       new FunctionBase("sleep",
                        VariableType(VariableTypeID::Nil),  // call on
                        VariableType(VariableTypeID::Nil),  // returns
                        {std::make_pair("milliseconds", VariableType(VariableTypeID::Integer))}),
       "scripting/coroutine.lua")},
   {"wake_me",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Pauses the current thread and resumes it at the given gametime (in milliseconds).");
	    },
       new FunctionBase("wake_me",
                        VariableType(VariableTypeID::Nil),  // call on
                        VariableType(VariableTypeID::Nil),  // returns
                        {std::make_pair("time", VariableType(VariableTypeID::Integer))}),
       "scripting/coroutine.lua")},

   // TODO(Nordfriese): This function can take varargs
   {"array_combine",
    new BuiltinFunctionInfo(
       []() { return _("Concatenates the given arrays into a single array."); },
       new FunctionBase("array_combine",
                        VariableType(VariableTypeID::Nil),  // call on
                        VariableType(VariableType(VariableTypeID::Integer),
                                     VariableType(VariableTypeID::Any)),  // returns
                        {std::make_pair("array1",
                                        VariableType(VariableType(VariableTypeID::Integer),
                                                     VariableType(VariableTypeID::Any))),
                         std::make_pair("array2",
                                        VariableType(VariableType(VariableTypeID::Integer),
                                                     VariableType(VariableTypeID::Any)))}),
       "scripting/table.lua")},

   {"connected_road",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Force-build one or more roads with flags. The string argument defining the road "
	          "layout has a format like e.g. 'r,r|br,r|r,r': Multiple directions (e, w, se, sw, ne, "
	          "nw) "
	          "separated by ','; '|' instead of ',' denotes flag placement.");
	    },
       new FunctionBase("connected_road",
                        VariableType(VariableTypeID::Nil),  // call on
                        VariableType(VariableTypeID::Nil),  // returns
                        {std::make_pair("roadtype", VariableType(VariableTypeID::String)),
                         std::make_pair("player", VariableType(VariableTypeID::Player)),
                         std::make_pair("start_flag", VariableType(VariableTypeID::Flag)),
                         std::make_pair("layout", VariableType(VariableTypeID::String)),
                         std::make_pair("create_carriers", VariableType(VariableTypeID::Boolean))}),
       "scripting/infrastructure.lua")},

   // Game

   {"game", new BuiltinFunctionInfo([]() { return _("Returns the running game instance."); },
                                    new FunctionBase("wl.Game",
                                                     VariableType(VariableTypeID::Nil),   // call on
                                                     VariableType(VariableTypeID::Game),  // returns
                                                     {}))},
   {"save", new BuiltinFunctionInfo(
               []() { return _("Saves the game under the given name."); },
               new FunctionBase("save",
                                VariableType(VariableTypeID::Game),  // call on
                                VariableType(VariableTypeID::Nil),   // returns
                                {std::make_pair("name", VariableType(VariableTypeID::String))}))},
   {"get_tribe_descr",
    new BuiltinFunctionInfo(
       []() { return _("Returns the description for the tribe with the given name."); },
       new FunctionBase("get_tribe_description",
                        VariableType(VariableTypeID::Game),        // call on
                        VariableType(VariableTypeID::TribeDescr),  // returns
                        {std::make_pair("name", VariableType(VariableTypeID::String))}))},
   {"get_terrain_descr",
    new BuiltinFunctionInfo(
       []() { return _("Returns the description for the terrain with the given name."); },
       new FunctionBase("get_terrain_description",
                        VariableType(VariableTypeID::Game),          // call on
                        VariableType(VariableTypeID::TerrainDescr),  // returns
                        {std::make_pair("name", VariableType(VariableTypeID::String))}))},
   {"get_resource_descr",
    new BuiltinFunctionInfo(
       []() { return _("Returns the description for the resource with the given name."); },
       new FunctionBase("get_resource_description",
                        VariableType(VariableTypeID::Game),           // call on
                        VariableType(VariableTypeID::ResourceDescr),  // returns
                        {std::make_pair("name", VariableType(VariableTypeID::String))}))},
   {"get_immovable_descr",
    new BuiltinFunctionInfo(
       []() { return _("Returns the description for the immovable with the given name."); },
       new FunctionBase("get_immovable_description",
                        VariableType(VariableTypeID::Game),            // call on
                        VariableType(VariableTypeID::ImmovableDescr),  // returns
                        {std::make_pair("name", VariableType(VariableTypeID::String))}))},
   {"get_building_descr",
    new BuiltinFunctionInfo(
       []() { return _("Returns the description for the building with the given name."); },
       new FunctionBase("get_building_description",
                        VariableType(VariableTypeID::Game),           // call on
                        VariableType(VariableTypeID::BuildingDescr),  // returns
                        {std::make_pair("name", VariableType(VariableTypeID::String))}))},
   {"get_worker_descr",
    new BuiltinFunctionInfo(
       []() { return _("Returns the description for the worker with the given name."); },
       new FunctionBase("get_worker_description",
                        VariableType(VariableTypeID::Game),         // call on
                        VariableType(VariableTypeID::WorkerDescr),  // returns
                        {std::make_pair("name", VariableType(VariableTypeID::String))}))},
   {"get_ware_descr",
    new BuiltinFunctionInfo(
       []() { return _("Returns the description for the ware with the given name."); },
       new FunctionBase("get_ware_description",
                        VariableType(VariableTypeID::Game),       // call on
                        VariableType(VariableTypeID::WareDescr),  // returns
                        {std::make_pair("name", VariableType(VariableTypeID::String))}))},
   {"save_campaign_data",
    new BuiltinFunctionInfo(
       []() { return _("Saves information that can be read by others scenarios."); },
       new FunctionBase("save_campaign_data",
                        VariableType(VariableTypeID::Game),  // call on
                        VariableType(VariableTypeID::Nil),   // returns
                        {std::make_pair("campaign_name", VariableType(VariableTypeID::String)),
                         std::make_pair("scenario_name", VariableType(VariableTypeID::String)),
                         std::make_pair("data",
                                        VariableType(VariableType(VariableTypeID::String),
                                                     VariableType(VariableTypeID::Any)))}))},
   {"read_campaign_data",
    new BuiltinFunctionInfo(
       []() { return _("Read campaign data saved by another scenario."); },
       new FunctionBase("read_campaign_data",
                        VariableType(VariableTypeID::Game),  // call on
                        VariableType(VariableType(VariableTypeID::Any),
                                     VariableType(VariableTypeID::Any)),  // returns
                        {std::make_pair("campaign_name", VariableType(VariableTypeID::String)),
                         std::make_pair("scenario_name", VariableType(VariableTypeID::String))}))},
   {"report_result",
    new BuiltinFunctionInfo(
       []() {
	       return _("In an internet game, reports the ending of the game for a certain player to "
	                "the metaserver. Valid result codes are 0 (lost), 1 (won), and 2 (resigned).");
	    },
       new FunctionBase(
          "wl.game.report_result",
          VariableType(VariableTypeID::Nil),  // call on
          VariableType(VariableTypeID::Nil),  // returns
          {std::make_pair("player", VariableType(VariableTypeID::Player)),
           std::make_pair("result", VariableType(VariableTypeID::Integer)),
           std::make_pair("additional_info", VariableType(VariableTypeID::String))}))},

   // Player

   {"place_flag",
    new BuiltinFunctionInfo(
       []() { return _("Place a flag on the given field."); },
       new FunctionBase("place_flag",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Flag),    // returns
                        {std::make_pair("field", VariableType(VariableTypeID::Field)),
                         std::make_pair("force", VariableType(VariableTypeID::Boolean))}))},
   {"place_flag_no_force",
    new BuiltinFunctionInfo(
       []() { return _("Place a flag on the given field."); },
       new FunctionBase("place_flag",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Flag),    // returns
                        {std::make_pair("field", VariableType(VariableTypeID::Field))}))},
   {"place_building",
    new BuiltinFunctionInfo(
       []() { return _("Place a building on the given field."); },
       new FunctionBase(
          "place_building",
          VariableType(VariableTypeID::Player),    // call on
          VariableType(VariableTypeID::Building),  // returns
          {std::make_pair("internal_building_name", VariableType(VariableTypeID::String)),
           std::make_pair("field", VariableType(VariableTypeID::Field)),
           std::make_pair("constructionsite", VariableType(VariableTypeID::Boolean)),
           std::make_pair("force", VariableType(VariableTypeID::Boolean))}))},
   {"place_building_no_force",
    new BuiltinFunctionInfo(
       []() { return _("Place a building on the given field."); },
       new FunctionBase(
          "place_building",
          VariableType(VariableTypeID::Player),    // call on
          VariableType(VariableTypeID::Building),  // returns
          {std::make_pair("internal_building_name", VariableType(VariableTypeID::String)),
           std::make_pair("field", VariableType(VariableTypeID::Field))}))},
   {"place_ship",
    new BuiltinFunctionInfo(
       []() { return _("Place a ship on the given field."); },
       new FunctionBase("place_ship",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Ship),    // returns
                        {std::make_pair("field", VariableType(VariableTypeID::Field))}))},
   {"conquer",
    new BuiltinFunctionInfo(
       []() { return _("Conquer a region for this player."); },
       new FunctionBase("conquer",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Nil),     // returns
                        {std::make_pair("field", VariableType(VariableTypeID::Field)),
                         std::make_pair("radius", VariableType(VariableTypeID::Integer))}))},
   {"player_get_workers",
    new BuiltinFunctionInfo(
       []() { return _("Returns the number of workers of a certain type in the player’s stock."); },
       new FunctionBase(
          "get_workers",
          VariableType(VariableTypeID::Player),   // call on
          VariableType(VariableTypeID::Integer),  // returns
          {std::make_pair("internal_worker_name", VariableType(VariableTypeID::String))}))},
   {"player_get_wares",
    new BuiltinFunctionInfo(
       []() { return _("Returns the number of wares of a certain type in the player’s stock."); },
       new FunctionBase(
          "get_wares",
          VariableType(VariableTypeID::Player),   // call on
          VariableType(VariableTypeID::Integer),  // returns
          {std::make_pair("internal_ware_name", VariableType(VariableTypeID::String))}))},
   {"send_msg",
    new BuiltinFunctionInfo(
       []() {
	       return _("Sends the player a message. The args table may contain the following keys:\n"
	                " · 'field' – connect a field with this message\n"
	                " · 'status' – 'new' (default), 'read', or 'archived'\n"
	                " · 'popup' – Whether to immediately open the inbox on message reception (true "
	                "or false)\n"
	                " · 'icon' – path to the icon to show instead of the default message icon\n"
	                " · 'heading' – Message header to show instead of 'title'\n"
	                "All keys in args are optional. args may also be nil.");
	    },
       new FunctionBase("send_message",
                        VariableType(VariableTypeID::Player),   // call on
                        VariableType(VariableTypeID::Message),  // returns
                        {std::make_pair("title", VariableType(VariableTypeID::String)),
                         std::make_pair("text", VariableType(VariableTypeID::String)),
                         std::make_pair("args",
                                        VariableType(VariableType(VariableTypeID::String),
                                                     VariableType(VariableTypeID::Any)))}))},
   {"msgbox",
    new BuiltinFunctionInfo(
       []() {
	       return _("Shows a story message box. The args table may contain the following keys:\n"
	                " · 'field' – center the view on this field\n"
	                " · 'w' – message box width in pixels (default: 400)\n"
	                " · 'h' – message box height in pixels (default: 300)\n"
	                " · 'posx' – horizontal message box position in pixels (default: centered)\n"
	                " · 'posy' – vertical message box position in pixels (default: centered)\n"
	                "All keys in args are optional. args may also be nil.");
	    },
       new FunctionBase("message_box",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Nil),     // returns
                        {std::make_pair("title", VariableType(VariableTypeID::String)),
                         std::make_pair("text", VariableType(VariableTypeID::String)),
                         std::make_pair("args",
                                        VariableType(VariableType(VariableTypeID::String),
                                                     VariableType(VariableTypeID::Any)))}))},
   {"player_sees",
    new BuiltinFunctionInfo(
       []() { return _("Whether the player can currently see the given field."); },
       new FunctionBase("sees_field",
                        VariableType(VariableTypeID::Player),   // call on
                        VariableType(VariableTypeID::Boolean),  // returns
                        {std::make_pair("field", VariableType(VariableTypeID::Field))}))},
   {"player_seen",
    new BuiltinFunctionInfo(
       []() { return _("Whether the player can currently see or has ever seen the given field."); },
       new FunctionBase("seen_field",
                        VariableType(VariableTypeID::Player),   // call on
                        VariableType(VariableTypeID::Boolean),  // returns
                        {std::make_pair("field", VariableType(VariableTypeID::Field))}))},
   {"player_reveal",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Reveal the specified fields to the player until they are explicitly hidden again.");
	    },
       new FunctionBase("reveal_fields",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Nil),     // returns
                        {std::make_pair("fields",
                                        VariableType(VariableType(VariableTypeID::Integer),
                                                     VariableType(VariableTypeID::Field)))}))},
   {"player_hide",
    new BuiltinFunctionInfo(
       []() { return _("Undo the effects of reveal_fields()."); },
       new FunctionBase("hide_fields",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Nil),     // returns
                        {std::make_pair("fields",
                                        VariableType(VariableType(VariableTypeID::Integer),
                                                     VariableType(VariableTypeID::Field)))}))},
   {"allow_bld",
    new BuiltinFunctionInfo(
       []() { return _("Allow the player to build the specified buildings."); },
       new FunctionBase("allow_buildings",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Nil),     // returns
                        {
                           std::make_pair("buildings",
                                          VariableType(VariableType(VariableTypeID::Integer),
                                                       VariableType(VariableTypeID::Building)))}))},
   {"forbid_bld", new BuiltinFunctionInfo(
                     []() { return _("Forbid the player to build the specified buildings."); },
                     new FunctionBase("forbid_buildings",
                                      VariableType(VariableTypeID::Player),  // call on
                                      VariableType(VariableTypeID::Nil),     // returns
                                      {std::make_pair(
                                         "buildings",
                                         VariableType(VariableType(VariableTypeID::Integer),
                                                      VariableType(VariableTypeID::Building)))}))},
   {"scenario_solved",
    new BuiltinFunctionInfo(
       []() { return _("Marks the current scenario as solved."); },
       new FunctionBase("mark_scenario_as_solved",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Nil),     // returns
                        {std::make_pair("scenario_name", VariableType(VariableTypeID::String))}))},
   {"player_ships",
    new BuiltinFunctionInfo(
       []() { return _("Returns an array of all the player’s ships."); },
       new FunctionBase("get_ships",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableType(VariableTypeID::Integer),
                                     VariableType(VariableTypeID::Ship)),  // returns
                        {}))},
   {"player_buildings",
    new BuiltinFunctionInfo(
       []() { return _("Returns an array of all the player’s buildings of the specified type."); },
       new FunctionBase(
          "get_buildings",
          VariableType(VariableTypeID::Player),  // call on
          VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Building)),  // returns
          {std::make_pair("internal_building_name", VariableType(VariableTypeID::String))}))},
   {"player_suitability",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Checks whether the player may build the specified building type on the given field.");
	    },
       new FunctionBase(
          "get_suitability",
          VariableType(VariableTypeID::Player),  // call on
          VariableType(VariableTypeID::Nil),     // returns
          {std::make_pair("internal_building_name", VariableType(VariableTypeID::String)),
           std::make_pair("field", VariableType(VariableTypeID::Field))}))},
   {"switchplayer",
    new BuiltinFunctionInfo(
       []() { return _("Switch to the player with the given player number."); },
       new FunctionBase("switchplayer",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Nil),     // returns
                        {std::make_pair("player_number", VariableType(VariableTypeID::Integer))}))},
   {"player_produced",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns the number of units the player produced of a given ware "
	                "since the game was started.");
	    },
       new FunctionBase(
          "switchplayer",
          VariableType(VariableTypeID::Player),   // call on
          VariableType(VariableTypeID::Integer),  // returns
          {std::make_pair("internal_ware_name", VariableType(VariableTypeID::String))}))},
   {"is_attack_forbidden",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns whether the player is currently forbidden to attack the player "
	                "with the given player number.");
	    },
       new FunctionBase("is_attack_forbidden",
                        VariableType(VariableTypeID::Player),   // call on
                        VariableType(VariableTypeID::Boolean),  // returns
                        {std::make_pair("player_number", VariableType(VariableTypeID::Integer))}))},
   {"set_attack_forbidden",
    new BuiltinFunctionInfo(
       []() {
	       return _("Sets whether the player is forbidden to attack the player with the given "
	                "player number.");
	    },
       new FunctionBase("set_attack_forbidden",
                        VariableType(VariableTypeID::Player),  // call on
                        VariableType(VariableTypeID::Nil),     // returns
                        {std::make_pair("player_number", VariableType(VariableTypeID::Integer)),
                         std::make_pair("forbid", VariableType(VariableTypeID::Boolean))}))},

   // Map

   {"field", new BuiltinFunctionInfo(
                []() { return _("Returns the field with the given coordinates."); },
                new FunctionBase("get_field",
                                 VariableType(VariableTypeID::Map),    // call on
                                 VariableType(VariableTypeID::Field),  // returns
                                 {std::make_pair("x", VariableType(VariableTypeID::Integer)),
                                  std::make_pair("y", VariableType(VariableTypeID::Integer))}))},
   {"place_immovable",
    new BuiltinFunctionInfo(
       []() { return _("Places the given 'world' or 'tribe' immovable on the given field."); },
       new FunctionBase(
          "place_immovable",
          VariableType(VariableTypeID::Map),        // call on
          VariableType(VariableTypeID::Immovable),  // returns
          {std::make_pair("internal_immovable_name", VariableType(VariableTypeID::String)),
           std::make_pair("field", VariableType(VariableTypeID::Field)),
           std::make_pair("world_or_tribe", VariableType(VariableTypeID::String))}))},
   {"map_recalc", new BuiltinFunctionInfo(
                     []() {
	                     return _(
	                        "Needs to be called after changing the raw_height of a field to update "
	                        "slope steepness, buildcaps etc.");
	                  },
                     new FunctionBase("recalculate",
                                      VariableType(VariableTypeID::Map),  // call on
                                      VariableType(VariableTypeID::Nil),  // returns
                                      {}))},
   {"map_recalc_sf",
    new BuiltinFunctionInfo(
       []() {
	       return _("Check again whether the maps allows seafaring. Needs to be called only "
	                "after changing watery terrains.");
	    },
       new FunctionBase("recalculate_seafaring",
                        VariableType(VariableTypeID::Map),  // call on
                        VariableType(VariableTypeID::Nil),  // returns
                        {}))},
   {"map_setport", new BuiltinFunctionInfo(
                      []() {
	                      return _("Sets whether a port may be built at the given coordinates. "
	                               "Returns false if this could not be set.");
	                   },
                      new FunctionBase("set_port_space",
                                       VariableType(VariableTypeID::Map),      // call on
                                       VariableType(VariableTypeID::Boolean),  // returns
                                       {std::make_pair("x", VariableType(VariableTypeID::Integer)),
                                        std::make_pair("y", VariableType(VariableTypeID::Integer)),
                                        std::make_pair("allow_port_space",
                                                       VariableType(VariableTypeID::Boolean))}))},

   // Field

   {"field_region",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns an array with all fields within a certain radius of this field.");
	    },
       new FunctionBase("region",
                        VariableType(VariableTypeID::Field),  // call on
                        VariableType(VariableType(VariableTypeID::Integer),
                                     VariableType(VariableTypeID::Field)),  // returns
                        {std::make_pair("radius", VariableType(VariableTypeID::Integer))}))},
   {"field_region_hollow",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Returns an array with all fields inside a certain hollow area around this field.");
	    },
       new FunctionBase("region",
                        VariableType(VariableTypeID::Field),  // call on
                        VariableType(VariableType(VariableTypeID::Integer),
                                     VariableType(VariableTypeID::Field)),  // returns
                        {std::make_pair("outer_radius", VariableType(VariableTypeID::Integer)),
                         std::make_pair("inner_radius", VariableType(VariableTypeID::Integer))}))},
   {"field_caps",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns whether this field has the given caps. Valid caps are:\n"
	                " · 'small' (suited for small buildings)\n"
	                " · 'medium' (suited for  buildings)\n"
	                " · 'big' (suited for big buildings)\n"
	                " · 'port' (suited for ports)\n"
	                " · 'mine' (suited for mines)\n"
	                " · 'flag' (suited for a flag)\n"
	                " · 'walkable' (bobs can walk here)\n"
	                " · 'swimmable' (bobs can swim here)");
	    },
       new FunctionBase("has_caps",
                        VariableType(VariableTypeID::Field),    // call on
                        VariableType(VariableTypeID::Boolean),  // returns
                        {std::make_pair("caps", VariableType(VariableTypeID::String))}))},
   {"field_maxcaps",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns whether this field has the given caps when "
	                "not taking nearby immovables into account.");
	    },
       new FunctionBase("has_max_caps",
                        VariableType(VariableTypeID::Field),    // call on
                        VariableType(VariableTypeID::Boolean),  // returns
                        {std::make_pair("caps", VariableType(VariableTypeID::String))}))},

   // TribeDescr

   {"tribe_has_bld",
    new BuiltinFunctionInfo(
       []() { return _("Whether this tribe uses a building with the given name."); },
       new FunctionBase("has_building",
                        VariableType(VariableTypeID::TribeDescr),  // call on
                        VariableType(VariableTypeID::Boolean),     // returns
                        {std::make_pair("internal_name", VariableType(VariableTypeID::String))}))},
   {"tribe_has_ware",
    new BuiltinFunctionInfo(
       []() { return _("Whether this tribe uses a ware with the given name."); },
       new FunctionBase("has_ware",
                        VariableType(VariableTypeID::TribeDescr),  // call on
                        VariableType(VariableTypeID::Boolean),     // returns
                        {std::make_pair("internal_name", VariableType(VariableTypeID::String))}))},
   {"tribe_has_worker",
    new BuiltinFunctionInfo(
       []() { return _("Whether this tribe uses a worker with the given name."); },
       new FunctionBase("has_worker",
                        VariableType(VariableTypeID::TribeDescr),  // call on
                        VariableType(VariableTypeID::Boolean),     // returns
                        {std::make_pair("internal_name", VariableType(VariableTypeID::String))}))},

   // ImmovableDescr

   {"immo_attr",
    new BuiltinFunctionInfo(
       []() { return _("Whether this immovable has the given attribute."); },
       new FunctionBase("has_attribute",
                        VariableType(VariableTypeID::ImmovableDescr),  // call on
                        VariableType(VariableTypeID::Boolean),         // returns
                        {std::make_pair("attribute", VariableType(VariableTypeID::String))}))},
   {"immo_growchance",
    new BuiltinFunctionInfo(
       []() { return _("Calculates the chance of this immovable to grow on the given terrain."); },
       new FunctionBase("probability_to_grow",
                        VariableType(VariableTypeID::ImmovableDescr),  // call on
                        VariableType(VariableTypeID::Double),          // returns
                        {std::make_pair("terrain", VariableType(VariableTypeID::TerrainDescr))}))},

   // ProductionSiteDescr

   {"prodsite_prodwares",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns which wares are produced by a certain production program of this "
	                "productionsite as a table of {ware_name = ware_amount} pairs.");
	    },
       new FunctionBase("produced_wares",
                        VariableType(VariableTypeID::ProductionSiteDescr),  // call on
                        VariableType(VariableType(VariableTypeID::String),
                                     VariableType(VariableTypeID::Integer)),  // returns
                        {std::make_pair("production_program_internal_name",
                                        VariableType(VariableTypeID::String))}))},
   {"prodsite_recruits",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns which workers are recruited by a certain production program of this "
	                "productionsite as a table of {worker_name = worker_amount} pairs.");
	    },
       new FunctionBase("recruited_workers",
                        VariableType(VariableTypeID::ProductionSiteDescr),  // call on
                        VariableType(VariableType(VariableTypeID::String),
                                     VariableType(VariableTypeID::Integer)),  // returns
                        {std::make_pair("production_program_internal_name",
                                        VariableType(VariableTypeID::String))}))},

   // WareDescr

   {"waredescr_consumers",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns an array with the BuildingDescriptions of all buildings "
	                "of the given tribe that require this ware for production.");
	    },
       new FunctionBase(
          "consumers",
          VariableType(VariableTypeID::WareDescr),  // call on
          VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::BuildingDescr)),  // returns
          {std::make_pair("tribe_internal_name", VariableType(VariableTypeID::String))}))},
   {"waredescr_producers",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns an array with the BuildingDescriptions of all buildings "
	                "of the given tribe that produce this ware.");
	    },
       new FunctionBase(
          "producers",
          VariableType(VariableTypeID::WareDescr),  // call on
          VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::BuildingDescr)),  // returns
          {std::make_pair("tribe_internal_name", VariableType(VariableTypeID::String))}))},
   {"waredescr_isconstructmat",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns whether the given tribe needs this ware for any constructionsite.");
	    },
       new FunctionBase(
          "is_construction_material",
          VariableType(VariableTypeID::WareDescr),  // call on
          VariableType(VariableTypeID::Boolean),    // returns
          {std::make_pair("tribe_internal_name", VariableType(VariableTypeID::String))}))},

   // Economy

   {"eco_target_get",
    new BuiltinFunctionInfo(
       []() { return _("Returns the target quantity for the given ware/worker in this economy."); },
       new FunctionBase("target_quantity",
                        VariableType(VariableTypeID::Economy),  // call on
                        VariableType(VariableTypeID::Integer),  // returns
                        {std::make_pair("internal_name", VariableType(VariableTypeID::String))}))},
   {"eco_target_set",
    new BuiltinFunctionInfo(
       []() { return _("Sets the target quantity for the given ware/worker in this economy."); },
       new FunctionBase("set_target_quantity",
                        VariableType(VariableTypeID::Economy),  // call on
                        VariableType(VariableTypeID::Nil),      // returns
                        {std::make_pair("internal_name", VariableType(VariableTypeID::String)),
                         std::make_pair("amount", VariableType(VariableTypeID::Integer))}))},

   // MapObject

   {"mo_destroy", new BuiltinFunctionInfo(
                     []() {
	                     return _("Destroy this map object immediately. May have special effects, "
	                              "e.g. a building will go up in flames.");
	                  },
                     new FunctionBase("destroy",
                                      VariableType(VariableTypeID::MapObject),  // call on
                                      VariableType(VariableTypeID::Nil),        // returns
                                      {}))},
   {"mo_remove",
    new BuiltinFunctionInfo(
       []() { return _("Remove this map object immediately and without any special effects."); },
       new FunctionBase("remove",
                        VariableType(VariableTypeID::MapObject),  // call on
                        VariableType(VariableTypeID::Nil),        // returns
                        {}))},

   // Flag

   {"flag_get_wares",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns the number of wares of a certain type currently waiting on this flag.");
	    },
       new FunctionBase("get_wares",
                        VariableType(VariableTypeID::Flag),     // call on
                        VariableType(VariableTypeID::Integer),  // returns
                        {std::make_pair("ware_name", VariableType(VariableTypeID::String))}))},
   {"flag_set_wares",
    new BuiltinFunctionInfo(
       []() {
	       return _("Sets the number of wares of a certain type currently waiting in this flag.");
	    },
       new FunctionBase("set_wares",
                        VariableType(VariableTypeID::Flag),  // call on
                        VariableType(VariableTypeID::Nil),   // returns
                        {std::make_pair("ware_name", VariableType(VariableTypeID::String)),
                         std::make_pair("amount", VariableType(VariableTypeID::Integer))}))},

   // Road

   {"road_get_workers",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Returns the number of workers of a certain type currently employed on this road.");
	    },
       new FunctionBase("get_workers",
                        VariableType(VariableTypeID::Road),     // call on
                        VariableType(VariableTypeID::Integer),  // returns
                        {std::make_pair("worker_name", VariableType(VariableTypeID::String))}))},
   {"road_set_workers",
    new BuiltinFunctionInfo(
       []() {
	       return _("Sets the number of workers of a certain type currently employed on this road.");
	    },
       new FunctionBase("set_workers",
                        VariableType(VariableTypeID::Road),  // call on
                        VariableType(VariableTypeID::Nil),   // returns
                        {std::make_pair("worker_name", VariableType(VariableTypeID::String)),
                         std::make_pair("amount", VariableType(VariableTypeID::Integer))}))},

   // Building

   {"dismantle",
    new BuiltinFunctionInfo([]() { return _("Immediately dismantle this building."); },
                            new FunctionBase("dismantle",
                                             VariableType(VariableTypeID::Building),  // call on
                                             VariableType(VariableTypeID::Nil),       // returns
                                             {}))},

   // ConstructionSite

   {"cs_get_priority",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns the priority for the given ware inputqueue (one of 2 (low), 4 "
	                "(normal), or 8 (high)).");
	    },
       new FunctionBase(
          "get_priority",
          VariableType(VariableTypeID::ConstructionSite),  // call on
          VariableType(VariableTypeID::Integer),           // returns
          {std::make_pair("ware_name", VariableType(VariableTypeID::String)),
           std::make_pair("is_in_building_settings", VariableType(VariableTypeID::Boolean))}))},
   {"cs_set_priority",
    new BuiltinFunctionInfo(
       []() {
	       return _("Sets the priority for the given ware inputqueue. Allowed values are one of 2 "
	                "(low), 4 (normal), or 8 (high).");
	    },
       new FunctionBase(
          "set_priority",
          VariableType(VariableTypeID::ConstructionSite),  // call on
          VariableType(VariableTypeID::Nil),               // returns
          {std::make_pair("ware_name", VariableType(VariableTypeID::String)),
           std::make_pair("priority", VariableType(VariableTypeID::Integer)),
           std::make_pair("is_in_building_settings", VariableType(VariableTypeID::Boolean))}))},
   {"cs_get_desired_fill",
    new BuiltinFunctionInfo(
       []() { return _("Returns the desired fill for the given ware or worker inputqueue."); },
       new FunctionBase(
          "get_desired_fill",
          VariableType(VariableTypeID::ConstructionSite),  // call on
          VariableType(VariableTypeID::Integer),           // returns
          {std::make_pair("ware_or_worker_name", VariableType(VariableTypeID::String)),
           std::make_pair("is_in_building_settings", VariableType(VariableTypeID::Boolean))}))},
   {"cs_set_desired_fill",
    new BuiltinFunctionInfo(
       []() { return _("Sets the desired fill for the given ware or worker inputqueue."); },
       new FunctionBase(
          "set_desired_fill",
          VariableType(VariableTypeID::ConstructionSite),  // call on
          VariableType(VariableTypeID::Nil),               // returns
          {std::make_pair("ware_or_worker_name", VariableType(VariableTypeID::String)),
           std::make_pair("amount", VariableType(VariableTypeID::Integer)),
           std::make_pair("is_in_building_settings", VariableType(VariableTypeID::Boolean))}))},
   {"cs_get_setting_warehouse_policy",
    new BuiltinFunctionInfo(
       []() {
	       return _("Only valid for warehouses under construction: Returns the stock policy to "
	                "apply to the given ware or worker after completion.");
	    },
       new FunctionBase(
          "get_setting_warehouse_policy",
          VariableType(VariableTypeID::ConstructionSite),  // call on
          VariableType(VariableTypeID::String),            // returns
          {std::make_pair("ware_or_worker_name", VariableType(VariableTypeID::String))}))},
   {"cs_set_setting_warehouse_policy",
    new BuiltinFunctionInfo(
       []() {
	       return _("Only valid for warehouses under construction: Sets the stock policy to apply "
	                "to the given ware or worker after completion.");
	    },
       new FunctionBase(
          "set_setting_warehouse_policy",
          VariableType(VariableTypeID::ConstructionSite),  // call on
          VariableType(VariableTypeID::Nil),               // returns
          {std::make_pair("ware_or_worker_name", VariableType(VariableTypeID::String)),
           std::make_pair("policy", VariableType(VariableTypeID::String))}))},

   // ProductionSite

   {"togglestartstop",
    new BuiltinFunctionInfo(
       []() { return _("If this productionsite is stopped), start it; otherwise stop it."); },
       new FunctionBase("toggle_start_stop",
                        VariableType(VariableTypeID::ProductionSite),  // call on
                        VariableType(VariableTypeID::Nil),             // returns
                        {}))},
   {"ps_get_workers",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns the number of workers of a certain type currently employed "
	                "in this productionsite.");
	    },
       new FunctionBase("get_workers",
                        VariableType(VariableTypeID::ProductionSite),  // call on
                        VariableType(VariableTypeID::Integer),         // returns
                        {std::make_pair("worker_name", VariableType(VariableTypeID::String))}))},
   {"ps_set_workers",
    new BuiltinFunctionInfo(
       []() {
	       return _("Sets the number of workers of a certain type currently "
	                "employed in this productionsite.");
	    },
       new FunctionBase("set_workers",
                        VariableType(VariableTypeID::ProductionSite),  // call on
                        VariableType(VariableTypeID::Nil),             // returns
                        {std::make_pair("worker_name", VariableType(VariableTypeID::String)),
                         std::make_pair("amount", VariableType(VariableTypeID::Integer))}))},
   {"ps_get_priority",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns the priority for the given ware inputqueue (one of 2 (low), 4 "
	                "(normal), or 8 (high)).");
	    },
       new FunctionBase("get_priority",
                        VariableType(VariableTypeID::ConstructionSite),  // call on
                        VariableType(VariableTypeID::Integer),           // returns
                        {std::make_pair("ware_name", VariableType(VariableTypeID::String))}))},
   {"ps_set_priority",
    new BuiltinFunctionInfo(
       []() {
	       return _("Sets the priority for the given ware inputqueue. Allowed values are one of 2 "
	                "(low), 4 (normal), or 8 (high).");
	    },
       new FunctionBase("set_priority",
                        VariableType(VariableTypeID::ConstructionSite),  // call on
                        VariableType(VariableTypeID::Nil),               // returns
                        {std::make_pair("ware_name", VariableType(VariableTypeID::String)),
                         std::make_pair("priority", VariableType(VariableTypeID::Integer))}))},
   {"ps_get_desired_fill",
    new BuiltinFunctionInfo(
       []() { return _("Returns the desired fill for the given ware or worker inputqueue."); },
       new FunctionBase(
          "get_desired_fill",
          VariableType(VariableTypeID::ConstructionSite),  // call on
          VariableType(VariableTypeID::Integer),           // returns
          {std::make_pair("ware_or_worker_name", VariableType(VariableTypeID::String))}))},
   {"ps_set_desired_fill",
    new BuiltinFunctionInfo(
       []() { return _("Sets the desired fill for the given ware or worker inputqueue."); },
       new FunctionBase(
          "set_desired_fill",
          VariableType(VariableTypeID::ConstructionSite),  // call on
          VariableType(VariableTypeID::Nil),               // returns
          {std::make_pair("ware_or_worker_name", VariableType(VariableTypeID::String)),
           std::make_pair("amount", VariableType(VariableTypeID::Integer))}))},

   // MilitarySite

   {"ms_get_soldiers",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns the number of soldiers matching the given soldier description "
	                "currently garrisoned here. A soldier description is an array that contains "
	                "the levels for health, attack, defense and evade (in this order).");
	    },
       new FunctionBase(
          "get_soldiers",
          VariableType(VariableTypeID::MilitarySite),  // call on
          VariableType(VariableTypeID::Integer),       // returns
          {std::make_pair("soldier_descr",
                          VariableType(VariableType(VariableType(VariableTypeID::Integer),
                                                    VariableType(VariableTypeID::Integer)),
                                       VariableType(VariableTypeID::Integer)))}))},
   {"ms_set_soldiers",
    new BuiltinFunctionInfo(
       []() {
	       return _("Sets the number of soldiers garrisoned here. "
	                "The argument is a table of soldier_description:amount pairs. "
	                "A soldier description is an array that contains "
	                "the levels for health, attack, defense and evade (in this order).");
	    },
       new FunctionBase(
          "set_soldiers",
          VariableType(VariableTypeID::MilitarySite),  // call on
          VariableType(VariableTypeID::Nil),           // returns
          {std::make_pair("soldiers",
                          VariableType(VariableType(VariableType(VariableTypeID::Integer),
                                                    VariableType(VariableTypeID::Integer)),
                                       VariableType(VariableTypeID::Integer)))}))},

   // TrainingSite

   {"ts_get_soldiers",
    new BuiltinFunctionInfo(
       []() {
	       return _("Returns the number of soldiers matching the given soldier description "
	                "in training here. A soldier description is an array that contains "
	                "the levels for health, attack, defense and evade (in this order).");
	    },
       new FunctionBase(
          "get_soldiers",
          VariableType(VariableTypeID::TrainingSite),  // call on
          VariableType(VariableTypeID::Integer),       // returns
          {std::make_pair("soldier_descr",
                          VariableType(VariableType(VariableType(VariableTypeID::Integer),
                                                    VariableType(VariableTypeID::Integer)),
                                       VariableType(VariableTypeID::Integer)))}))},
   {"ts_set_soldiers",
    new BuiltinFunctionInfo(
       []() {
	       return _("Sets the number of soldiers in training here. "
	                "The argument is a table of soldier_description:amount pairs. "
	                "A soldier description is an array that contains "
	                "the levels for health, attack, defense and evade (in this order).");
	    },
       new FunctionBase(
          "set_soldiers",
          VariableType(VariableTypeID::TrainingSite),  // call on
          VariableType(VariableTypeID::Nil),           // returns
          {std::make_pair("soldiers",
                          VariableType(VariableType(VariableType(VariableTypeID::Integer),
                                                    VariableType(VariableTypeID::Integer)),
                                       VariableType(VariableTypeID::Integer)))}))},

   // Warehouse

   {"wh_get_wares",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Returns the number of wares of a certain type currently stored in this warehouse.");
	    },
       new FunctionBase("get_wares",
                        VariableType(VariableTypeID::Warehouse),  // call on
                        VariableType(VariableTypeID::Integer),    // returns
                        {std::make_pair("ware_name", VariableType(VariableTypeID::String))}))},
   {"wh_set_wares",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Sets the number of wares of a certain type currently stored in this warehouse.");
	    },
       new FunctionBase("set_wares",
                        VariableType(VariableTypeID::Warehouse),  // call on
                        VariableType(VariableTypeID::Nil),        // returns
                        {std::make_pair("ware_name", VariableType(VariableTypeID::String)),
                         std::make_pair("amount", VariableType(VariableTypeID::Integer))}))},
   {"wh_get_workers",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Returns the number of workers of a certain type currently stored in this warehouse.");
	    },
       new FunctionBase("get_workers",
                        VariableType(VariableTypeID::Warehouse),  // call on
                        VariableType(VariableTypeID::Integer),    // returns
                        {std::make_pair("worker_name", VariableType(VariableTypeID::String))}))},
   {"wh_set_workers",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Sets the number of workers of a certain type currently stored in this warehouse.");
	    },
       new FunctionBase("set_workers",
                        VariableType(VariableTypeID::Warehouse),  // call on
                        VariableType(VariableTypeID::Nil),        // returns
                        {std::make_pair("worker_name", VariableType(VariableTypeID::String)),
                         std::make_pair("amount", VariableType(VariableTypeID::Integer))}))},
   {"wh_get_soldiers",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Returns the number of soldiers matching the given soldier description "
	          "currently stored in this warehouse. A soldier description is an array that contains "
	          "the levels for health, attack, defense and evade (in this order).");
	    },
       new FunctionBase(
          "get_soldiers",
          VariableType(VariableTypeID::Warehouse),  // call on
          VariableType(VariableTypeID::Integer),    // returns
          {std::make_pair("soldier_descr",
                          VariableType(VariableType(VariableType(VariableTypeID::Integer),
                                                    VariableType(VariableTypeID::Integer)),
                                       VariableType(VariableTypeID::Integer)))}))},
   {"wh_set_soldiers",
    new BuiltinFunctionInfo(
       []() {
	       return _("Sets the number of soldiers currently stored in this warehouse. "
	                "The argument is a table of soldier_description:amount pairs. "
	                "A soldier description is an array that contains "
	                "the levels for health, attack, defense and evade (in this order).");
	    },
       new FunctionBase(
          "set_soldiers",
          VariableType(VariableTypeID::Warehouse),  // call on
          VariableType(VariableTypeID::Nil),        // returns
          {std::make_pair("soldiers",
                          VariableType(VariableType(VariableType(VariableTypeID::Integer),
                                                    VariableType(VariableTypeID::Integer)),
                                       VariableType(VariableTypeID::Integer)))}))},
   {"wh_setpol",
    new BuiltinFunctionInfo(
       []() {
	       return _("Set this warehouse’s storage policy for the given ware or worker. "
	                "Valid policies are 'normal', 'prefer', 'dontstock', and 'remove'.");
	    },
       new FunctionBase("set_warehouse_policy",
                        VariableType(VariableTypeID::Warehouse),  // call on
                        VariableType(VariableTypeID::Nil),        // returns
                        {std::make_pair("internal_name", VariableType(VariableTypeID::String)),
                         std::make_pair("policy", VariableType(VariableTypeID::String))}))},
   {"wh_getpol",
    new BuiltinFunctionInfo(
       []() { return _("Returns this warehouse’s storage policy for the given ware or worker."); },
       new FunctionBase("get_warehouse_policy",
                        VariableType(VariableTypeID::Warehouse),  // call on
                        VariableType(VariableTypeID::String),     // returns
                        {std::make_pair("internal_name", VariableType(VariableTypeID::String))}))},
   {"wh_exp_start",
    new BuiltinFunctionInfo(
       []() { return _("Starts an expedition from this warehouse if it is a port."); },
       new FunctionBase("start_expedition",
                        VariableType(VariableTypeID::Warehouse),  // call on
                        VariableType(VariableTypeID::Nil),        // returns
                        {}))},
   {"wh_exp_cancel", new BuiltinFunctionInfo(
                        []() {
	                        return _(
	                           "Cancel the preparations for an expedition from this warehouse if it "
	                           "is a port and an expedition is being prepared.");
	                     },
                        new FunctionBase("start_expedition",
                                         VariableType(VariableTypeID::Warehouse),  // call on
                                         VariableType(VariableTypeID::Nil),        // returns
                                         {}))},

   // Bob

   {"bob_caps",
    new BuiltinFunctionInfo(
       []() { return _("Returns whether this bob can 'walk' or 'swim'."); },
       new FunctionBase("has_caps",
                        VariableType(VariableTypeID::Bob),      // call on
                        VariableType(VariableTypeID::Boolean),  // returns
                        {std::make_pair("caps", VariableType(VariableTypeID::String))}))},

   // Ship

   {"ship_get_wares",
    new BuiltinFunctionInfo([]() { return _("Returns the number of wares on this ship."); },
                            new FunctionBase("get_wares",
                                             VariableType(VariableTypeID::Ship),     // call on
                                             VariableType(VariableTypeID::Integer),  // returns
                                             {}))},
   {"ship_get_workers",
    new BuiltinFunctionInfo([]() { return _("Returns the number of workers on this ship."); },
                            new FunctionBase("get_workers",
                                             VariableType(VariableTypeID::Ship),     // call on
                                             VariableType(VariableTypeID::Integer),  // returns
                                             {}))},
   {"ship_buildport",
    new BuiltinFunctionInfo(
       []() {
	       return _("If this ship is an expedition ship that has found a port space), start "
	                "building a colonization port. Returns whether colonising was started.");
	    },
       new FunctionBase("build_colonization_port",
                        VariableType(VariableTypeID::Ship),     // call on
                        VariableType(VariableTypeID::Boolean),  // returns
                        {}))},
   {"ship_make_expedition",
    new BuiltinFunctionInfo(
       []() {
	       return _(
	          "Turns this ship into an expedition ship without a base port. Creates all necessary "
	          "wares and a builder plus the specified additional items. "
	          "The ship must be empty when this method is called.");
	    },
       new FunctionBase("make_expedition",
                        VariableType(VariableTypeID::Ship),  // call on
                        VariableType(VariableTypeID::Nil),   // returns
                        {std::make_pair("items",
                                        VariableType(VariableType(VariableTypeID::String),
                                                     VariableType(VariableTypeID::Integer)))}))},
};

/************************************************************
                      Builtin properties
************************************************************/

const std::map<std::string, BuiltinPropertyInfo*> kBuiltinProperties = {

   // Game

   {"map", new BuiltinPropertyInfo([]() { return _("The map instance."); },
                                   new Property("map",
                                                Property::Access::RO,
                                                VariableType(VariableTypeID::Game),  // class
                                                VariableType(VariableTypeID::Map)    // type
                                                ))},
   {"real_speed",
    new BuiltinPropertyInfo(
       []() {
	       return _("The speed the game is currently running at in milliseconds per real second.");
	    },
       new Property("real_speed",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Game),    // class
                    VariableType(VariableTypeID::Integer)  // type
                    ))},
   {"desired_speed", new BuiltinPropertyInfo(
                        []() {
	                        return _(
	                           "Sets the desired speed of the game in milliseconds per real second. "
	                           "Note that this will not work in network games as expected.");
	                     },
                        new Property("desired_speed",
                                     Property::Access::RW,
                                     VariableType(VariableTypeID::Game),    // class
                                     VariableType(VariableTypeID::Integer)  // type
                                     ))},
   {"gametime",
    new BuiltinPropertyInfo(
       []() { return _("The absolute time elapsed since the game was started in milliseconds."); },
       new Property("time",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Game),    // class
                    VariableType(VariableTypeID::Integer)  // type
                    ))},
   {"gametype",
    new BuiltinPropertyInfo(
       []() {
	       return _(
	          "One string out of ‘undefined’, ‘singleplayer’, ‘netclient’, ‘nethost’, ‘replay’, "
	          "describing the type of game that is played.");
	    },
       new Property("type",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Game),   // class
                    VariableType(VariableTypeID::String)  // type
                    ))},
   {"scenario_difficulty",
    new BuiltinPropertyInfo(
       []() {
	       return _("The difficulty level of the current scenario. Values range from 1 "
	                "to the number of levels specified in the campaign's configuration in "
	                "campaigns.lua. By convention higher values mean more difficult.");
	    },
       new Property("scenario_difficulty",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Game),    // class
                    VariableType(VariableTypeID::Integer)  // type
                    ))},
   {"interactive_player",
    new BuiltinPropertyInfo(
       []() { return _("The player number of the interactive player, or 0 for spectators."); },
       new Property("interactive_player",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Game),    // class
                    VariableType(VariableTypeID::Integer)  // type
                    ))},
   {"players",
    new BuiltinPropertyInfo([]() { return _("An array with the player instances."); },
                            new Property("players",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Game),  // class
                                         VariableType(VariableType(VariableTypeID::Integer),
                                                      VariableType(VariableTypeID::Player))  // type
                                         ))},

   // Player

   {"pl_number",
    new BuiltinPropertyInfo([]() { return _("The player’s player number."); },
                            new Property("number",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Player),  // class
                                         VariableType(VariableTypeID::Integer)  // type
                                         ))},
   {"pl_name", new BuiltinPropertyInfo([]() { return _("The player’s name."); },
                                       new Property("name",
                                                    Property::Access::RO,
                                                    VariableType(VariableTypeID::Player),  // class
                                                    VariableType(VariableTypeID::String)   // type
                                                    ))},
   {"pl_tribename",
    new BuiltinPropertyInfo([]() { return _("The name of the player’s tribe."); },
                            new Property("tribe_name",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Player),  // class
                                         VariableType(VariableTypeID::String)   // type
                                         ))},
   {"pl_tribe",
    new BuiltinPropertyInfo([]() { return _("The player’s tribe."); },
                            new Property("tribe",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Player),     // class
                                         VariableType(VariableTypeID::TribeDescr)  // type
                                         ))},
   {"pl_color", new BuiltinPropertyInfo(
                   []() { return _("The playercolor assigned to this Player), in hex notation."); },
                   new Property("color",
                                Property::Access::RO,
                                VariableType(VariableTypeID::Player),  // class
                                VariableType(VariableTypeID::String)   // type
                                ))},
   {"pl_team",
    new BuiltinPropertyInfo([]() { return _("The player’s team number), 0 meaning no team."); },
                            new Property("team",
                                         Property::Access::RW,
                                         VariableType(VariableTypeID::Player),  // class
                                         VariableType(VariableTypeID::Integer)  // type
                                         ))},
   {"pl_defeated",
    new BuiltinPropertyInfo([]() { return _("Whether this player was defeated."); },
                            new Property("defeated",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Player),  // class
                                         VariableType(VariableTypeID::Boolean)  // type
                                         ))},
   {"pl_see_all",
    new BuiltinPropertyInfo(
       []() {
	       return _("If you set this to true, the map will be completely visible for this player.");
	    },
       new Property("see_all",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Player),  // class
                    VariableType(VariableTypeID::Boolean)  // type
                    ))},
   {"pl_allowed_buildings",
    new BuiltinPropertyInfo(
       []() {
	       return _("A table with name:bool values with all buildings that are "
	                "currently allowed for this player. Note that you can not enable/forbid "
	                "a building by setting the value. Use allow_buildings() or forbid_buildings() "
	                "for that.");
	    },
       new Property("allowed_buildings",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Player),  // class
                    VariableType(VariableType(VariableTypeID::String),
                                 VariableType(VariableTypeID::Boolean))  // type
                    ))},
   {"pl_objectives",
    new BuiltinPropertyInfo(
       []() {
	       return _("A table of name:Objective. You can change the objectives in this table "
	                "and it will be reflected in the game. To add a new item, use add_objective().");
	    },
       new Property("objectives",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Player),  // class
                    VariableType(VariableType(VariableTypeID::String),
                                 VariableType(VariableTypeID::Objective))  // type
                    ))},
   {"pl_messages",
    new BuiltinPropertyInfo(
       []() {
	       return _("An array of all the messages sent to the player. Note that you can’t add "
	                "messages to this array), use send_message() for that.");
	    },
       new Property("messages",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Player),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableTypeID::Message))  // type
                    ))},
   {"pl_inbox", new BuiltinPropertyInfo(
                   []() {
	                   return _(
	                      "An array of the messages that are either read or new. Note that "
	                      "you can’t add messages to this array), use send_message() for that.");
	                },
                   new Property("inbox",
                                Property::Access::RO,
                                VariableType(VariableTypeID::Player),  // class
                                VariableType(VariableType(VariableTypeID::Integer),
                                             VariableType(VariableTypeID::Message))  // type
                                ))},

   // Objective

   {"obj_name",
    new BuiltinPropertyInfo(
       []() { return _("The internal name), to reference this objective in Player.objectives."); },
       new Property("name",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Objective),  // class
                    VariableType(VariableTypeID::String)      // type
                    ))},
   {"obj_title",
    new BuiltinPropertyInfo([]() { return _("The localized objective title."); },
                            new Property("title",
                                         Property::Access::RW,
                                         VariableType(VariableTypeID::Objective),  // class
                                         VariableType(VariableTypeID::String)      // type
                                         ))},
   {"obj_body",
    new BuiltinPropertyInfo(
       []() { return _("The complete localized objective text. May use richtext markup."); },
       new Property("body",
                    Property::Access::RW,
                    VariableType(VariableTypeID::Objective),  // class
                    VariableType(VariableTypeID::String)      // type
                    ))},
   {"obj_visible", new BuiltinPropertyInfo(
                      []() { return _("Whether this objective is shown in the Objectives menu."); },
                      new Property("visible",
                                   Property::Access::RW,
                                   VariableType(VariableTypeID::Objective),  // class
                                   VariableType(VariableTypeID::Boolean)     // type
                                   ))},
   {"obj_done", new BuiltinPropertyInfo(
                   []() {
	                   return _("Defines if this objective is already fulfilled. If done is true, "
	                            "the objective will not be shown to the user), no matter what "
	                            "visible is set to. A savegame will be created when this attribute "
	                            "is changed to true.");
	                },
                   new Property("done",
                                Property::Access::RW,
                                VariableType(VariableTypeID::Objective),  // class
                                VariableType(VariableTypeID::Boolean)     // type
                                ))},

   // Message

   {"msg_title",
    new BuiltinPropertyInfo([]() { return _("The message’s title."); },
                            new Property("title",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Message),  // class
                                         VariableType(VariableTypeID::String)    // type
                                         ))},
   {"msg_heading",
    new BuiltinPropertyInfo([]() { return _("The extended title."); },
                            new Property("heading",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Message),  // class
                                         VariableType(VariableTypeID::String)    // type
                                         ))},
   {"msg_body",
    new BuiltinPropertyInfo([]() { return _("The message’s full text."); },
                            new Property("body",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Message),  // class
                                         VariableType(VariableTypeID::String)    // type
                                         ))},
   {"msg_sent", new BuiltinPropertyInfo(
                   []() { return _("The game time in milliseconds when this message was sent."); },
                   new Property("sent",
                                Property::Access::RO,
                                VariableType(VariableTypeID::Message),  // class
                                VariableType(VariableTypeID::Integer)   // type
                                ))},
   {"msg_field",
    new BuiltinPropertyInfo([]() { return _("The field attached to this message), or nil."); },
                            new Property("field",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Message),  // class
                                         VariableType(VariableTypeID::Field)     // type
                                         ))},
   {"msg_icon_name",
    new BuiltinPropertyInfo(
       []() { return _("The filename for the icon that is shown with the message title."); },
       new Property("icon_name",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Message),  // class
                    VariableType(VariableTypeID::String)    // type
                    ))},
   {"msg_status",
    new BuiltinPropertyInfo([]() { return _("May be 'new', 'read', or 'archived'."); },
                            new Property(
                               "status",
                               Property::Access::RW,
                               VariableType(VariableTypeID::Message),  // class
                               VariableType(VariableTypeID::String)    // type
                               ))},

   // Map

   {"map_allows_seafaring",
    new BuiltinPropertyInfo([]() { return _("Whether seafaring is possible on this map."); },
                            new Property("allows_seafaring",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Map),     // class
                                         VariableType(VariableTypeID::Boolean)  // type
                                         ))},
   {"map_nrports",
    new BuiltinPropertyInfo([]() { return _("The number of port spaces on this map."); },
                            new Property("number_of_port_spaces",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Map),     // class
                                         VariableType(VariableTypeID::Integer)  // type
                                         ))},
   {"map_ports",
    new BuiltinPropertyInfo(
       []() {
	       return _("An array of tables of {x, y} pairs with the coordinates of the port spaces.");
	    },
       new Property("port_spaces",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Map),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableType(VariableTypeID::Integer),
                                              VariableType(VariableTypeID::Integer)))  // type
                    ))},
   {"map_w", new BuiltinPropertyInfo([]() { return _("The map’s width."); },
                                     new Property("width",
                                                  Property::Access::RO,
                                                  VariableType(VariableTypeID::Map),     // class
                                                  VariableType(VariableTypeID::Integer)  // type
                                                  ))},
   {"map_h", new BuiltinPropertyInfo([]() { return _("The map’s height."); },
                                     new Property(
                                        "height",
                                        Property::Access::RO,
                                        VariableType(VariableTypeID::Map),     // class
                                        VariableType(VariableTypeID::Integer)  // type
                                        ))},

   // TribeDescr

   {"td_name",
    new BuiltinPropertyInfo([]() { return _("The tribe’s internal name"); },
                            new Property("name",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::TribeDescr),  // class
                                         VariableType(VariableTypeID::String)       // type
                                         ))},
   {"td_descname",
    new BuiltinPropertyInfo([]() { return _("The tribe’s localized name"); },
                            new Property("descname",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::TribeDescr),  // class
                                         VariableType(VariableTypeID::String)       // type
                                         ))},
   {"td_carrier",
    new BuiltinPropertyInfo(
       []() { return _("The internal name of the carrier type that this tribe uses"); },
       new Property("carrier",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TribeDescr),  // class
                    VariableType(VariableTypeID::String)       // type
                    ))},
   {"td_carrier2",
    new BuiltinPropertyInfo(
       []() { return _("The internal name of the second carrier type that this tribe uses"); },
       new Property("carrier2",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TribeDescr),  // class
                    VariableType(VariableTypeID::String)       // type
                    ))},
   {"td_geologist",
    new BuiltinPropertyInfo(
       []() { return _("The internal name of the geologist that this tribe uses"); },
       new Property("geologist",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TribeDescr),  // class
                    VariableType(VariableTypeID::String)       // type
                    ))},
   {"td_soldier", new BuiltinPropertyInfo(
                     []() { return _("The internal name of the soldier that this tribe uses"); },
                     new Property("soldier",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::TribeDescr),  // class
                                  VariableType(VariableTypeID::String)       // type
                                  ))},
   {"td_ship", new BuiltinPropertyInfo(
                  []() { return _("The internal name of the ship that this tribe uses"); },
                  new Property("ship",
                               Property::Access::RO,
                               VariableType(VariableTypeID::TribeDescr),  // class
                               VariableType(VariableTypeID::String)       // type
                               ))},
   {"td_port", new BuiltinPropertyInfo(
                  []() { return _("The internal name of the port that this tribe uses"); },
                  new Property("port",
                               Property::Access::RO,
                               VariableType(VariableTypeID::TribeDescr),  // class
                               VariableType(VariableTypeID::String)       // type
                               ))},
   {"td_buildings",
    new BuiltinPropertyInfo(
       []() { return _("An array with all the BuildingDescriptions this tribe uses"); },
       new Property("buildings",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TribeDescr),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableTypeID::BuildingDescr))  // type
                    ))},
   {"td_workers", new BuiltinPropertyInfo(
                     []() { return _("An array with all the WorkerDescriptions this tribe uses"); },
                     new Property("workers",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::TribeDescr),  // class
                                  VariableType(VariableType(VariableTypeID::Integer),
                                               VariableType(VariableTypeID::WorkerDescr))  // type
                                  ))},
   {"td_wares", new BuiltinPropertyInfo(
                   []() { return _("An array with all the WareDescriptions this tribe uses"); },
                   new Property("wares",
                                Property::Access::RO,
                                VariableType(VariableTypeID::TribeDescr),  // class
                                VariableType(VariableType(VariableTypeID::Integer),
                                             VariableType(VariableTypeID::WorkerDescr))  // type
                                ))},
   {"td_immovables",
    new BuiltinPropertyInfo(
       []() { return _("An array with all the ImmovableDescriptions this tribe uses"); },
       new Property("immovables",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TribeDescr),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableTypeID::ImmovableDescr))  // type
                    ))},
   {"td_resi", new BuiltinPropertyInfo(
                  []() { return _("A table with the resource indicators this tribe uses"); },
                  new Property("resource_indicators",
                               Property::Access::RO,
                               VariableType(VariableTypeID::TribeDescr),  // class
                               VariableType(VariableType(VariableTypeID::Integer),
                                            VariableType(VariableTypeID::ImmovableDescr))  // type
                               ))},

   // MapObjectDescr

   {"mo_d_name",
    new BuiltinPropertyInfo([]() { return _("The map object’s internal name"); },
                            new Property("name",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::MapObjectDescr),  // class
                                         VariableType(VariableTypeID::String)           // type
                                         ))},
   {"mo_d_descname",
    new BuiltinPropertyInfo([]() { return _("The map object’s localized name"); },
                            new Property("descname",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::MapObjectDescr),  // class
                                         VariableType(VariableTypeID::String)           // type
                                         ))},
   {"mo_d_type",
    new BuiltinPropertyInfo([]() { return _("The map object’s type"); },
                            new Property("type_name",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::MapObjectDescr),  // class
                                         VariableType(VariableTypeID::String)           // type
                                         ))},
   {"mo_d_icon_name",
    new BuiltinPropertyInfo([]() { return _("The filename for the menu icon"); },
                            new Property("icon_name",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::MapObjectDescr),  // class
                                         VariableType(VariableTypeID::String)           // type
                                         ))},
   {"mo_d_help",
    new BuiltinPropertyInfo(
       []() { return _("The path and filename to the helptext script. Can be empty."); },
       new Property("helptext_script",
                    Property::Access::RO,
                    VariableType(VariableTypeID::MapObjectDescr),  // class
                    VariableType(VariableTypeID::String)           // type
                    ))},

   // ImmovableDescr

   {"immo_d_size",
    new BuiltinPropertyInfo(
       []() { return _("The size of this immovable: 'none', 'small', 'medium', or 'big'"); },
       new Property("size",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ImmovableDescr),  // class
                    VariableType(VariableTypeID::String)           // type
                    ))},
   {"immo_d_owner_type",
    new BuiltinPropertyInfo(
       []() { return _("'world' for world immovables and 'tribe' for tribe immovables"); },
       new Property("owner_type",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ImmovableDescr),  // class
                    VariableType(VariableTypeID::String)           // type
                    ))},
   {"immo_d_terrain_affinity",
    new BuiltinPropertyInfo(
       []() {
	       return _("A table containing numbers labeled as 'pickiness', "
	                "'preferred_fertility', 'preferred_humidity', and 'preferred_temperature', "
	                "or nil if the immovable has no terrain affinity");
	    },
       new Property("terrain_affinity",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ImmovableDescr),  // class
                    VariableType(VariableType(VariableTypeID::String),
                                 VariableType(VariableTypeID::Integer))  // type
                    ))},
   {"immo_d_species",
    new BuiltinPropertyInfo(
       []() {
	       return _("The localized species name of a tree (empty if this immovable is not a tree)");
	    },
       new Property("species",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ImmovableDescr),  // class
                    VariableType(VariableTypeID::String)           // type
                    ))},
   {"immo_d_buildcost",
    new BuiltinPropertyInfo(
       []() {
	       return _("A table of ware-to-count pairs describing the buildcost for the immovable");
	    },
       new Property("buildcost",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ImmovableDescr),  // class
                    VariableType(VariableType(VariableTypeID::String),
                                 VariableType(VariableTypeID::Integer))  // type
                    ))},

   // BuildingDescr

   {"bld_d_conquers",
    new BuiltinPropertyInfo([]() { return _("The conquer radius"); },
                            new Property("conquers",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::BuildingDescr),  // class
                                         VariableType(VariableTypeID::Integer)         // type
                                         ))},
   {"bld_d_vision_range",
    new BuiltinPropertyInfo([]() { return _("The building’s vision range"); },
                            new Property("vision_range",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::BuildingDescr),  // class
                                         VariableType(VariableTypeID::Integer)         // type
                                         ))},
   {"bld_d_workarea_radius", new BuiltinPropertyInfo(
                                []() {
	                                return _("The first workarea radius of the building), or nil in "
	                                         "case the building has no workarea");
	                             },
                                new Property("workarea_radius",
                                             Property::Access::RO,
                                             VariableType(VariableTypeID::BuildingDescr),  // class
                                             VariableType(VariableTypeID::Integer)         // type
                                             ))},
   {"bld_d_is_mine",
    new BuiltinPropertyInfo(
       []() { return _("Whether this building can be built only on mining plots"); },
       new Property("is_mine",
                    Property::Access::RO,
                    VariableType(VariableTypeID::BuildingDescr),  // class
                    VariableType(VariableTypeID::Boolean)         // type
                    ))},
   {"bld_d_is_port",
    new BuiltinPropertyInfo(
       []() { return _("Whether this building can be built only on port spaces"); },
       new Property("is_port",
                    Property::Access::RO,
                    VariableType(VariableTypeID::BuildingDescr),  // class
                    VariableType(VariableTypeID::Boolean)         // type
                    ))},
   {"bld_d_buildable",
    new BuiltinPropertyInfo(
       []() { return _("Whether the building can be built directly by the player"); },
       new Property("buildable",
                    Property::Access::RO,
                    VariableType(VariableTypeID::BuildingDescr),  // class
                    VariableType(VariableTypeID::Boolean)         // type
                    ))},
   {"bld_d_destructible",
    new BuiltinPropertyInfo(
       []() { return _("Whether the building can be burnt down by the player"); },
       new Property("destructible",
                    Property::Access::RO,
                    VariableType(VariableTypeID::BuildingDescr),  // class
                    VariableType(VariableTypeID::Boolean)         // type
                    ))},
   {"bld_d_enhanced_from", new BuiltinPropertyInfo(
                              []() {
	                              return _("The building type that can be enhanced to this "
	                                       "building), or nil if this is not an enhanced building");
	                           },
                              new Property("enhanced_from",
                                           Property::Access::RO,
                                           VariableType(VariableTypeID::BuildingDescr),  // class
                                           VariableType(VariableTypeID::BuildingDescr)   // type
                                           ))},
   {"bld_d_enhancement", new BuiltinPropertyInfo(
                            []() {
	                            return _("The building type this building can be enhanced to, or "
	                                     "nil if this building is not enhanceable");
	                         },
                            new Property("enhancement",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::BuildingDescr),  // class
                                         VariableType(VariableTypeID::BuildingDescr)   // type
                                         ))},
   {"bld_d_enhancement_cost",
    new BuiltinPropertyInfo(
       []() { return _("The cost for enhancing this building"); },
       new Property("enhancement_cost",
                    Property::Access::RO,
                    VariableType(VariableTypeID::BuildingDescr),  // class
                    VariableType(VariableType(VariableTypeID::String),
                                 VariableType(VariableTypeID::Integer))  // type
                    ))},
   {"bld_d_returned_wares",
    new BuiltinPropertyInfo(
       []() { return _("The list of wares returned upon dismantling"); },
       new Property("returned_wares",
                    Property::Access::RO,
                    VariableType(VariableTypeID::BuildingDescr),  // class
                    VariableType(VariableType(VariableTypeID::String),
                                 VariableType(VariableTypeID::Integer))  // type
                    ))},
   {"bld_d_returned_wares_enhanced",
    new BuiltinPropertyInfo(
       []() { return _("The list of wares returned upon dismantling an enhanced building"); },
       new Property("returned_wares_enhanced",
                    Property::Access::RO,
                    VariableType(VariableTypeID::BuildingDescr),  // class
                    VariableType(VariableType(VariableTypeID::String),
                                 VariableType(VariableTypeID::Integer))  // type
                    ))},

   // ProductionSiteDescr

   {"pd_d_inputs", new BuiltinPropertyInfo(
                      []() {
	                      return _("An array of WareDescriptions containing the wares this "
	                               "productionsite needs for its production");
	                   },
                      new Property(
                         "inputs",
                         Property::Access::RO,
                         VariableType(VariableTypeID::ProductionSiteDescr),  // class
                         VariableType(VariableType(VariableTypeID::Integer),
                                      VariableType(VariableTypeID::WareDescr))  // type
                         ))},
   {"pd_d_output_ware_types",
    new BuiltinPropertyInfo(
       []() {
	       return _(
	          "An array with WareDescriptions containing the wares this productionsite can produce");
	    },
       new Property("output_ware_types",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ProductionSiteDescr),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableTypeID::WareDescr))  // type
                    ))},
   {"pd_d_output_worker_types",
    new BuiltinPropertyInfo(
       []() {
	       return _("An array with WorkerDescriptions containing the workers this productionsite "
	                "can recruit");
	    },
       new Property("output_worker_types",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ProductionSiteDescr),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableTypeID::WorkerDescr))  // type
                    ))},
   {"pd_d_production_programs",
    new BuiltinPropertyInfo(
       []() { return _("An array with the production program names as string"); },
       new Property("production_programs",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ProductionSiteDescr),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableTypeID::String))  // type
                    ))},
   {"pd_d_working_positions",
    new BuiltinPropertyInfo(
       []() {
	       return _(
	          "An array with WorkerDescriptions containing the workers that need to work here. "
	          "If several instances of a certain worker type are required), this instance is "
	          "contained several times.");
	    },
       new Property("working_positions",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ProductionSiteDescr),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableTypeID::WorkerDescr))  // type
                    ))},

   // MilitarySiteDescr

   {"ms_d_heal",
    new BuiltinPropertyInfo(
       []() { return _("The number of health healed per second by the militarysite"); },
       new Property("heal_per_second",
                    Property::Access::RO,
                    VariableType(VariableTypeID::MilitarySiteDescr),  // class
                    VariableType(VariableTypeID::Integer)             // type
                    ))},
   {"ms_d_maxsoldiers",
    new BuiltinPropertyInfo(
       []() { return _("The number of soldiers that can be garrisoned at the militarysite"); },
       new Property("max_number_of_soldiers",
                    Property::Access::RO,
                    VariableType(VariableTypeID::MilitarySiteDescr),  // class
                    VariableType(VariableTypeID::Integer)             // type
                    ))},

   // WarehouseDescr

   {"wh_d_heal",
    new BuiltinPropertyInfo(
       []() { return _("The number of health healed per second for soldiers in the warehouse"); },
       new Property("heal_per_second",
                    Property::Access::RO,
                    VariableType(VariableTypeID::WarehouseDescr),  // class
                    VariableType(VariableTypeID::Integer)          // type
                    ))},

   // TrainingSiteDescr

   {"ts_d_min_a", new BuiltinPropertyInfo(
                     []() { return _("The minimum attack level soldier may start training with"); },
                     new Property("min_attack",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::TrainingSiteDescr),  // class
                                  VariableType(VariableTypeID::Integer)             // type
                                  ))},
   {"ts_d_min_h", new BuiltinPropertyInfo(
                     []() { return _("The minimum health level soldier may start training with"); },
                     new Property("min_health",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::TrainingSiteDescr),  // class
                                  VariableType(VariableTypeID::Integer)             // type
                                  ))},
   {"ts_d_min_d",
    new BuiltinPropertyInfo(
       []() { return _("The minimum defense level soldier may start training with"); },
       new Property("min_defense",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TrainingSiteDescr),  // class
                    VariableType(VariableTypeID::Integer)             // type
                    ))},
   {"ts_d_min_e", new BuiltinPropertyInfo(
                     []() { return _("The minimum evade level soldier may start training with"); },
                     new Property("min_evade",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::TrainingSiteDescr),  // class
                                  VariableType(VariableTypeID::Integer)             // type
                                  ))},
   {"ts_d_max_a", new BuiltinPropertyInfo(
                     []() { return _("The attack level up to which a soldier can train here"); },
                     new Property("max_attack",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::TrainingSiteDescr),  // class
                                  VariableType(VariableTypeID::Integer)             // type
                                  ))},
   {"ts_d_max_h", new BuiltinPropertyInfo(
                     []() { return _("The health level up to which a soldier can train here"); },
                     new Property("max_health",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::TrainingSiteDescr),  // class
                                  VariableType(VariableTypeID::Integer)             // type
                                  ))},
   {"ts_d_max_d", new BuiltinPropertyInfo(
                     []() { return _("The defense level up to which a soldier can train here"); },
                     new Property("max_defense",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::TrainingSiteDescr),  // class
                                  VariableType(VariableTypeID::Integer)             // type
                                  ))},
   {"ts_d_max_e", new BuiltinPropertyInfo(
                     []() { return _("The evade level up to which a soldier can train here"); },
                     new Property("max_evade",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::TrainingSiteDescr),  // class
                                  VariableType(VariableTypeID::Integer)             // type
                                  ))},

   // WorkerDescr

   {"wd_becomes", new BuiltinPropertyInfo(
                     []() {
	                     return _("The WorkerDescription of the worker this one will level up to or "
	                              "nil if it never levels up.");
	                  },
                     new Property("becomes",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::WorkerDescr),  // class
                                  VariableType(VariableTypeID::WorkerDescr)   // type
                                  ))},
   {"wd_needed_experience",
    new BuiltinPropertyInfo(
       []() { return _("The amount of experience points this worker needs to level up"); },
       new Property("needed_experience",
                    Property::Access::RO,
                    VariableType(VariableTypeID::WorkerDescr),  // class
                    VariableType(VariableTypeID::Integer)       // type
                    ))},
   {"wd_buildable",
    new BuiltinPropertyInfo([]() { return _("Whether this worker can be built in warehouses"); },
                            new Property("buildable",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::WorkerDescr),  // class
                                         VariableType(VariableTypeID::Boolean)       // type
                                         ))},
   {"wd_buildcost",
    new BuiltinPropertyInfo(
       []() {
	       return _("A table with the buildcost for this worker if it may be created in warehouses");
	    },
       new Property("buildcost",
                    Property::Access::RO,
                    VariableType(VariableTypeID::WorkerDescr),  // class
                    VariableType(VariableType(VariableTypeID::String),
                                 VariableType(VariableTypeID::Integer))  // type
                    ))},
   {"wd_employers",
    new BuiltinPropertyInfo(
       []() {
	       return _(
	          "An array with BuildingDescriptions with buildings where this worker can be employed");
	    },
       new Property("employers",
                    Property::Access::RO,
                    VariableType(VariableTypeID::WorkerDescr),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableTypeID::BuildingDescr))  // type
                    ))},

   // SoldierDescr

   {"sd_h_max",
    new BuiltinPropertyInfo([]() { return _("The maximum health level this soldier can have"); },
                            new Property("max_health_level",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::SoldierDescr),  // class
                                         VariableType(VariableTypeID::Integer)        // type
                                         ))},
   {"sd_a_max",
    new BuiltinPropertyInfo([]() { return _("The maximum attack level this soldier can have"); },
                            new Property("max_attack_level",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::SoldierDescr),  // class
                                         VariableType(VariableTypeID::Integer)        // type
                                         ))},
   {"sd_d_max",
    new BuiltinPropertyInfo([]() { return _("The maximum defense level this soldier can have"); },
                            new Property("max_defense_level",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::SoldierDescr),  // class
                                         VariableType(VariableTypeID::Integer)        // type
                                         ))},
   {"sd_e_max",
    new BuiltinPropertyInfo([]() { return _("The maximum evade level this soldier can have"); },
                            new Property("max_evade_level",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::SoldierDescr),  // class
                                         VariableType(VariableTypeID::Integer)        // type
                                         ))},
   {"sd_h_base", new BuiltinPropertyInfo(
                    []() { return _("The number of health points this soldier starts with"); },
                    new Property("base_health",
                                 Property::Access::RO,
                                 VariableType(VariableTypeID::SoldierDescr),  // class
                                 VariableType(VariableTypeID::Integer)        // type
                                 ))},
   {"sd_a_base",
    new BuiltinPropertyInfo([]() { return _("The attack strength this soldier starts with"); },
                            new Property("base_attack",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::SoldierDescr),  // class
                                         VariableType(VariableTypeID::Integer)        // type
                                         ))},
   {"sd_d_base", new BuiltinPropertyInfo(
                    []() { return _("The blow absorption percentage this soldier starts with"); },
                    new Property("base_defense",
                                 Property::Access::RO,
                                 VariableType(VariableTypeID::SoldierDescr),  // class
                                 VariableType(VariableTypeID::Integer)        // type
                                 ))},
   {"sd_e_base", new BuiltinPropertyInfo(
                    []() { return _("The evade chance in percent this soldier starts with"); },
                    new Property("base_evade",
                                 Property::Access::RO,
                                 VariableType(VariableTypeID::SoldierDescr),  // class
                                 VariableType(VariableTypeID::Integer)        // type
                                 ))},
   {"sd_h_incr",
    new BuiltinPropertyInfo([]() { return _("The number of hitpoints gained per health level"); },
                            new Property("health_incr_per_level",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::SoldierDescr),  // class
                                         VariableType(VariableTypeID::Integer)        // type
                                         ))},
   {"sd_a_incr",
    new BuiltinPropertyInfo([]() { return _("The attack strength gained per attack level"); },
                            new Property("attack_incr_per_level",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::SoldierDescr),  // class
                                         VariableType(VariableTypeID::Integer)        // type
                                         ))},
   {"sd_d_incr",
    new BuiltinPropertyInfo(
       []() {
	       return _("The blow absorption rate increase gained per defense level in percent points");
	    },
       new Property("defense_incr_per_level",
                    Property::Access::RO,
                    VariableType(VariableTypeID::SoldierDescr),  // class
                    VariableType(VariableTypeID::Integer)        // type
                    ))},
   {"sd_e_incr",
    new BuiltinPropertyInfo(
       []() { return _("The evade chance increase gained per evade level in percent points"); },
       new Property("evade_incr_per_level",
                    Property::Access::RO,
                    VariableType(VariableTypeID::SoldierDescr),  // class
                    VariableType(VariableTypeID::Integer)        // type
                    ))},

   // ResourceDescr

   {"rd_name",
    new BuiltinPropertyInfo([]() { return _("The resource’s internal name"); },
                            new Property("name",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::ResourceDescr),  // class
                                         VariableType(VariableTypeID::String)          // type
                                         ))},
   {"rd_descname",
    new BuiltinPropertyInfo([]() { return _("The resource’s localized name"); },
                            new Property("descname",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::ResourceDescr),  // class
                                         VariableType(VariableTypeID::String)          // type
                                         ))},
   {"rd_dect",
    new BuiltinPropertyInfo([]() { return _("Whether geologists can find this resource"); },
                            new Property("is_detectable",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::ResourceDescr),  // class
                                         VariableType(VariableTypeID::Boolean)         // type
                                         ))},
   {"rd_max_amount",
    new BuiltinPropertyInfo(
       []() { return _("The highest amount of this resource that can be contained on a field"); },
       new Property("max_amount",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ResourceDescr),  // class
                    VariableType(VariableTypeID::Integer)         // type
                    ))},
   {"rd_representative_image",
    new BuiltinPropertyInfo(
       []() { return _("The path to the image representing this resource in the GUI"); },
       new Property("representative_image",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ResourceDescr),  // class
                    VariableType(VariableTypeID::String)          // type
                    ))},

   // TerrainDescr

   {"terrd_name",
    new BuiltinPropertyInfo([]() { return _("The terrain’s internal name"); },
                            new Property("name",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::TerrainDescr),  // class
                                         VariableType(VariableTypeID::String)         // type
                                         ))},
   {"terrd_descname",
    new BuiltinPropertyInfo([]() { return _("The terrain’s localized name"); },
                            new Property("descname",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::TerrainDescr),  // class
                                         VariableType(VariableTypeID::String)         // type
                                         ))},
   {"terrd_default_resource", new BuiltinPropertyInfo(
                                 []() {
	                                 return _("The resource that can be found here unless another "
	                                          "resource is explicitly placed here. Can be nil.");
	                              },
                                 new Property("default_resource",
                                              Property::Access::RO,
                                              VariableType(VariableTypeID::TerrainDescr),  // class
                                              VariableType(VariableTypeID::ResourceDescr)  // type
                                              ))},
   {"terrd_default_resource_amount",
    new BuiltinPropertyInfo(
       []() { return _("The amount of the default resource provided by this terrain"); },
       new Property("default_resource_amount",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TerrainDescr),  // class
                    VariableType(VariableTypeID::Integer)        // type
                    ))},
   {"terrd_valid_resources",
    new BuiltinPropertyInfo(
       []() {
	       return _("An array of ResourceDescriptions with all valid resources for this terrain");
	    },
       new Property("valid_resources",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TerrainDescr),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableTypeID::ResourceDescr))  // type
                    ))},
   {"terrd_representative_image",
    new BuiltinPropertyInfo([]() { return _("The file path to the representative image"); },
                            new Property("representative_image",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::TerrainDescr),  // class
                                         VariableType(VariableTypeID::String)         // type
                                         ))},
   {"terrd_temperature",
    new BuiltinPropertyInfo(
       []() {
	       return _("The temperature of this terrain (regarding immovables’ terrain affinity)");
	    },
       new Property("temperature",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TerrainDescr),  // class
                    VariableType(VariableTypeID::Integer)        // type
                    ))},
   {"terrd_humidity",
    new BuiltinPropertyInfo(
       []() { return _("The humidity of this terrain (regarding immovables’ terrain affinity)"); },
       new Property("humidity",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TerrainDescr),  // class
                    VariableType(VariableTypeID::Integer)        // type
                    ))},
   {"terrd_fertility",
    new BuiltinPropertyInfo(
       []() { return _("The fertility of this terrain (regarding immovables’ terrain affinity)"); },
       new Property("fertility",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TerrainDescr),  // class
                    VariableType(VariableTypeID::Integer)        // type
                    ))},

   // MapObject

   {"mo_serial", new BuiltinPropertyInfo(
                    []() {
	                    return _("The unique serial number of this map object. "
	                             "Note that this value may not stay constant after saving/loading.");
	                 },
                    new Property(
                       "serial",
                       Property::Access::RO,
                       VariableType(VariableTypeID::MapObject),  // class
                       VariableType(VariableTypeID::Integer)     // type
                       ))},

// MapObject descrs

#define DESCR_FOR(type)                                                                            \
	{                                                                                               \
		"descr_" #type,                                                                              \
		   new BuiltinPropertyInfo(                                                                  \
		      []() { return _("The MapObjectDescr associated with this map object"); },              \
		      new Property("descr", Property::Access::RO, VariableType(VariableTypeID::type),        \
		                   VariableType(VariableTypeID::type##Descr), false))                        \
	}

   DESCR_FOR(MapObject),
   DESCR_FOR(Bob),
   DESCR_FOR(Ship),
   DESCR_FOR(Worker),
   DESCR_FOR(Carrier),
   DESCR_FOR(Ferry),
   DESCR_FOR(Soldier),
   DESCR_FOR(BaseImmovable),
   DESCR_FOR(PlayerImmovable),
   DESCR_FOR(Immovable),
   DESCR_FOR(Flag),
   DESCR_FOR(Road),
   DESCR_FOR(Building),
   DESCR_FOR(Warehouse),
   DESCR_FOR(ProductionSite),
   DESCR_FOR(TrainingSite),
   DESCR_FOR(Market),
   DESCR_FOR(MilitarySite),
   DESCR_FOR(ConstructionSite),
   DESCR_FOR(DismantleSite),

#undef DESCR_FOR

   // BaseImmovable

   {"baseimmo_fields",
    new BuiltinPropertyInfo(
       []() {
	       return _("An array of Field that is occupied by this Immovable. "
	                "If the immovable occupies more than one field (roads or big buildings for "
	                "example) the first entry in this list will be the main field");
	    },
       new Property(
          "fields",
          Property::Access::RO,
          VariableType(VariableTypeID::BaseImmovable),  // class
          VariableType(
             VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::Field))  // type
          ))},

   // PlayerImmovable

   {"plimmo_owner",
    new BuiltinPropertyInfo([]() { return _("The player owning this player immovable"); },
                            new Property("owner",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::PlayerImmovable),  // class
                                         VariableType(VariableTypeID::Player)            // type
                                         ))},

   // Flag

   {"flag_waeco",
    new BuiltinPropertyInfo(
       []() {
	       return _("The ware economy that this flag belongs to. Warning: Since economies "
	                "can disappear when a player merges them through placing/deleting "
	                "roads and flags, you must get a fresh economy object every time you call "
	                "another function on the resulting economy object.");
	    },
       new Property("ware_economy",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Flag),    // class
                    VariableType(VariableTypeID::Economy)  // type
                    ))},
   {"flag_woeco",
    new BuiltinPropertyInfo(
       []() {
	       return _("The worker economy that this flag belongs to. Warning: Since economies "
	                "can disappear when a player merges them through placing/deleting "
	                "roads and flags, you must get a fresh economy object every time you call "
	                "another function on the resulting economy object.");
	    },
       new Property("worker_economy",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Flag),    // class
                    VariableType(VariableTypeID::Economy)  // type
                    ))},
   {"flag_building",
    new BuiltinPropertyInfo([]() { return _("The building attached to this flag."); },
                            new Property("building",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Flag),     // class
                                         VariableType(VariableTypeID::Building)  // type
                                         ))},
   {"flag_roads", new BuiltinPropertyInfo(
                     []() {
	                     return _("The roads leading to the flag. Directions can be 'tr', 'r', "
	                              "'br', 'bl', 'l', and 'tl'.");
	                  },
                     new Property("roads",
                                  Property::Access::RO,
                                  VariableType(VariableTypeID::Flag),  // class
                                  VariableType(VariableType(VariableTypeID::String),
                                               VariableType(VariableTypeID::Road))  // type
                                  ))},

   // Road

   {"road_length",
    new BuiltinPropertyInfo([]() { return _("The number of edges this road covers."); },
                            new Property(
                               "length",
                               Property::Access::RO,
                               VariableType(VariableTypeID::Road),    // class
                               VariableType(VariableTypeID::Integer)  // type
                               ))},
   {"road_start_flag",
    new BuiltinPropertyInfo([]() { return _("The flag where this road starts"); },
                            new Property("start_flag",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Road),  // class
                                         VariableType(VariableTypeID::Flag)   // type
                                         ))},
   {"road_end_flag",
    new BuiltinPropertyInfo([]() { return _("The flag where this road ends"); },
                            new Property("end_flag",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Road),  // class
                                         VariableType(VariableTypeID::Flag)   // type
                                         ))},
   {"road_type", new BuiltinPropertyInfo([]() { return _("'normal', 'busy', or 'waterway'"); },
                                         new Property("road_type",
                                                      Property::Access::RO,
                                                      VariableType(VariableTypeID::Road),   // class
                                                      VariableType(VariableTypeID::String)  // type
                                                      ))},
   {"road_valid_workers",
    new BuiltinPropertyInfo(
       []() {
	       return _("A table of the workers that can be employed on this road), "
	                "in the format worker_name:number_of_working_positions");
	    },
       new Property("valid_workers",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Road),  // class
                    VariableType(VariableType(VariableTypeID::String),
                                 VariableType(VariableTypeID::Integer))  // type
                    ))},

   // Building

   {"bld_flag",
    new BuiltinPropertyInfo([]() { return _("The flag this building belongs to"); },
                            new Property("flag",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Building),  // class
                                         VariableType(VariableTypeID::Flag)       // type
                                         ))},

   // DismantleSite

   {"ds_has_builder",
    new BuiltinPropertyInfo([]() { return _("Whether this site has a builder"); },
                            new Property("has_builder",
                                         Property::Access::RW,
                                         VariableType(VariableTypeID::DismantleSite),  // class
                                         VariableType(VariableTypeID::Boolean)         // type
                                         ))},

   // ConstructionSite

   {"cs_has_builder",
    new BuiltinPropertyInfo([]() { return _("Whether this site has a builder"); },
                            new Property("has_builder",
                                         Property::Access::RW,
                                         VariableType(VariableTypeID::ConstructionSite),  // class
                                         VariableType(VariableTypeID::Boolean)            // type
                                         ))},
   {"cs_setting_soldier_capacity",
    new BuiltinPropertyInfo(
       []() {
	       return _(
	          "The number of soldiers stationed at this military- or trainingsite after completion");
	    },
       new Property("setting_soldier_capacity",
                    Property::Access::RW,
                    VariableType(VariableTypeID::ConstructionSite),  // class
                    VariableType(VariableTypeID::Integer)            // type
                    ))},
   {"cs_setting_prefer_heroes",
    new BuiltinPropertyInfo(
       []() { return _("Whether this militarysite initially prefers heroes after completion"); },
       new Property("setting_prefer_heroes",
                    Property::Access::RW,
                    VariableType(VariableTypeID::ConstructionSite),  // class
                    VariableType(VariableTypeID::Boolean)            // type
                    ))},
   {"cs_setting_stopped",
    new BuiltinPropertyInfo(
       []() { return _("Whether this productionsite will initially be stopped after completion"); },
       new Property("setting_stopped",
                    Property::Access::RW,
                    VariableType(VariableTypeID::ConstructionSite),  // class
                    VariableType(VariableTypeID::Boolean)            // type
                    ))},
   {"cs_setting_launch_expedition",
    new BuiltinPropertyInfo(
       []() { return _("Whether this port will launch an expedition right after completion"); },
       new Property("setting_launch_expedition",
                    Property::Access::RW,
                    VariableType(VariableTypeID::ConstructionSite),  // class
                    VariableType(VariableTypeID::Boolean)            // type
                    ))},

   // MilitarySite

   {"ms_capacity",
    new BuiltinPropertyInfo(
       []() { return _("The maximum number of soldiers that can be garrisoned in this building"); },
       new Property("max_soldiers",
                    Property::Access::RO,
                    VariableType(VariableTypeID::MilitarySite),  // class
                    VariableType(VariableTypeID::Integer)        // type
                    ))},
   {"ms_prefer_heroes",
    new BuiltinPropertyInfo([]() { return _("Whether this site prefers heroes"); },
                            new Property("prefer_heroes",
                                         Property::Access::RW,
                                         VariableType(VariableTypeID::MilitarySite),  // class
                                         VariableType(VariableTypeID::Boolean)        // type
                                         ))},

   // TrainingSite

   {"ts_capacity",
    new BuiltinPropertyInfo(
       []() { return _("The maximum number of soldiers that can train here at the same time"); },
       new Property("max_soldiers",
                    Property::Access::RO,
                    VariableType(VariableTypeID::TrainingSite),  // class
                    VariableType(VariableTypeID::Integer)        // type
                    ))},

   // ConstructionSite

   {"cs_building",
    new BuiltinPropertyInfo([]() { return _("The name of the building under construction"); },
                            new Property("building",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::ConstructionSite),  // class
                                         VariableType(VariableTypeID::String)             // type
                                         ))},

   // Warehouse

   {"wh_portdock",
    new BuiltinPropertyInfo(
       []() {
	       return _(
	          "The PortDock associated with this port), or nil if this warehouse is not a port.");
	    },
       new Property("portdock",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Warehouse),  // class
                    VariableType(VariableTypeID::PortDock)    // type
                    ))},
   {"wh_exp_in_p",
    new BuiltinPropertyInfo(
       []() { return _("Whether this is a port and an expedition is being prepared here"); },
       new Property("expedition_in_progress",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Warehouse),  // class
                    VariableType(VariableTypeID::Boolean)     // type
                    ))},

   // ProductionSite

   {"ps_stopped",
    new BuiltinPropertyInfo([]() { return _("Whether this productionsite is currently stopped"); },
                            new Property("is_stopped",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::ProductionSite),  // class
                                         VariableType(VariableTypeID::Boolean)          // type
                                         ))},
   {"ps_valid_workers",
    new BuiltinPropertyInfo(
       []() {
	       return _("A table of the workers that can be employed in this productionsite), "
	                "in the format worker_name:number_of_working_positions");
	    },
       new Property("valid_workers",
                    Property::Access::RO,
                    VariableType(VariableTypeID::ProductionSite),  // class
                    VariableType(VariableType(VariableTypeID::String),
                                 VariableType(VariableTypeID::Integer))  // type
                    ))},

   // Bob

   {"bob_field",
    new BuiltinPropertyInfo([]() { return _("The field this bob is currently located on"); },
                            new Property("field",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Bob),   // class
                                         VariableType(VariableTypeID::Field)  // type
                                         ))},

   // Ship

   {"ship_shipname",
    new BuiltinPropertyInfo([]() { return _("The ship’s name"); },
                            new Property("shipname",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Ship),   // class
                                         VariableType(VariableTypeID::String)  // type
                                         ))},
   {"ship_destination",
    new BuiltinPropertyInfo(
       []() { return _("The port dock this ship is heading to, or nil if it has no destination"); },
       new Property("destination",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Ship),     // class
                    VariableType(VariableTypeID::PortDock)  // type
                    ))},
   {"ship_capacity",
    new BuiltinPropertyInfo([]() { return _("The ship's capacity"); },
                            new Property("capacity",
                                         Property::Access::RW,
                                         VariableType(VariableTypeID::Ship),    // class
                                         VariableType(VariableTypeID::Integer)  // type
                                         ))},
   {"ship_last_portdock",
    new BuiltinPropertyInfo(
       []() {
	       return _("nil if no port was ever visited or the last portdock was destroyed; "
	                "otherwise the port dock of the last visited port");
	    },
       new Property("last_portdock",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Ship),     // class
                    VariableType(VariableTypeID::PortDock)  // type
                    ))},
   {"ship_state",
    new BuiltinPropertyInfo(
       []() {
	       return _("What this ship is currently doing: 'transport', 'exp_scouting', "
	                "'exp_found_port_space', 'exp_colonizing', 'sink_request', or 'sink_animation'");
	    },
       new Property("state",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Ship),   // class
                    VariableType(VariableTypeID::String)  // type
                    ))},
   {"ship_island_explore_direction",
    new BuiltinPropertyInfo(
       []() {
	       return _("The direction if the ship is an expedition ship sailing "
	                "around an island. Valid values are 'cw', 'ccw', and nil.");
	    },
       new Property("island_explore_direction",
                    Property::Access::RW,
                    VariableType(VariableTypeID::Ship),   // class
                    VariableType(VariableTypeID::String)  // type
                    ))},

   // Worker

   {"worker_owner",
    new BuiltinPropertyInfo([]() { return _("The player this worker belongs to"); },
                            new Property("owner",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Worker),  // class
                                         VariableType(VariableTypeID::Player)   // type
                                         ))},
   {"worker_location",
    new BuiltinPropertyInfo(
       []() {
	       return _("The location where this worker is situated. "
	                "This will be either a Building), Road), Flag or nil. "
	                "Note that a worker that is stored in a warehouse has a location nil. "
	                "A worker that is out working (e.g. hunter) has as a location his building. "
	                "A stationed soldier has his military building as location. "
	                "Workers on transit usually have the Road they are currently on as location.");
	    },
       new Property("location",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Worker),          // class
                    VariableType(VariableTypeID::PlayerImmovable)  // type
                    ))},

   // Soldier

   {"soldier_level_h",
    new BuiltinPropertyInfo([]() { return _("The soldier’s current health level"); },
                            new Property("health_level",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Soldier),  // class
                                         VariableType(VariableTypeID::Integer)   // type
                                         ))},
   {"soldier_level_a",
    new BuiltinPropertyInfo([]() { return _("The soldier’s current attack level"); },
                            new Property("attack_level",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Soldier),  // class
                                         VariableType(VariableTypeID::Integer)   // type
                                         ))},
   {"soldier_level_d",
    new BuiltinPropertyInfo([]() { return _("The soldier’s current defense level"); },
                            new Property("defense_level",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Soldier),  // class
                                         VariableType(VariableTypeID::Integer)   // type
                                         ))},
   {"soldier_level_e",
    new BuiltinPropertyInfo([]() { return _("The soldier’s current evade level"); },
                            new Property("evade_level",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Soldier),  // class
                                         VariableType(VariableTypeID::Integer)   // type
                                         ))},
   {"soldier_curhealth",
    new BuiltinPropertyInfo([]() { return _("The soldier’s current total health"); },
                            new Property("current_health",
                                         Property::Access::RW,
                                         VariableType(VariableTypeID::Soldier),  // class
                                         VariableType(VariableTypeID::Integer)   // type
                                         ))},

   // Field

   {"f_x", new BuiltinPropertyInfo([]() { return _("The x coordinate of this field"); },
                                   new Property("x",
                                                Property::Access::RO,
                                                VariableType(VariableTypeID::Field),   // class
                                                VariableType(VariableTypeID::Integer)  // type
                                                ))},
   {"f_y", new BuiltinPropertyInfo([]() { return _("The y coordinate of this field"); },
                                   new Property("y",
                                                Property::Access::RO,
                                                VariableType(VariableTypeID::Field),   // class
                                                VariableType(VariableTypeID::Integer)  // type
                                                ))},
   {"f_height",
    new BuiltinPropertyInfo(
       []() {
	       return _("The height of this field. Note though that if you change this value "
	                "too much, all surrounding fields will also change their heights because "
	                "the slope is constrained. If you are changing the height of many terrains "
	                "at once), use raw_height instead and then call recalculate() afterwards.");
	    },
       new Property(
          "height",
          Property::Access::RW,
          VariableType(VariableTypeID::Field),   // class
          VariableType(VariableTypeID::Integer)  // type
          ))},
   {"f_raw_height",
    new BuiltinPropertyInfo(
       []() {
	       return _("The same as 'height', but setting this will not trigger a recalculation "
	                "of the surrounding fields. You can use this field to change the height "
	                "of many fields on a map quickly), then use recalculate() to make sure that "
	                "everything is in order.");
	    },
       new Property("raw_height",
                    Property::Access::RW,
                    VariableType(VariableTypeID::Field),   // class
                    VariableType(VariableTypeID::Integer)  // type
                    ))},
   {"f_viewpoint_x", new BuiltinPropertyInfo(
                        []() {
	                        return _("Returns the position in pixels to move the view to to center "
	                                 "this field for the current interactive player");
	                     },
                        new Property("viewpoint_x",
                                     Property::Access::RO,
                                     VariableType(VariableTypeID::Field),   // class
                                     VariableType(VariableTypeID::Integer)  // type
                                     ))},
   {"f_viewpoint_y", new BuiltinPropertyInfo(
                        []() {
	                        return _("Returns the position in pixels to move the view to to center "
	                                 "this field for the current interactive player");
	                     },
                        new Property("viewpoint_y",
                                     Property::Access::RO,
                                     VariableType(VariableTypeID::Field),   // class
                                     VariableType(VariableTypeID::Integer)  // type
                                     ))},
   {"f_resource", new BuiltinPropertyInfo(
                     []() { return _("The name of the resource on this field. May be nil."); },
                     new Property("resource",
                                  Property::Access::RW,
                                  VariableType(VariableTypeID::Field),  // class
                                  VariableType(VariableTypeID::String)  // type
                                  ))},
   {"f_resource_amount",
    new BuiltinPropertyInfo([]() { return _("The resource amount left on this field."); },
                            new Property("resource_amount",
                                         Property::Access::RW,
                                         VariableType(VariableTypeID::Field),   // class
                                         VariableType(VariableTypeID::Integer)  // type
                                         ))},
   {"f_initial_resource_amount",
    new BuiltinPropertyInfo(
       []() { return _("The resource amount originally present on this field."); },
       new Property("initial_resource_amount",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Field),   // class
                    VariableType(VariableTypeID::Integer)  // type
                    ))},
   {"f_immovable",
    new BuiltinPropertyInfo([]() { return _("The immovable occupying this field. May be nil."); },
                            new Property("immovable",
                                         Property::Access::RO,
                                         VariableType(VariableTypeID::Field),         // class
                                         VariableType(VariableTypeID::BaseImmovable)  // type
                                         ))},
   {"f_bobs", new BuiltinPropertyInfo(
                 []() { return _("An array with all bobs currently located on this field"); },
                 new Property("bobs",
                              Property::Access::RO,
                              VariableType(VariableTypeID::Field),  // class
                              VariableType(VariableType(VariableTypeID::Integer),
                                           VariableType(VariableTypeID::Bob))  // type
                              ))},
   {"f_terd",
    new BuiltinPropertyInfo(
       []() { return _("The name of the terrain on the triangle straight south of this field "); },
       new Property("terd",
                    Property::Access::RW,
                    VariableType(VariableTypeID::Field),  // class
                    VariableType(VariableTypeID::String)  // type
                    ))},
   {"f_terr",
    new BuiltinPropertyInfo(
       []() { return _("The name of the terrain on the triangle southeast of this field "); },
       new Property("terr",
                    Property::Access::RW,
                    VariableType(VariableTypeID::Field),  // class
                    VariableType(VariableTypeID::String)  // type
                    ))},
   {"f_owner", new BuiltinPropertyInfo([]() { return _("The player owning this field"); },
                                       new Property("owner",
                                                    Property::Access::RO,
                                                    VariableType(VariableTypeID::Field),  // class
                                                    VariableType(VariableTypeID::Player)  // type
                                                    ))},
   {"f_claimers",
    new BuiltinPropertyInfo(
       []() {
	       return _(
	          "An array of players that have military influence over this field "
	          "sorted by the amount of influence they have. Note that this does "
	          "not necessarily mean that claimers[1] is also the owner of the "
	          "field), as a field that houses a surrounded military building is "
	          "owned by the surrounded Player), but others have more military influence over it.");
	    },
       new Property("claimers",
                    Property::Access::RO,
                    VariableType(VariableTypeID::Field),  // class
                    VariableType(VariableType(VariableTypeID::Integer),
                                 VariableType(VariableTypeID::Player))  // type
                    ))},
   {"f_bln", new BuiltinPropertyInfo([]() { return _("The southwestern neighbour of this field"); },
                                     new Property("bln",
                                                  Property::Access::RO,
                                                  VariableType(VariableTypeID::Field),  // class
                                                  VariableType(VariableTypeID::Field)   // type
                                                  ))},
   {"f_ln", new BuiltinPropertyInfo([]() { return _("The western neighbour of this field"); },
                                    new Property("ln",
                                                 Property::Access::RO,
                                                 VariableType(VariableTypeID::Field),  // class
                                                 VariableType(VariableTypeID::Field)   // type
                                                 ))},
   {"f_tln", new BuiltinPropertyInfo([]() { return _("The northwestern neighbour of this field"); },
                                     new Property("tln",
                                                  Property::Access::RO,
                                                  VariableType(VariableTypeID::Field),  // class
                                                  VariableType(VariableTypeID::Field)   // type
                                                  ))},
   {"f_brn", new BuiltinPropertyInfo([]() { return _("The southeastern neighbour of this field"); },
                                     new Property("brn",
                                                  Property::Access::RO,
                                                  VariableType(VariableTypeID::Field),  // class
                                                  VariableType(VariableTypeID::Field)   // type
                                                  ))},
   {"f_rn", new BuiltinPropertyInfo([]() { return _("The eastern neighbour of this field"); },
                                    new Property("rn",
                                                 Property::Access::RO,
                                                 VariableType(VariableTypeID::Field),  // class
                                                 VariableType(VariableTypeID::Field)   // type
                                                 ))},
   {"f_trn", new BuiltinPropertyInfo([]() { return _("The northeastern neighbour of this field"); },
                                     new Property("trn",
                                                  Property::Access::RO,
                                                  VariableType(VariableTypeID::Field),  // class
                                                  VariableType(VariableTypeID::Field)   // type
                                                  ))}};
