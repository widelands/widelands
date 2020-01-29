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
	for (size_t i = 0; kBuiltinFunctions[i]; ++i) {
		if (kBuiltinFunctions[i]->unique_name == name) {
			return *kBuiltinFunctions[i];
		}
	}
	throw wexception("Unknown builtin function %s", name.c_str());
}

const BuiltinFunctionInfo* builtin_f(const FunctionBase& f) {
	for (size_t i = 0; kBuiltinFunctions[i]; ++i) {
		if (kBuiltinFunctions[i]->function.get() == &f) {
			return kBuiltinFunctions[i];
		}
	}
	return nullptr;
}

const BuiltinPropertyInfo& builtin_p(const std::string& name) {
	for (size_t i = 0; kBuiltinProperties[i]; ++i) {
		if (kBuiltinProperties[i]->unique_name == name) {
			return *kBuiltinProperties[i];
		}
	}
	throw wexception("Unknown builtin property %s", name.c_str());
}

/************************************************************
                     Builtin functions
************************************************************/

// Do not change the order! Indices are stored in map files!
// The _() function is not contained here – access it instead via
// `ConstexprString`'s `translatable` attribute.
const BuiltinFunctionInfo* kBuiltinFunctions[] = {

   // Real Lua builtins

   new BuiltinFunctionInfo(
      "print",
      []() { return _("Prints debug information to the stdandard output."); },
      new FunctionBase("print",
                       VariableType(VariableTypeID::Nil), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("text", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "random_1",
      /** TRANSLATORS: max is the name of a function parameter */
      []() { return _("Returns a random value between 1 and max."); },
      new FunctionBase("math.random",
                       VariableType(VariableTypeID::Nil), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("max", VariableType(VariableTypeID::Integer))})),
   new BuiltinFunctionInfo(
      "random_2",
      /** TRANSLATORS: min and max are names of function parameters */
      []() { return _("Returns a random value between min and max."); },
      new FunctionBase("math.random",
                       VariableType(VariableTypeID::Nil), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("min", VariableType(VariableTypeID::Integer)),
                        std::make_pair("max", VariableType(VariableTypeID::Integer))})),

	// Shipped Lua functions

   new BuiltinFunctionInfo(
      "sleep",
      []() { return _("Pauses the current thread for the given number of milliseconds."); },
      new FunctionBase("sleep",
                       VariableType(VariableTypeID::Nil), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("milliseconds", VariableType(VariableTypeID::Integer))}),
      "scripting/coroutine.lua"),
   new BuiltinFunctionInfo(
      "wake_me",
      []() { return _("Pauses the current thread and resumes it at the given gametime (in milliseconds)."); },
      new FunctionBase("wake_me",
                       VariableType(VariableTypeID::Nil), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("time", VariableType(VariableTypeID::Integer))}),
      "scripting/coroutine.lua"),
   // TODO(Nordfriese): This function takes varargs
   new BuiltinFunctionInfo("array_combine", []() { return
		_("Concatenates the given arrays into a single array."); },
		new FunctionBase("array_combine", VariableType(VariableTypeID::Nil), // call on
   		VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::Any)), // returns
                       {std::make_pair("array1", VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Any))),
                       std::make_pair("array2", VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Any)))}),
      "scripting/table.lua"),

   // Game

   new BuiltinFunctionInfo(
      "game",
      []() { return _("Returns the running game instance."); },
      new FunctionBase("wl.Game",
                       VariableType(VariableTypeID::Nil), // call on
                       VariableType(VariableTypeID::Game), // returns
                       {})),
   new BuiltinFunctionInfo(
      "save",
      []() { return _("Saves the game under the given name."); },
      new FunctionBase("save",
                       VariableType(VariableTypeID::Game), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "get_tribe_descr",
      []() { return _("Returns the description for the tribe with the given name."); },
      new FunctionBase("get_tribe_description",
                       VariableType(VariableTypeID::Game), // call on
                       VariableType(VariableTypeID::TribeDescr), // returns
                       {std::make_pair("name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "get_terrain_descr",
      []() { return _("Returns the description for the terrain with the given name."); },
      new FunctionBase("get_terrain_description",
                       VariableType(VariableTypeID::Game), // call on
                       VariableType(VariableTypeID::TerrainDescr), // returns
                       {std::make_pair("name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "get_resource_descr",
      []() { return _("Returns the description for the resource with the given name."); },
      new FunctionBase("get_resource_description",
                       VariableType(VariableTypeID::Game), // call on
                       VariableType(VariableTypeID::ResourceDescr), // returns
                       {std::make_pair("name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "get_immovable_descr",
      []() { return _("Returns the description for the immovable with the given name."); },
      new FunctionBase("get_immovable_description",
                       VariableType(VariableTypeID::Game), // call on
                       VariableType(VariableTypeID::ImmovableDescr), // returns
                       {std::make_pair("name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "get_building_descr",
      []() { return _("Returns the description for the building with the given name."); },
      new FunctionBase("get_building_description",
                       VariableType(VariableTypeID::Game), // call on
                       VariableType(VariableTypeID::BuildingDescr), // returns
                       {std::make_pair("name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "get_worker_descr",
      []() { return _("Returns the description for the worker with the given name."); },
      new FunctionBase("get_worker_description",
                       VariableType(VariableTypeID::Game), // call on
                       VariableType(VariableTypeID::WorkerDescr), // returns
                       {std::make_pair("name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "get_ware_descr",
      []() { return _("Returns the description for the ware with the given name."); },
      new FunctionBase("get_ware_description",
                       VariableType(VariableTypeID::Game), // call on
                       VariableType(VariableTypeID::WareDescr), // returns
                       {std::make_pair("name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "save_campaign_data",
      []() { return _("Saves information that can be read by others scenarios."); },
      new FunctionBase("save_campaign_data",
                       VariableType(VariableTypeID::Game), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("campaign_name", VariableType(VariableTypeID::String)),
                       std::make_pair("scenario_name", VariableType(VariableTypeID::String)),
                       std::make_pair("data", VariableType(VariableType(VariableTypeID::String),
                       VariableType(VariableTypeID::Any)))})),
   new BuiltinFunctionInfo(
      "read_campaign_data",
      []() { return _("Read campaign data saved by another scenario."); },
      new FunctionBase("read_campaign_data",
                       VariableType(VariableTypeID::Game), // call on
                       VariableType(VariableType(VariableTypeID::Any),
                       VariableType(VariableTypeID::Any)), // returns
                       {std::make_pair("campaign_name", VariableType(VariableTypeID::String)),
                       std::make_pair("scenario_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "report_result",
      []() { return _("In an internet game), reports the ending of the game for a certain player to "
                      "the metaserver. Valid result codes are 0 (lost), 1 (won), and 2 (resigned)."); },
      new FunctionBase("wl.game.report_result",
                       VariableType(VariableTypeID::Nil), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("player", VariableType(VariableTypeID::Player)),
                       std::make_pair("result", VariableType(VariableTypeID::Integer)),
                       std::make_pair("additional_info", VariableType(VariableTypeID::String))})),

   // Player

   new BuiltinFunctionInfo(
      "place_flag",
      []() { return _("Place a flag on the given field."); },
      new FunctionBase("place_flag",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Flag), // returns
                       {std::make_pair("field", VariableType(VariableTypeID::Field)),
                       std::make_pair("force", VariableType(VariableTypeID::Boolean))})),
   new BuiltinFunctionInfo(
      "place_flag_no_force",
      []() { return _("Place a flag on the given field."); },
      new FunctionBase("place_flag",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Flag), // returns
                       {std::make_pair("field", VariableType(VariableTypeID::Field))})),
   new BuiltinFunctionInfo(
      "place_building",
      []() { return _("Place a building on the given field."); },
      new FunctionBase("place_building",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Building), // returns
                       {std::make_pair("internal_building_name", VariableType(VariableTypeID::String)),
                       std::make_pair("field", VariableType(VariableTypeID::Field)),
                       std::make_pair("constructionsite", VariableType(VariableTypeID::Boolean)),
                       std::make_pair("force", VariableType(VariableTypeID::Boolean))})),
   new BuiltinFunctionInfo(
      "place_building_no_force",
      []() { return _("Place a building on the given field."); },
      new FunctionBase("place_building",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Building), // returns
                       {std::make_pair("internal_building_name", VariableType(VariableTypeID::String)),
                       std::make_pair("field", VariableType(VariableTypeID::Field))})),
   new BuiltinFunctionInfo(
      "place_ship",
      []() { return _("Place a ship on the given field."); },
      new FunctionBase("place_ship",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Ship), // returns
                       {std::make_pair("field", VariableType(VariableTypeID::Field))})),
   new BuiltinFunctionInfo(
      "conquer",
      []() { return _("Conquer a region for this player."); },
      new FunctionBase("conquer",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("field", VariableType(VariableTypeID::Field)),
                       std::make_pair("radius", VariableType(VariableTypeID::Integer))})),
   new BuiltinFunctionInfo(
      "player_get_workers",
      []() { return _("Returns the number of workers of a certain type in the player’s stock."); },
      new FunctionBase("get_workers",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("internal_worker_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "player_get_wares",
      []() { return _("Returns the number of wares of a certain type in the player’s stock."); },
      new FunctionBase("get_wares",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("internal_ware_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "send_msg",
      []() { return _("Sends the player a message. The args table may contain the following keys:\n"
                      " · 'field' – connect a field with this message\n"
                      " · 'status' – 'new' (default), 'read', or 'archived'\n"
                      " · 'popup' – Whether to immediately open the inbox on message reception (true or false)\n"
                      " · 'icon' – path to the icon to show instead of the default message icon\n"
                      " · 'heading' – Message header to show instead of 'title'\n"
                      "All keys in args are optional. args may also be nil."); },
      new FunctionBase("send_message",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Message), // returns
                       {std::make_pair("title", VariableType(VariableTypeID::String)),
                       std::make_pair("text", VariableType(VariableTypeID::String)),
                       std::make_pair("args", VariableType(VariableType(VariableTypeID::String),
                       VariableType(VariableTypeID::Any)))})),
   new BuiltinFunctionInfo(
      "msgbox",
      []() { return _("Shows a story message box. The args table may contain the following keys:\n"
                      " · 'field' – center the view on this field\n"
                      " · 'w' – message box width in pixels (default: 400)\n"
                      " · 'h' – message box height in pixels (default: 300)\n"
                      " · 'posx' – horizontal message box position in pixels (default: centered)\n"
                      " · 'posy' – vertical message box position in pixels (default: centered)\n"
                      "All keys in args are optional. args may also be nil."); },
      new FunctionBase("message_box",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("title", VariableType(VariableTypeID::String)),
                       std::make_pair("text", VariableType(VariableTypeID::String)),
                       std::make_pair("args", VariableType(VariableType(VariableTypeID::String),
                       VariableType(VariableTypeID::Any)))})),
   new BuiltinFunctionInfo(
      "player_sees",
      []() { return _("Whether the player can currently see the given field."); },
      new FunctionBase("sees_field",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("field", VariableType(VariableTypeID::Field))})),
   new BuiltinFunctionInfo(
      "player_seen",
      []() { return _("Whether the player can currently see or has ever seen the given field."); },
      new FunctionBase("seen_field",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("field", VariableType(VariableTypeID::Field))})),
   new BuiltinFunctionInfo(
      "player_reveal",
      []() { return _("Reveal the specified fields to the player until they are explicitly hidden again."); },
      new FunctionBase("reveal_fields",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("fields", VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Field)))})),
   new BuiltinFunctionInfo(
      "player_hide",
      []() { return _("Undo the effects of reveal_fields()."); },
      new FunctionBase("hide_fields",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("fields", VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Field)))})),
   new BuiltinFunctionInfo(
      "allow_bld",
      []() { return _("Allow the player to build the specified buildings."); },
      new FunctionBase("allow_buildings",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("buildings", VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Building)))})),
   new BuiltinFunctionInfo(
      "forbid_bld",
      []() { return _("Forbid the player to build the specified buildings."); },
      new FunctionBase("forbid_buildings",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("buildings", VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Building)))})),
   new BuiltinFunctionInfo(
      "scenario_solved",
      []() { return _("Marks the current scenario as solved."); },
      new FunctionBase("mark_scenario_as_solved",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("scenario_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "player_ships",
      []() { return _("Returns an array of all the player’s ships."); },
      new FunctionBase("get_ships",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Ship)), // returns
                       {})),
   new BuiltinFunctionInfo(
      "player_buildings",
      []() { return _("Returns an array of all the player’s buildings of the specified type."); },
      new FunctionBase("get_buildings",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Building)), // returns
                       {std::make_pair("internal_building_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "player_suitability",
      []() { return _("Checks whether the player may build the specified building type on the given field."); },
      new FunctionBase("get_suitability",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("internal_building_name", VariableType(VariableTypeID::String)),
                       std::make_pair("field", VariableType(VariableTypeID::Field))})),
   new BuiltinFunctionInfo(
      "switchplayer",
      []() { return _("Switch to the player with the given player number."); },
      new FunctionBase("switchplayer",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("player_number", VariableType(VariableTypeID::Integer))})),
   new BuiltinFunctionInfo(
      "player_produced",
      []() { return _("Returns the number of units the player produced of a given ware "
                      "since the game was started."); },
      new FunctionBase("switchplayer",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("internal_ware_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "is_attack_forbidden",
      []() { return _("Returns whether the player is currently forbidden to attack the player "
                      "with the given player number."); },
      new FunctionBase("is_attack_forbidden",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("player_number", VariableType(VariableTypeID::Integer))})),
   new BuiltinFunctionInfo(
      "set_attack_forbidden",
      []() { return _("Sets whether the player is forbidden to attack the player with the given player number."); },
      new FunctionBase("set_attack_forbidden",
                       VariableType(VariableTypeID::Player), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("player_number", VariableType(VariableTypeID::Integer)),
                       std::make_pair("forbid", VariableType(VariableTypeID::Boolean))})),

   // Map

   new BuiltinFunctionInfo(
      "field",
      []() { return _("Returns the field with the given coordinates."); },
      new FunctionBase("get_field",
                       VariableType(VariableTypeID::Map), // call on
                       VariableType(VariableTypeID::Field), // returns
                       {std::make_pair("x", VariableType(VariableTypeID::Integer)),
                        std::make_pair("y", VariableType(VariableTypeID::Integer))})),
   new BuiltinFunctionInfo(
      "place_immovable",
      []() { return _("Places the given 'world' or 'tribe' immovable on the given field."); },
      new FunctionBase("place_immovable",
                       VariableType(VariableTypeID::Map), // call on
                       VariableType(VariableTypeID::Immovable), // returns
                       {std::make_pair("internal_immovable_name", VariableType(VariableTypeID::String)),
                        std::make_pair("field", VariableType(VariableTypeID::Field)),
                        std::make_pair("world_or_tribe", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "map_recalc",
      []() { return _("Needs to be called after changing the raw_height of a field to update "
                      "slope steepness, buildcaps etc."); },
      new FunctionBase("recalculate",
                       VariableType(VariableTypeID::Map), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {})),
   new BuiltinFunctionInfo(
      "map_recalc_sf",
      []() { return _("Check again whether the maps allows seafaring. Needs to be called only "
                      "after changing watery terrains."); },
      new FunctionBase("recalculate_seafaring",
                       VariableType(VariableTypeID::Map), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {})),
   new BuiltinFunctionInfo(
      "map_setport",
      []() { return _("Sets whether a port may be built at the given coordinates. "
                      "Returns false if this could not be set."); },
      new FunctionBase("set_port_space",
                       VariableType(VariableTypeID::Map), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("x", VariableType(VariableTypeID::Integer)),
                       std::make_pair("y", VariableType(VariableTypeID::Integer)),
                       std::make_pair("allow_port_space", VariableType(VariableTypeID::Boolean))})),

   // Field

   new BuiltinFunctionInfo(
      "field_region",
      []() { return _("Returns an array with all fields within a certain radius of this field."); },
      new FunctionBase("region",
                       VariableType(VariableTypeID::Field), // call on
                       VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Field)), // returns
                       {std::make_pair("radius", VariableType(VariableTypeID::Integer))})),
   new BuiltinFunctionInfo(
      "field_region_hollow",
      []() { return _("Returns an array with all fields inside a certain hollow area around this field."); },
      new FunctionBase("region",
                       VariableType(VariableTypeID::Field), // call on
                       VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Field)), // returns
                       {std::make_pair("outer_radius", VariableType(VariableTypeID::Integer)),
                       std::make_pair("inner_radius", VariableType(VariableTypeID::Integer))})),
   new BuiltinFunctionInfo(
      "field_caps",
      []() { return _("Returns whether this field has the given caps. Valid caps are:\n"
                      " · 'small' (suited for small buildings)\n"
                      " · 'medium' (suited for  buildings)\n"
                      " · 'big' (suited for big buildings)\n"
                      " · 'port' (suited for ports)\n"
                      " · 'mine' (suited for mines)\n"
                      " · 'flag' (suited for a flag)\n"
                      " · 'walkable' (bobs can walk here)\n"
                      " · 'swimmable' (bobs can swim here)"); },
      new FunctionBase("has_caps",
                       VariableType(VariableTypeID::Field), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("caps", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "field_maxcaps",
      []() { return _("Returns whether this field has the given caps when "
                      "not taking nearby immovables into account."); },
      new FunctionBase("has_max_caps",
                       VariableType(VariableTypeID::Field), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("caps", VariableType(VariableTypeID::String))})),

   // TribeDescr

   new BuiltinFunctionInfo(
      "tribe_has_bld",
      []() { return _("Whether this tribe uses a building with the given name."); },
      new FunctionBase("has_building",
                       VariableType(VariableTypeID::TribeDescr), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("internal_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "tribe_has_ware",
      []() { return _("Whether this tribe uses a ware with the given name."); },
      new FunctionBase("has_ware",
                       VariableType(VariableTypeID::TribeDescr), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("internal_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "tribe_has_worker",
      []() { return _("Whether this tribe uses a worker with the given name."); },
      new FunctionBase("has_worker",
                       VariableType(VariableTypeID::TribeDescr), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("internal_name", VariableType(VariableTypeID::String))})),

   // ImmovableDescr

   new BuiltinFunctionInfo(
      "immo_attr",
      []() { return _("Whether this immovable has the given attribute."); },
      new FunctionBase("has_attribute",
                       VariableType(VariableTypeID::ImmovableDescr), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("attribute", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "immo_growchance",
      []() { return _("Calculates the chance of this immovable to grow on the given terrain."); },
      new FunctionBase("probability_to_grow",
                       VariableType(VariableTypeID::ImmovableDescr), // call on
                       VariableType(VariableTypeID::Double), // returns
                       {std::make_pair("terrain", VariableType(VariableTypeID::TerrainDescr))})),

   // ProductionSiteDescr

   new BuiltinFunctionInfo(
      "prodsite_prodwares",
      []() { return _("Returns which wares are produced by a certain production program of this "
                      "productionsite as a table of {ware_name = ware_amount} pairs."); },
      new FunctionBase("produced_wares",
                       VariableType(VariableTypeID::ProductionSiteDescr), // call on
                       VariableType(VariableType(VariableTypeID::String),
                       VariableType(VariableTypeID::Integer)), // returns
                       {std::make_pair("production_program_internal_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "prodsite_recruits",
      []() { return _("Returns which workers are recruited by a certain production program of this "
                      "productionsite as a table of {worker_name = worker_amount} pairs."); },
      new FunctionBase("recruited_workers",
                       VariableType(VariableTypeID::ProductionSiteDescr), // call on
                       VariableType(VariableType(VariableTypeID::String),
                       VariableType(VariableTypeID::Integer)), // returns
                       {std::make_pair("production_program_internal_name", VariableType(VariableTypeID::String))})),

   // WareDescr

   new BuiltinFunctionInfo(
      "waredescr_consumers",
      []() { return _("Returns an array with the BuildingDescriptions of all buildings "
                      "of the given tribe that require this ware for production."); },
      new FunctionBase("consumers",
                       VariableType(VariableTypeID::WareDescr), // call on
                       VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::BuildingDescr)), // returns
                       {std::make_pair("tribe_internal_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "waredescr_producers",
      []() { return _("Returns an array with the BuildingDescriptions of all buildings "
                      "of the given tribe that produce this ware."); },
      new FunctionBase("producers",
                       VariableType(VariableTypeID::WareDescr), // call on
                       VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::BuildingDescr)), // returns
                       {std::make_pair("tribe_internal_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "waredescr_isconstructmat",
      []() { return _("Returns whether the given tribe needs this ware for any constructionsite."); },
      new FunctionBase("is_construction_material",
                       VariableType(VariableTypeID::WareDescr), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("tribe_internal_name", VariableType(VariableTypeID::String))})),

   // Economy

   new BuiltinFunctionInfo(
      "eco_target_get",
      []() { return _("Returns the target quantity for the given ware/worker in this economy."); },
      new FunctionBase("target_quantity",
                       VariableType(VariableTypeID::Economy), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("internal_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "eco_target_set",
      []() { return _("Sets the target quantity for the given ware/worker in this economy."); },
      new FunctionBase("set_target_quantity",
                       VariableType(VariableTypeID::Economy), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("internal_name", VariableType(VariableTypeID::String)),
                       std::make_pair("amount", VariableType(VariableTypeID::Integer))})),

   // MapObject

   new BuiltinFunctionInfo(
      "mo_destroy",
      []() { return _("Destroy this map object immediately. May have special effects, "
                       "e.g. a building will go up in flames."); },
      new FunctionBase("destroy",
                       VariableType(VariableTypeID::MapObject), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {})),
   new BuiltinFunctionInfo(
      "mo_remove",
      []() { return _("Remove this map object immediately and without any special effects."); },
      new FunctionBase("remove",
                       VariableType(VariableTypeID::MapObject), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {})),

	// Flag

   new BuiltinFunctionInfo(
      "flag_get_wares",
      []() { return _("Returns the number of wares of a certain type currently waiting on this flag."); },
      new FunctionBase("get_wares",
                       VariableType(VariableTypeID::Flag), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("ware_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "flag_set_wares",
      []() { return _("Sets the number of wares of a certain type currently waiting in this flag."); },
      new FunctionBase("set_wares",
                       VariableType(VariableTypeID::Flag), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("ware_name", VariableType(VariableTypeID::String)),
                       std::make_pair("amount", VariableType(VariableTypeID::Integer))})),
   new BuiltinFunctionInfo(
      "wh_get_workers",
      []() { return _("Returns the number of workers of a certain type currently stored in this warehouse."); },
      new FunctionBase("get_workers",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("worker_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "wh_set_workers",
      []() { return _("Sets the number of workers of a certain type currently stored in this warehouse."); },
      new FunctionBase("set_workers",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("worker_name", VariableType(VariableTypeID::String)),
                       std::make_pair("amount", VariableType(VariableTypeID::Integer))})),

	// Road

   new BuiltinFunctionInfo(
      "road_get_workers",
      []() { return _("Returns the number of workers of a certain type currently employed on this road."); },
      new FunctionBase("get_workers",
                       VariableType(VariableTypeID::Road), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("worker_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "road_set_workers",
      []() { return _("Sets the number of workers of a certain type currently employed on this road."); },
      new FunctionBase("set_workers",
                       VariableType(VariableTypeID::Road), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("worker_name", VariableType(VariableTypeID::String)),
                       std::make_pair("amount", VariableType(VariableTypeID::Integer))})),

   // ProductionSite

   new BuiltinFunctionInfo(
      "togglestartstop",
      []() { return _("If this productionsite is stopped), start it; otherwise stop it."); },
      new FunctionBase("toggle_start_stop",
                       VariableType(VariableTypeID::ProductionSite), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {})),
   new BuiltinFunctionInfo(
      "ps_get_workers",
      []() { return _("Returns the number of workers of a certain type currently employed "
      "in this productionsite."); },
      new FunctionBase("get_workers",
                       VariableType(VariableTypeID::ProductionSite), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("worker_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "ps_set_workers",
      []() { return _("Sets the number of workers of a certain type currently "
      "employed in this productionsite."); },
      new FunctionBase("set_workers",
                       VariableType(VariableTypeID::ProductionSite), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("worker_name", VariableType(VariableTypeID::String)),
                       std::make_pair("amount", VariableType(VariableTypeID::Integer))})),

	// MilitarySite

   new BuiltinFunctionInfo(
      "ms_get_soldiers",
      []() { return _("Returns the number of soldiers matching the given soldier description "
      "currently garrisoned here. A soldier description is an array that contains "
      "the levels for health, attack, defense and evade (in this order)."); },
      new FunctionBase("get_soldiers",
                       VariableType(VariableTypeID::MilitarySite), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("soldier_descr",
                       VariableType(VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Integer)), VariableType(VariableTypeID::Integer)))})),
   new BuiltinFunctionInfo(
      "ms_set_soldiers",
      []() { return _("Sets the number of soldiers garrisoned here. "
      "The argument is a table of soldier_description:amount pairs. "
      "A soldier description is an array that contains "
      "the levels for health, attack, defense and evade (in this order)."); },
      new FunctionBase("set_soldiers",
                       VariableType(VariableTypeID::MilitarySite), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("soldiers",
                       VariableType(VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Integer)), VariableType(VariableTypeID::Integer)))})),

	// TrainingSite

   new BuiltinFunctionInfo(
      "ts_get_soldiers",
      []() { return _("Returns the number of soldiers matching the given soldier description "
      "in training here. A soldier description is an array that contains "
      "the levels for health, attack, defense and evade (in this order)."); },
      new FunctionBase("get_soldiers",
                       VariableType(VariableTypeID::TrainingSite), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("soldier_descr",
                       VariableType(VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Integer)), VariableType(VariableTypeID::Integer)))})),
   new BuiltinFunctionInfo(
      "ts_set_soldiers",
      []() { return _("Sets the number of soldiers in training here. "
      "The argument is a table of soldier_description:amount pairs. "
      "A soldier description is an array that contains "
      "the levels for health, attack, defense and evade (in this order)."); },
      new FunctionBase("set_soldiers",
                       VariableType(VariableTypeID::TrainingSite), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("soldiers",
                       VariableType(VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Integer)), VariableType(VariableTypeID::Integer)))})),

   // Warehouse

   new BuiltinFunctionInfo(
      "wh_get_wares",
      []() { return _("Returns the number of wares of a certain type currently stored in this warehouse."); },
      new FunctionBase("get_wares",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("ware_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "wh_set_wares",
      []() { return _("Sets the number of wares of a certain type currently stored in this warehouse."); },
      new FunctionBase("set_wares",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("ware_name", VariableType(VariableTypeID::String)),
                       std::make_pair("amount", VariableType(VariableTypeID::Integer))})),
   new BuiltinFunctionInfo(
      "wh_get_workers",
      []() { return _("Returns the number of workers of a certain type currently stored in this warehouse."); },
      new FunctionBase("get_workers",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("worker_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "wh_set_workers",
      []() { return _("Sets the number of workers of a certain type currently stored in this warehouse."); },
      new FunctionBase("set_workers",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("worker_name", VariableType(VariableTypeID::String)),
                       std::make_pair("amount", VariableType(VariableTypeID::Integer))})),
   new BuiltinFunctionInfo(
      "wh_get_soldiers",
      []() { return _("Returns the number of soldiers matching the given soldier description "
      "currently stored in this warehouse. A soldier description is an array that contains "
      "the levels for health, attack, defense and evade (in this order)."); },
      new FunctionBase("get_soldiers",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {std::make_pair("soldier_descr",
                       VariableType(VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Integer)), VariableType(VariableTypeID::Integer)))})),
   new BuiltinFunctionInfo(
      "wh_set_soldiers",
      []() { return _("Sets the number of soldiers currently stored in this warehouse. "
      "The argument is a table of soldier_description:amount pairs. "
      "A soldier description is an array that contains "
      "the levels for health, attack, defense and evade (in this order)."); },
      new FunctionBase("set_soldiers",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("soldiers",
                       VariableType(VariableType(VariableType(VariableTypeID::Integer),
                       VariableType(VariableTypeID::Integer)), VariableType(VariableTypeID::Integer)))})),
   new BuiltinFunctionInfo(
      "wh_setpol",
      []() { return _("Set this warehouse’s storage policy for the given ware or worker. "
                      "Valid policies are 'normal', 'prefer', 'dontstock', and 'remove'."); },
      new FunctionBase("set_warehouse_policy",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {std::make_pair("internal_name", VariableType(VariableTypeID::String)),
                       std::make_pair("policy", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "wh_getpol",
      []() { return _("Returns this warehouse’s storage policy for the given ware or worker."); },
      new FunctionBase("get_warehouse_policy",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::String), // returns
                       {std::make_pair("internal_name", VariableType(VariableTypeID::String))})),
   new BuiltinFunctionInfo(
      "wh_exp_start",
      []() { return _("Starts an expedition from this warehouse if it is a port."); },
      new FunctionBase("start_expedition",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {})),
   new BuiltinFunctionInfo(
      "wh_exp_cancel",
      []() { return _("Cancel the preparations for an expedition from this warehouse if it "
                      "is a port and an expedition is being prepared."); },
      new FunctionBase("start_expedition",
                       VariableType(VariableTypeID::Warehouse), // call on
                       VariableType(VariableTypeID::Nil), // returns
                       {})),

   // Bob

   new BuiltinFunctionInfo(
      "bob_caps",
      []() { return _("Returns whether this bob can 'walk' or 'swim'."); },
      new FunctionBase("has_caps",
                       VariableType(VariableTypeID::Bob), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {std::make_pair("caps", VariableType(VariableTypeID::String))})),

   // Ship

   new BuiltinFunctionInfo(
      "ship_get_wares",
      []() { return _("Returns the number of wares on this ship."); },
      new FunctionBase("get_wares",
                       VariableType(VariableTypeID::Ship), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {})),
   new BuiltinFunctionInfo(
      "ship_get_workers",
      []() { return _("Returns the number of workers on this ship."); },
      new FunctionBase("get_workers",
                       VariableType(VariableTypeID::Ship), // call on
                       VariableType(VariableTypeID::Integer), // returns
                       {})),
   new BuiltinFunctionInfo(
      "ship_buildport",
      []() { return _("If this ship is an expedition ship that has found a port space), start "
                      "building a colonization port. Returns whether colonising was started."); },
      new FunctionBase("build_colonization_port",
                       VariableType(VariableTypeID::Ship), // call on
                       VariableType(VariableTypeID::Boolean), // returns
                       {})),

nullptr};

/************************************************************
                      Builtin properties
************************************************************/

// Do not change the order! Indices are stored in map files!
const BuiltinPropertyInfo* kBuiltinProperties[] = {

	// Game

	new BuiltinPropertyInfo("map", []() { return _("The map instance."); },
		new Property("map", true,
			VariableType(VariableTypeID::Game), // class
			VariableType(VariableTypeID::Map) // type
		)),
	new BuiltinPropertyInfo("real_speed", []() { return
	_("The speed the game is currently running at in milliseconds per real second."); },
		new Property("real_speed", true,
			VariableType(VariableTypeID::Game), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("desired_speed", []() { return
	_("Sets the desired speed of the game in milliseconds per real second. "
	"Note that this will not work in network games as expected."); },
		new Property("desired_speed", false,
			VariableType(VariableTypeID::Game), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("gametime", []() { return
	_("The absolute time elapsed since the game was started in milliseconds."); },
		new Property("time", true,
			VariableType(VariableTypeID::Game), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("gametype", []() { return
	_("One string out of ‘undefined’, ‘singleplayer’, ‘netclient’, ‘nethost’, ‘replay’, "
	"describing the type of game that is played."); },
		new Property("type", true,
			VariableType(VariableTypeID::Game), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("scenario_difficulty", []() { return
	_("The difficulty level of the current scenario. Values range from 1 "
	"to the number of levels specified in the campaign's configuration in "
	"campaigns.lua. By convention higher values mean more difficult."); },
		new Property("scenario_difficulty", true,
			VariableType(VariableTypeID::Game), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("players", []() { return _("An array with the player instances."); },
		new Property("players", true,
			VariableType(VariableTypeID::Game), // class
			VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::Player)) // type
		)),

	// Player

	new BuiltinPropertyInfo("pl_number", []() { return _("The player’s player number."); },
		new Property("number", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("pl_name", []() { return _("The player’s name."); },
		new Property("name", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("pl_tribename", []() { return _("The name of the player’s tribe."); },
		new Property("tribe_name", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("pl_tribe", []() { return _("The player’s tribe."); },
		new Property("tribe", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableTypeID::TribeDescr) // type
		)),
	new BuiltinPropertyInfo("pl_color", []() { return
	_("The playercolor assigned to this Player), in hex notation."); },
		new Property("color", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("pl_team", []() { return _("The player’s team number), 0 meaning no team."); },
		new Property("team", false,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("pl_defeated", []() { return _("Whether this player was defeated."); },
		new Property("defeated", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("pl_see_all", []() { return
	_("If you set this to true, the map will be completely visible for this player."); },
		new Property("see_all", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("pl_allowed_buildings", []() { return
	_("A table with name:bool values with all buildings that are "
	"currently allowed for this player. Note that you can not enable/forbid "
	"a building by setting the value. Use allow_buildings() or forbid_buildings() for that."); },
		new Property("allowed_buildings", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Boolean)) // type
		)),
	new BuiltinPropertyInfo("pl_objectives", []() { return
	_("A table of name:Objective. You can change the objectives in this table "
	"and it will be reflected in the game. To add a new item, use add_objective()."); },
		new Property("objectives", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Objective)) // type
		)),
	new BuiltinPropertyInfo("pl_messages", []() { return
	_("An array of all the messages sent to the player. Note that you can’t add "
	"messages to this array), use send_message() for that."); },
		new Property("messages", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::Message)) // type
		)),
	new BuiltinPropertyInfo("pl_inbox", []() { return
	_("An array of the messages that are either read or new. Note that "
	"you can’t add messages to this array), use send_message() for that."); },
		new Property("inbox", true,
			VariableType(VariableTypeID::Player), // class
			VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::Message)) // type
		)),

	// Objective

	new BuiltinPropertyInfo("obj_name", []() { return
	_("The internal name), to reference this objective in Player.objectives."); },
		new Property("name", true,
			VariableType(VariableTypeID::Objective), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("obj_title", []() { return _("The localized objective title."); },
		new Property("title", false,
			VariableType(VariableTypeID::Objective), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("obj_body", []() { return
	_("The complete localized objective text. May use richtext markup."); },
		new Property("body", false,
			VariableType(VariableTypeID::Objective), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("obj_visible", []() { return
	_("Whether this objective is shown in the Objectives menu."); },
		new Property("visible", false,
			VariableType(VariableTypeID::Objective), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("obj_done", []() { return
	_("Defines if this objective is already fulfilled. If done is true, "
	"the objective will not be shown to the user), no matter what "
	"visible is set to. A savegame will be created when this attribute is changed to true."); },
		new Property("done", false,
			VariableType(VariableTypeID::Objective), // class
			VariableType(VariableTypeID::Boolean) // type
		)),

	// Message

	new BuiltinPropertyInfo("msg_title", []() { return _("The message’s title."); },
		new Property("title", true,
			VariableType(VariableTypeID::Message), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("msg_heading", []() { return _("The extended title."); },
		new Property("heading", true,
			VariableType(VariableTypeID::Message), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("msg_body", []() { return _("The message’s full text."); },
		new Property("body", true,
			VariableType(VariableTypeID::Message), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("msg_sent", []() { return
	_("The game time in milliseconds when this message was sent."); },
		new Property("sent", true,
			VariableType(VariableTypeID::Message), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("msg_field", []() { return _("The field attached to this message), or nil."); },
		new Property("field", true,
			VariableType(VariableTypeID::Message), // class
			VariableType(VariableTypeID::Field) // type
		)),
	new BuiltinPropertyInfo("msg_icon_name", []() { return
	_("The filename for the icon that is shown with the message title."); },
		new Property("icon_name", true,
			VariableType(VariableTypeID::Message), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("msg_status", []() { return _("May be 'new', 'read', or 'archived'."); },
		new Property("status", false,
			VariableType(VariableTypeID::Message), // class
			VariableType(VariableTypeID::String) // type
		)),

	// Map

	new BuiltinPropertyInfo("map_allows_seafaring", []() { return
			_("Whether seafaring is possible on this map."); },
		new Property("allows_seafaring", true,
			VariableType(VariableTypeID::Map), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("map_nrports", []() { return _("The number of port spaces on this map."); },
		new Property("number_of_port_spaces", true,
			VariableType(VariableTypeID::Map), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("map_ports", []() { return
	_("An array of tables of {x, y} pairs with the coordinates of the port spaces."); },
		new Property("port_spaces", true,
			VariableType(VariableTypeID::Map), // class
			VariableType(VariableType(VariableTypeID::Integer),
			VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::Integer))) // type
		)),
	new BuiltinPropertyInfo("map_w", []() { return _("The map’s width."); },
		new Property("width", true,
			VariableType(VariableTypeID::Map), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("map_h", []() { return _("The map’s height."); },
		new Property("height", true,
			VariableType(VariableTypeID::Map), // class
			VariableType(VariableTypeID::Integer) // type
		)),

	// TribeDescr

	new BuiltinPropertyInfo("td_name", []() { return _("The tribe’s internal name"); },
		new Property("name", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_descname", []() { return _("The tribe’s localized name"); },
		new Property("descname", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_carrier", []() { return
	_("The internal name of the carrier type that this tribe uses"); },
		new Property("carrier", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_carrier2", []() { return
	_("The internal name of the second carrier type that this tribe uses"); },
		new Property("carrier2", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_geologist", []() { return
	_("The internal name of the geologist that this tribe uses"); },
		new Property("geologist", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_soldier", []() { return
	_("The internal name of the soldier that this tribe uses"); },
		new Property("soldier", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_ship", []() { return _("The internal name of the ship that this tribe uses"); },
		new Property("ship", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_port", []() { return _("The internal name of the port that this tribe uses"); },
		new Property("port", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_buildings", []() { return
	_("An array with all the BuildingDescriptions this tribe uses"); },
		new Property("buildings", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableType(VariableTypeID::Integer),
			VariableType(VariableTypeID::BuildingDescr)) // type
		)),
	new BuiltinPropertyInfo("td_workers", []() { return
	_("An array with all the WorkerDescriptions this tribe uses"); },
		new Property("workers", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableType(VariableTypeID::Integer),
			VariableType(VariableTypeID::WorkerDescr)) // type
		)),
	new BuiltinPropertyInfo("td_wares", []() { return
	_("An array with all the WareDescriptions this tribe uses"); },
		new Property("wares", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableType(VariableTypeID::Integer),
			VariableType(VariableTypeID::WorkerDescr)) // type
		)),
	new BuiltinPropertyInfo("td_immovables", []() { return
	_("An array with all the ImmovableDescriptions this tribe uses"); },
		new Property("immovables", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableType(VariableTypeID::Integer),
			VariableType(VariableTypeID::ImmovableDescr)) // type
		)),
	new BuiltinPropertyInfo("td_resi", []() { return
	_("A table with the resource indicators this tribe uses"); },
		new Property("resource_indicators", true,
			VariableType(VariableTypeID::TribeDescr), // class
			VariableType(VariableType(VariableTypeID::Integer),
			VariableType(VariableTypeID::ImmovableDescr)) // type
		)),

	// MapObjectDescr

	new BuiltinPropertyInfo("mo_d_name", []() { return _("The map object’s internal name"); },
		new Property("name", true,
			VariableType(VariableTypeID::MapObjectDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("mo_d_descname", []() { return _("The map object’s localized name"); },
		new Property("descname", true,
			VariableType(VariableTypeID::MapObjectDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("mo_d_type", []() { return _("The map object’s type"); },
		new Property("type_name", true,
			VariableType(VariableTypeID::MapObjectDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("mo_d_icon_name", []() { return _("The filename for the menu icon"); },
		new Property("icon_name", true,
			VariableType(VariableTypeID::MapObjectDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("mo_d_help", []() { return
	_("The path and filename to the helptext script. Can be empty."); },
		new Property("helptext_script", true,
			VariableType(VariableTypeID::MapObjectDescr), // class
			VariableType(VariableTypeID::String) // type
		)),

	// ImmovableDescr

	new BuiltinPropertyInfo("immo_d_size", []() { return
	_("The size of this immovable: 'none', 'small', 'medium', or 'big'"); },
		new Property("size", true,
			VariableType(VariableTypeID::ImmovableDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("immo_d_owner_type", []() { return
	_("'world' for world immovables and 'tribe' for tribe immovables"); },
		new Property("owner_type", true,
			VariableType(VariableTypeID::ImmovableDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("immo_d_terrain_affinity", []() { return
	_("A table containing numbers labeled as 'pickiness', "
	"'preferred_fertility', 'preferred_humidity', and 'preferred_temperature', "
	"or nil if the immovable has no terrain affinity"); },
		new Property("terrain_affinity", true,
			VariableType(VariableTypeID::ImmovableDescr), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Integer)) // type
		)),
	new BuiltinPropertyInfo("immo_d_species", []() { return
	_("The localized species name of a tree (empty if this immovable is not a tree)"); },
		new Property("species", true,
			VariableType(VariableTypeID::ImmovableDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("immo_d_buildcost", []() { return
	_("A table of ware-to-count pairs describing the buildcost for the immovable"); },
		new Property("buildcost", true,
			VariableType(VariableTypeID::ImmovableDescr), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Integer)) // type
		)),

	// BuildingDescr

	new BuiltinPropertyInfo("bld_d_conquers", []() { return _("The conquer radius"); },
		new Property("conquers", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("bld_d_vision_range", []() { return _("The building’s vision range"); },
		new Property("vision_range", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("bld_d_workarea_radius", []() { return
	_("The first workarea radius of the building), or nil in case the building has no workarea"); },
		new Property("workarea_radius", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("bld_d_is_mine", []() { return
	_("Whether this building can be built only on mining plots"); },
		new Property("is_mine", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("bld_d_is_port", []() { return
	_("Whether this building can be built only on port spaces"); },
		new Property("is_port", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("bld_d_buildable", []() { return
	_("Whether the building can be built directly by the player"); },
		new Property("buildable", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("bld_d_destructible", []() { return
	_("Whether the building can be burnt down by the player"); },
		new Property("destructible", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("bld_d_enhanced_from", []() { return
	_("The building type that can be enhanced to this building), or nil if this is not an enhanced building"); },
		new Property("enhanced_from", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableTypeID::BuildingDescr) // type
		)),
	new BuiltinPropertyInfo("bld_d_enhancement", []() { return
	_("The building type this building can be enhanced to, or nil if this building is not enhanceable"); },
		new Property("enhancement", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableTypeID::BuildingDescr) // type
		)),
	new BuiltinPropertyInfo("bld_d_enhancement_cost", []() { return
	_("The cost for enhancing this building"); },
		new Property("enhancement_cost", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Integer)) // type
		)),
	new BuiltinPropertyInfo("bld_d_returned_wares", []() { return
	_("The list of wares returned upon dismantling"); },
		new Property("returned_wares", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Integer)) // type
		)),
	new BuiltinPropertyInfo("bld_d_returned_wares_enhanced", []() {
	return _("The list of wares returned upon dismantling an enhanced building"); },
		new Property("returned_wares_enhanced", true,
			VariableType(VariableTypeID::BuildingDescr), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Integer)) // type
		)),

	// ProductionSiteDescr

	new BuiltinPropertyInfo("pd_d_inputs", []() { return
	_("An array of WareDescriptions containing the wares this productionsite needs for its production"); },
		new Property("inputs", true,
			VariableType(VariableTypeID::ProductionSiteDescr), // class
			VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::WareDescr)) // type
		)),
	new BuiltinPropertyInfo("pd_d_output_ware_types", []() { return
	_("An array with WareDescriptions containing the wares this productionsite can produce"); },
		new Property("output_ware_types", true,
			VariableType(VariableTypeID::ProductionSiteDescr), // class
			VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::WareDescr)) // type
		)),
	new BuiltinPropertyInfo("pd_d_output_worker_types", []() { return
	_("An array with WorkerDescriptions containing the workers this productionsite can recruit"); },
		new Property("output_worker_types", true,
			VariableType(VariableTypeID::ProductionSiteDescr), // class
			VariableType(VariableType(VariableTypeID::Integer),
			VariableType(VariableTypeID::WorkerDescr)) // type
		)),
	new BuiltinPropertyInfo("pd_d_production_programs", []() { return
	_("An array with the production program names as string"); },
		new Property("production_programs", true,
			VariableType(VariableTypeID::ProductionSiteDescr), // class
			VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::String)) // type
		)),
	new BuiltinPropertyInfo("pd_d_working_positions", []() { return
	_("An array with WorkerDescriptions containing the workers that need to work here. "
	"If several instances of a certain worker type are required), this instance is contained several times."); },
		new Property("working_positions", true,
			VariableType(VariableTypeID::ProductionSiteDescr), // class
			VariableType(VariableType(VariableTypeID::Integer),
			VariableType(VariableTypeID::WorkerDescr)) // type
		)),

	// MilitarySiteDescr

	new BuiltinPropertyInfo("ms_d_heal", []() { return
	_("The number of health healed per second by the militarysite"); },
		new Property("heal_per_second", true,
			VariableType(VariableTypeID::MilitarySiteDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("ms_d_maxsoldiers", []() { return
	_("The number of soldiers that can be garrisoned at the militarysite"); },
		new Property("max_number_of_soldiers", true,
			VariableType(VariableTypeID::MilitarySiteDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),

	// WarehouseDescr

	new BuiltinPropertyInfo("wh_d_heal", []() { return
	_("The number of health healed per second for soldiers in the warehouse"); },
		new Property("heal_per_second", true,
			VariableType(VariableTypeID::WarehouseDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),

	// TrainingSiteDescr

	new BuiltinPropertyInfo("ts_d_min_a", []() { return
	_("The minimum attack level soldier may start training with"); },
		new Property("min_attack", true,
			VariableType(VariableTypeID::TrainingSiteDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("ts_d_min_h", []() { return
	_("The minimum health level soldier may start training with"); },
		new Property("min_health", true,
			VariableType(VariableTypeID::TrainingSiteDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("ts_d_min_d", []() { return
	_("The minimum defense level soldier may start training with"); },
		new Property("min_defense", true,
			VariableType(VariableTypeID::TrainingSiteDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("ts_d_min_e", []() { return
	_("The minimum evade level soldier may start training with"); },
		new Property("min_evade", true,
			VariableType(VariableTypeID::TrainingSiteDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("ts_d_max_a", []() { return
	_("The attack level up to which a soldier can train here"); },
		new Property("max_attack", true,
			VariableType(VariableTypeID::TrainingSiteDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("ts_d_max_h", []() { return
	_("The health level up to which a soldier can train here"); },
		new Property("max_health", true,
			VariableType(VariableTypeID::TrainingSiteDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("ts_d_max_d", []() { return
	_("The defense level up to which a soldier can train here"); },
		new Property("max_defense", true,
			VariableType(VariableTypeID::TrainingSiteDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("ts_d_max_e", []() { return
	_("The evade level up to which a soldier can train here"); },
		new Property("max_evade", true,
			VariableType(VariableTypeID::TrainingSiteDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),

	// WorkerDescr

	new BuiltinPropertyInfo("wd_becomes", []() { return
	_("The WorkerDescription of the worker this one will level up to or nil if it never levels up."); },
		new Property("becomes", true,
			VariableType(VariableTypeID::WorkerDescr), // class
			VariableType(VariableTypeID::WorkerDescr) // type
		)),
	new BuiltinPropertyInfo("wd_needed_experience", []() { return
	_("The amount of experience points this worker needs to level up"); },
		new Property("needed_experience", true,
			VariableType(VariableTypeID::WorkerDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("wd_buildable", []() { return
	_("Whether this worker can be built in warehouses"); },
		new Property("buildable", true,
			VariableType(VariableTypeID::WorkerDescr), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("wd_buildcost", []() { return
	_("A table with the buildcost for this worker if it may be created in warehouses"); },
		new Property("buildcost", true,
			VariableType(VariableTypeID::WorkerDescr), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Integer)) // type
		)),
	new BuiltinPropertyInfo("wd_employers", []() { return
	_("An array with BuildingDescriptions with buildings where this worker can be employed"); },
		new Property("employers", true,
			VariableType(VariableTypeID::WorkerDescr), // class
			VariableType(VariableType(VariableTypeID::Integer),
			VariableType(VariableTypeID::BuildingDescr)) // type
		)),

	// SoldierDescr

	new BuiltinPropertyInfo("sd_h_max", []() { return _("The maximum health level this soldier can have"); },
		new Property("max_health_level", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_a_max", []() { return _("The maximum attack level this soldier can have"); },
		new Property("max_attack_level", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_d_max", []() { return _("The maximum defense level this soldier can have"); },
		new Property("max_defense_level", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_e_max", []() { return _("The maximum evade level this soldier can have"); },
		new Property("max_evade_level", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_h_base", []() { return
	_("The number of health points this soldier starts with"); },
		new Property("base_health", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_a_base", []() { return _("The attack strength this soldier starts with"); },
		new Property("base_attack", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_d_base", []() { return
	_("The blow absorption percentage this soldier starts with"); },
		new Property("base_defense", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_e_base", []() { return
	_("The evade chance in percent this soldier starts with"); },
		new Property("base_evade", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_h_incr", []() { return _("The number of hitpoints gained per health level"); },
		new Property("health_incr_per_level", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_a_incr", []() { return _("The attack strength gained per attack level"); },
		new Property("attack_incr_per_level", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_d_incr", []() { return
	_("The blow absorption rate increase gained per defense level in percent points"); },
		new Property("defense_incr_per_level", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("sd_e_incr", []() { return
	_("The evade chance increase gained per evade level in percent points"); },
		new Property("evade_incr_per_level", true,
			VariableType(VariableTypeID::SoldierDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),

	// ResourceDescr

	new BuiltinPropertyInfo("rd_name", []() { return _("The resource’s internal name"); },
		new Property("name", true,
			VariableType(VariableTypeID::ResourceDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("rd_descname", []() { return _("The resource’s localized name"); },
		new Property("descname", true,
			VariableType(VariableTypeID::ResourceDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("rd_dect", []() { return _("Whether geologists can find this resource"); },
		new Property("is_detectable", true,
			VariableType(VariableTypeID::ResourceDescr), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("rd_max_amount", []() { return
	_("The highest amount of this resource that can be contained on a field"); },
		new Property("max_amount", true,
			VariableType(VariableTypeID::ResourceDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("rd_representative_image", []() { return
	_("The path to the image representing this resource in the GUI"); },
		new Property("representative_image", true,
			VariableType(VariableTypeID::ResourceDescr), // class
			VariableType(VariableTypeID::String) // type
		)),

	// TerrainDescr

	new BuiltinPropertyInfo("td_name", []() { return _("The terrain’s internal name"); },
		new Property("name", true,
			VariableType(VariableTypeID::TerrainDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_descname", []() { return _("The terrain’s localized name"); },
		new Property("descname", true,
			VariableType(VariableTypeID::TerrainDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_default_resource", []() { return
	_("The resource that can be found here unless another resource is explicitly placed here. Can be nil."); },
		new Property("default_resource", true,
			VariableType(VariableTypeID::TerrainDescr), // class
			VariableType(VariableTypeID::ResourceDescr) // type
		)),
	new BuiltinPropertyInfo("td_default_resource_amount", []() { return
	_("The amount of the default resource provided by this terrain"); },
		new Property("default_resource_amount", true,
			VariableType(VariableTypeID::TerrainDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("td_valid_resources", []() { return
	_("An array of ResourceDescriptions with all valid resources for this terrain"); },
		new Property("valid_resources", true,
			VariableType(VariableTypeID::TerrainDescr), // class
			VariableType(VariableType(VariableTypeID::Integer),
			VariableType(VariableTypeID::ResourceDescr)) // type
		)),
	new BuiltinPropertyInfo("td_representative_image", []() { return
	_("The file path to the representative image"); },
		new Property("representative_image", true,
			VariableType(VariableTypeID::TerrainDescr), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("td_temperature", []() { return
	_("The temperature of this terrain (regarding immovables’ terrain affinity)"); },
		new Property("temperature", true,
			VariableType(VariableTypeID::TerrainDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("td_humidity", []() { return
	_("The humidity of this terrain (regarding immovables’ terrain affinity)"); },
		new Property("humidity", true,
			VariableType(VariableTypeID::TerrainDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("td_fertility", []() { return
	_("The fertility of this terrain (regarding immovables’ terrain affinity)"); },
		new Property("fertility", true,
			VariableType(VariableTypeID::TerrainDescr), // class
			VariableType(VariableTypeID::Integer) // type
		)),

	// MapObject

	new BuiltinPropertyInfo("mo_serial", []() { return
	_("The unique serial number of this map object. "
	"Note that this value may not stay constant after saving/loading."); },
		new Property("serial", true,
			VariableType(VariableTypeID::MapObject), // class
			VariableType(VariableTypeID::Integer) // type
		)),

// MapObject descrs

#define DESCR_FOR(type)                                                                            \
	new BuiltinPropertyInfo(                                                                        \
	   "descr_" #type, []() { return _("The MapObjectDescr associated with this map object"); },    \
	   new Property("descr", true, VariableType(VariableTypeID::type),                              \
	                VariableType(VariableTypeID::type##Descr), false))

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

	new BuiltinPropertyInfo("baseimmo_fields", []() { return
	_("An array of Field that is occupied by this Immovable. "
	"If the immovable occupies more than one field (roads or big buildings for "
	"example) the first entry in this list will be the main field"); },
		new Property("fields", true,
			VariableType(VariableTypeID::BaseImmovable), // class
			VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::Field)) // type
		)),

	// PlayerImmovable

	new BuiltinPropertyInfo("plimmo_owner", []() { return _("The player owning this player immovable"); },
		new Property("owner", true,
			VariableType(VariableTypeID::PlayerImmovable), // class
			VariableType(VariableTypeID::Player) // type
		)),

	// Flag

	new BuiltinPropertyInfo("flag_waeco", []() { return
	_("The ware economy that this flag belongs to. Warning: Since economies "
	"can disappear when a player merges them through placing/deleting "
	"roads and flags, you must get a fresh economy object every time you call "
	"another function on the resulting economy object."); },
		new Property("ware_economy", true,
			VariableType(VariableTypeID::Flag), // class
			VariableType(VariableTypeID::Economy) // type
		)),
	new BuiltinPropertyInfo("flag_woeco", []() { return
	_("The worker economy that this flag belongs to. Warning: Since economies "
	"can disappear when a player merges them through placing/deleting "
	"roads and flags, you must get a fresh economy object every time you call "
	"another function on the resulting economy object."); },
		new Property("worker_economy", true,
			VariableType(VariableTypeID::Flag), // class
			VariableType(VariableTypeID::Economy) // type
		)),
	new BuiltinPropertyInfo("flag_building", []() { return _("The building attached to this flag."); },
		new Property("building", true,
			VariableType(VariableTypeID::Flag), // class
			VariableType(VariableTypeID::Building) // type
		)),
	new BuiltinPropertyInfo("flag_roads", []() { return
	_("The roads leading to the flag. Directions can be 'tr', 'r', 'br', 'bl', 'l', and 'tl'."); },
		new Property("roads", true,
			VariableType(VariableTypeID::Flag), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Road)) // type
		)),

	// Road

	new BuiltinPropertyInfo("road_length", []() { return _("The number of edges this road covers."); },
		new Property("length", true,
			VariableType(VariableTypeID::Road), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("road_start_flag", []() { return _("The flag where this road starts"); },
		new Property("start_flag", true,
			VariableType(VariableTypeID::Road), // class
			VariableType(VariableTypeID::Flag) // type
		)),
	new BuiltinPropertyInfo("road_end_flag", []() { return _("The flag where this road ends"); },
		new Property("end_flag", true,
			VariableType(VariableTypeID::Road), // class
			VariableType(VariableTypeID::Flag) // type
		)),
	new BuiltinPropertyInfo("road_type", []() { return _("'normal', 'busy', or 'waterway'"); },
		new Property("road_type", true,
			VariableType(VariableTypeID::Road), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("road_valid_workers", []() { return
			_("A table of the workers that can be employed on this road), "
			"in the format worker_name:number_of_working_positions"); },
		new Property("valid_workers", true,
			VariableType(VariableTypeID::Road), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Integer)) // type
		)),

	// Building

	new BuiltinPropertyInfo("bld_flag", []() { return _("The flag this building belongs to"); },
		new Property("flag", true,
			VariableType(VariableTypeID::Building), // class
			VariableType(VariableTypeID::Flag) // type
		)),

	// MilitarySite

	new BuiltinPropertyInfo("ms_capacity", []() { return
			_("The maximum number of soldiers that can be garrisoned in this building"); },
		new Property("max_soldiers", true,
			VariableType(VariableTypeID::MilitarySite), // class
			VariableType(VariableTypeID::Flag) // type
		)),

	// TrainingSite

	new BuiltinPropertyInfo("ts_capacity", []() { return
			_("The maximum number of soldiers that can train here at the same time"); },
		new Property("max_soldiers", true,
			VariableType(VariableTypeID::TrainingSite), // class
			VariableType(VariableTypeID::Flag) // type
		)),

	// ConstructionSite

	new BuiltinPropertyInfo("cs_building", []() { return
	_("The name of the building under construction"); },
		new Property("building", true,
			VariableType(VariableTypeID::ConstructionSite), // class
			VariableType(VariableTypeID::String) // type
		)),

	// Warehouse

	new BuiltinPropertyInfo("wh_portdock", []() { return
	_("The PortDock associated with this port), or nil if this warehouse is not a port."); },
		new Property("portdock", true,
			VariableType(VariableTypeID::Warehouse), // class
			VariableType(VariableTypeID::PortDock) // type
		)),
	new BuiltinPropertyInfo("wh_exp_in_p", []() { return
	_("Whether this is a port and an expedition is being prepared here"); },
		new Property("expedition_in_progress", true,
			VariableType(VariableTypeID::Warehouse), // class
			VariableType(VariableTypeID::Boolean) // type
		)),

	// ProductionSite

	new BuiltinPropertyInfo("ps_stopped", []() { return
	_("Whether this productionsite is currently stopped"); },
		new Property("is_stopped", true,
			VariableType(VariableTypeID::ProductionSite), // class
			VariableType(VariableTypeID::Boolean) // type
		)),
	new BuiltinPropertyInfo("ps_valid_workers", []() { return
			_("A table of the workers that can be employed in this productionsite), "
			"in the format worker_name:number_of_working_positions"); },
		new Property("valid_workers", true,
			VariableType(VariableTypeID::ProductionSite), // class
			VariableType(VariableType(VariableTypeID::String), VariableType(VariableTypeID::Integer)) // type
		)),

	// Bob

	new BuiltinPropertyInfo("bob_field", []() { return
	_("The field this bob is currently located on"); },
		new Property("field", true,
			VariableType(VariableTypeID::Bob), // class
			VariableType(VariableTypeID::Field) // type
		)),

	// Ship

	new BuiltinPropertyInfo("ship_shipname", []() { return _("The ship’s name"); },
		new Property("shipname", true,
			VariableType(VariableTypeID::Ship), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("ship_destination", []() { return
	_("The port dock this ship is heading to, or nil if it has no destination"); },
		new Property("destination", true,
			VariableType(VariableTypeID::Ship), // class
			VariableType(VariableTypeID::PortDock) // type
		)),
	new BuiltinPropertyInfo("ship_last_portdock", []() { return
	_("nil if no port was ever visited or the last portdock was destroyed; "
	"otherwise the port dock of the last visited port"); },
		new Property("last_portdock", true,
			VariableType(VariableTypeID::Ship), // class
			VariableType(VariableTypeID::PortDock) // type
		)),
	new BuiltinPropertyInfo("ship_state", []() { return
	_("What this ship is currently doing: 'transport', 'exp_scouting', "
	"'exp_found_port_space', 'exp_colonizing', 'sink_request', or 'sink_animation'"); },
		new Property("state", true,
			VariableType(VariableTypeID::Ship), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("ship_island_explore_direction", []() { return
	_("The direction if the ship is an expedition ship sailing "
	"around an island. Valid values are 'cw', 'ccw', and nil."); },
		new Property("island_explore_direction", false,
			VariableType(VariableTypeID::Ship), // class
			VariableType(VariableTypeID::String) // type
		)),

	// Worker

	new BuiltinPropertyInfo("worker_owner", []() { return _("The player this worker belongs to"); },
		new Property("owner", true,
			VariableType(VariableTypeID::Worker), // class
			VariableType(VariableTypeID::Player) // type
		)),
	new BuiltinPropertyInfo("worker_location", []() { return
	_("The location where this worker is situated. "
	"This will be either a Building), Road), Flag or nil. "
	"Note that a worker that is stored in a warehouse has a location nil. "
	"A worker that is out working (e.g. hunter) has as a location his building. "
	"A stationed soldier has his military building as location. "
	"Workers on transit usually have the Road they are currently on as location."); },
		new Property("location", true,
			VariableType(VariableTypeID::Worker), // class
			VariableType(VariableTypeID::PlayerImmovable) // type
		)),

	// Soldier

	new BuiltinPropertyInfo("soldier_level_h", []() { return _("The soldier’s current health level"); },
		new Property("health_level", true,
			VariableType(VariableTypeID::Soldier), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("soldier_level_a", []() { return _("The soldier’s current attack level"); },
		new Property("attack_level", true,
			VariableType(VariableTypeID::Soldier), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("soldier_level_d", []() { return _("The soldier’s current defense level"); },
		new Property("defense_level", true,
			VariableType(VariableTypeID::Soldier), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("soldier_level_e", []() { return _("The soldier’s current evade level"); },
		new Property("evade_level", true,
			VariableType(VariableTypeID::Soldier), // class
			VariableType(VariableTypeID::Integer) // type
		)),

	// Field

	new BuiltinPropertyInfo("f_x", []() { return _("The x coordinate of this field"); },
		new Property("x", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("f_y", []() { return _("The y coordinate of this field"); },
		new Property("y", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("f_height", []() { return
	_("The height of this field. Note though that if you change this value "
	"too much, all surrounding fields will also change their heights because "
	"the slope is constrained. If you are changing the height of many terrains "
	"at once), use raw_height instead and then call recalculate() afterwards."); },
		new Property("height", false,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("f_raw_height", []() { return
	_("The same as 'height', but setting this will not trigger a recalculation "
	"of the surrounding fields. You can use this field to change the height "
	"of many fields on a map quickly), then use recalculate() to make sure that everything is in order."); },
		new Property("raw_height", false,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("f_viewpoint_x", []() { return
	_("Returns the position in pixels to move the view to to center "
	"this field for the current interactive player"); },
		new Property("viewpoint_x", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("f_viewpoint_y", []() { return
	_("Returns the position in pixels to move the view to to center "
	"this field for the current interactive player"); },
		new Property("viewpoint_y", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("f_resource", []() { return
	_("The name of the resource on this field. May be nil."); },
		new Property("resource", false,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("f_resource_amount", []() { return _("The resource amount left on this field."); },
		new Property("resource_amount", false,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("f_initial_resource_amount", []() { return
	_("The resource amount originally present on this field."); },
		new Property("initial_resource_amount", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Integer) // type
		)),
	new BuiltinPropertyInfo("f_immovable", []() { return _("The immovable occupying this field. May be nil."); },
		new Property("immovable", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::BaseImmovable) // type
		)),
	new BuiltinPropertyInfo("f_bobs", []() { return _("An array with all bobs currently located on this field"); },
		new Property("bobs", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::Bob)) // type
		)),
	new BuiltinPropertyInfo("f_terd", []() { return
	_("The name of the terrain on the triangle straight south of this field "); },
		new Property("terd", false,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("f_terr", []() { return
	_("The name of the terrain on the triangle southeast of this field "); },
		new Property("terr", false,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::String) // type
		)),
	new BuiltinPropertyInfo("f_owner", []() { return _("The player owning this field"); },
		new Property("owner", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Player) // type
		)),
	new BuiltinPropertyInfo("f_claimers", []() { return
	_("An array of players that have military influence over this field "
	"sorted by the amount of influence they have. Note that this does "
	"not necessarily mean that claimers[1] is also the owner of the "
	"field), as a field that houses a surrounded military building is "
	"owned by the surrounded Player), but others have more military influence over it."); },
		new Property("claimers", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableType(VariableTypeID::Integer), VariableType(VariableTypeID::Player)) // type
		)),
	new BuiltinPropertyInfo("f_bln", []() { return _("The southwestern neighbour of this field"); },
		new Property("bln", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Field) // type
		)),
	new BuiltinPropertyInfo("f_ln", []() { return _("The western neighbour of this field"); },
		new Property("ln", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Field) // type
		)),
	new BuiltinPropertyInfo("f_tln", []() { return _("The northwestern neighbour of this field"); },
		new Property("tln", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Field) // type
		)),
	new BuiltinPropertyInfo("f_brn", []() { return _("The southeastern neighbour of this field"); },
		new Property("brn", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Field) // type
		)),
	new BuiltinPropertyInfo("f_rn", []() { return _("The eastern neighbour of this field"); },
		new Property("rn", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Field) // type
		)),
	new BuiltinPropertyInfo("f_trn", []() { return _("The northeastern neighbour of this field"); },
		new Property("trn", true,
			VariableType(VariableTypeID::Field), // class
			VariableType(VariableTypeID::Field) // type
		)),

nullptr};
