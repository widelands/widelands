/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#include "scripting/map/lua_training_site_description.h"

namespace LuaMaps {

/* RST
TrainingSiteDescription
-----------------------

.. class:: TrainingSiteDescription

   A static description of a tribe's trainingsite.

   A training site can train some or all of a soldier's properties (attack, defense, evade and
   health). See the parent classes for more properties.
*/
const char LuaTrainingSiteDescription::className[] = "TrainingSiteDescription";
const MethodType<LuaTrainingSiteDescription> LuaTrainingSiteDescription::Methods[] = {
   METHOD(LuaTrainingSiteDescription, trained_soldiers),
   {nullptr, nullptr},
};
const PropertyType<LuaTrainingSiteDescription> LuaTrainingSiteDescription::Properties[] = {
   PROP_RO(LuaTrainingSiteDescription, max_attack),
   PROP_RO(LuaTrainingSiteDescription, max_defense),
   PROP_RO(LuaTrainingSiteDescription, max_evade),
   PROP_RO(LuaTrainingSiteDescription, max_health),
   PROP_RO(LuaTrainingSiteDescription, max_number_of_soldiers),
   PROP_RO(LuaTrainingSiteDescription, min_attack),
   PROP_RO(LuaTrainingSiteDescription, min_defense),
   PROP_RO(LuaTrainingSiteDescription, min_evade),
   PROP_RO(LuaTrainingSiteDescription, min_health),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: max_attack

      (RO) The number of attack points that a soldier can train.
*/
int LuaTrainingSiteDescription::get_max_attack(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_attack()) {
		lua_pushinteger(L, descr->get_max_level(Widelands::TrainingAttribute::kAttack));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_defense

      (RO) The number of defense points that a soldier can train.
*/
int LuaTrainingSiteDescription::get_max_defense(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_defense()) {
		lua_pushinteger(L, descr->get_max_level(Widelands::TrainingAttribute::kDefense));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_evade

      (RO) The number of evade points that a soldier can train.
*/
int LuaTrainingSiteDescription::get_max_evade(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_evade()) {
		lua_pushinteger(L, descr->get_max_level(Widelands::TrainingAttribute::kEvade));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_health

      (RO) The number of health points that a soldier can train.
*/
int LuaTrainingSiteDescription::get_max_health(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_health()) {
		lua_pushinteger(L, descr->get_max_level(Widelands::TrainingAttribute::kHealth));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_number_of_soldiers

      (RO) The number of soldiers that can be garrisoned at the trainingsite.
*/
int LuaTrainingSiteDescription::get_max_number_of_soldiers(lua_State* L) {
	lua_pushinteger(L, get()->get_max_number_of_soldiers());
	return 1;
}

/* RST
   .. attribute:: min_attack

      (RO) The number of attack points that a soldier starts training with.
*/
int LuaTrainingSiteDescription::get_min_attack(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_attack()) {
		lua_pushinteger(L, descr->get_min_level(Widelands::TrainingAttribute::kAttack));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: min_defense

      (RO) The number of defense points that a soldier starts training with.
*/
int LuaTrainingSiteDescription::get_min_defense(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_defense()) {
		lua_pushinteger(L, descr->get_min_level(Widelands::TrainingAttribute::kDefense));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: min_evade

      (RO) The number of evade points that a soldier starts training with.
*/
int LuaTrainingSiteDescription::get_min_evade(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_evade()) {
		lua_pushinteger(L, descr->get_min_level(Widelands::TrainingAttribute::kEvade));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: min_health

      (RO) The number of health points that a soldier starts training with.
*/
int LuaTrainingSiteDescription::get_min_health(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_health()) {
		lua_pushinteger(L, descr->get_min_level(Widelands::TrainingAttribute::kHealth));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. method:: trained_soldiers(program_name)

Returns a :class:`table` with following entries [1] = the trained skill, [2] = the starting level,
      [3] = the resulting level trained by this production program.
      See :ref:`production site programs <productionsite_programs>`.

      :arg program_name: the name of the production program that we want to get the trained
         soldiers for.
      :type program_name: :class:`string`

*/
int LuaTrainingSiteDescription::trained_soldiers(lua_State* L) {
	std::string program_name = luaL_checkstring(L, -1);
	const Widelands::ProductionSiteDescr::Programs& programs = get()->programs();
	if (programs.count(program_name) == 1) {
		const Widelands::ProductionProgram& program = *programs.at(program_name);
		lua_newtable(L);
		lua_pushint32(L, 1);
		lua_pushstring(L, program.trained_attribute());
		lua_settable(L, -3);
		lua_pushint32(L, 2);
		lua_pushint32(L, program.train_from_level());
		lua_settable(L, -3);
		lua_pushint32(L, 3);
		lua_pushint32(L, program.train_to_level());
		lua_settable(L, -3);
	}
	return 1;
}

}  // namespace LuaMaps
