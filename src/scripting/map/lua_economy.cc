/*
 * Copyright (C) 2006-2026 by the Widelands Development Team
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

#include "scripting/map/lua_economy.h"

#include "logic/player.h"
#include "scripting/globals.h"
#include "scripting/map/lua_flag.h"

namespace LuaMaps {

/* RST
Economy
-------
.. class:: Economy

   Provides access to an economy. An economy will be created each time a player places a
   flag on the map. As soon this flag is connected to another flag, their two economies will
   be merged into a single economy. A player can have multiple economies, each of which has
   its own set of economy target settings.

   You can get an economy from a :class:`Flag`.
*/
const char LuaEconomy::className[] = "Economy";
const MethodType<LuaEconomy> LuaEconomy::Methods[] = {
   METHOD(LuaEconomy, __eq),
   METHOD(LuaEconomy, target_quantity),
   METHOD(LuaEconomy, set_target_quantity),
   METHOD(LuaEconomy, needs),
   {nullptr, nullptr},
};
const PropertyType<LuaEconomy> LuaEconomy::Properties[] = {
   {nullptr, nullptr, nullptr},
};

void LuaEconomy::__persist(lua_State* L) {
	const Widelands::Economy* economy = get();
	const Widelands::Player& player = economy->owner();
	PERS_UINT32("player", player.player_number());
	PERS_UINT32("economy", economy->serial());
}

void LuaEconomy::__unpersist(lua_State* L) {
	Widelands::PlayerNumber player_number;
	Widelands::Serial economy_serial;
	UNPERS_UINT32("player", player_number)
	UNPERS_UINT32("economy", economy_serial)
	const Widelands::Player& player = get_egbase(L).player(player_number);
	set_economy_pointer(player.get_economy(economy_serial));
}

int LuaEconomy::__eq(lua_State* L) {
	lua_pushboolean(L, static_cast<int>((*get_user_class<LuaEconomy>(L, -1))->get() == get()));
	return 1;
}

/* RST
   .. method:: target_quantity(name)

      Returns the amount of the given ware or worker that should be kept in stock for this economy.
      Whether this works only for wares or only for workers is determined by the type of this
      economy.

      **Warning**: Since economies can disappear when a player merges them
      through placing/deleting roads and flags, you must get a fresh economy
      object every time you use this function.

      :arg name: The name of the ware or worker.
      :type name: :class:`string`
      :returns: :class:`integer`
*/
int LuaEconomy::target_quantity(lua_State* L) {
	const std::string wname = luaL_checkstring(L, 2);
	switch (get()->type()) {
	case Widelands::wwWARE: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().ware_index(wname);
		if (get_egbase(L).descriptions().ware_exists(index)) {
			const Widelands::Economy::TargetQuantity& quantity = get()->target_quantity(index);
			lua_pushinteger(L, quantity.permanent);
		} else {
			report_error(L, "There is no ware '%s'.", wname.c_str());
		}
		break;
	}
	case Widelands::wwWORKER: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().worker_index(wname);
		if (get_egbase(L).descriptions().worker_exists(index)) {
			const Widelands::Economy::TargetQuantity& quantity = get()->target_quantity(index);
			lua_pushinteger(L, quantity.permanent);
		} else {
			report_error(L, "There is no worker '%s'.", wname.c_str());
		}
		break;
	}
	default:
		NEVER_HERE();
	}
	return 1;
}

/* RST
   .. method:: set_target_quantity(name, amount)

      Sets the amount of the given ware or worker type that should be kept in stock for this
      economy. Whether this works only for wares or only for workers is determined by the type of
      this economy.

      **Warning**: Since economies can disappear when a player merges them
      through placing/deleting roads and flags, you must get a fresh economy
      object every time you use this function.

      :arg workername: The name of the worker type.
      :type workername: :class:`string`

      :arg amount: The new target amount for the worker. Needs to be ``>=0``.
      :type amount: :class:`integer`
*/
int LuaEconomy::set_target_quantity(lua_State* L) {
	const std::string wname = luaL_checkstring(L, 2);
	switch (get()->type()) {
	case Widelands::wwWARE: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().ware_index(wname);
		if (get_egbase(L).descriptions().ware_exists(index)) {
			const int quantity = luaL_checkinteger(L, 3);
			if (quantity < 0) {
				report_error(L, "Target ware quantity needs to be >= 0 but was '%d'.", quantity);
			}
			get()->set_target_quantity(get()->type(), index, quantity, get_egbase(L).get_gametime());
		} else {
			report_error(L, "There is no ware '%s'.", wname.c_str());
		}
		break;
	}
	case Widelands::wwWORKER: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().worker_index(wname);
		if (get_egbase(L).descriptions().worker_exists(index)) {
			const int quantity = luaL_checkinteger(L, 3);
			if (quantity < 0) {
				report_error(L, "Target worker quantity needs to be >= 0 but was '%d'.", quantity);
			}
			get()->set_target_quantity(get()->type(), index, quantity, get_egbase(L).get_gametime());
		} else {
			report_error(L, "There is no worker '%s'.", wname.c_str());
		}
		break;
	}
	default:
		NEVER_HERE();
	}
	return 0;
}

/* RST
   .. method:: needs(name[, flag = nil])

      .. versionchanged:: 1.3
         Added parameter ``flag``.

      Check whether the economy's stock of the given
      ware or worker is lower than the target setting.

      If a flag is provided, only consider the flag's district, otherwise the entire economy.

      **Warning**: Since economies can disappear when a player merges them
      through placing/deleting roads and flags, you must get a fresh economy
      object every time you use this function.

      :arg name: The name of the ware or worker.
      :type name: :class:`string`
      :arg flag: The flag whose district to query.
      :type flag: :class:`wl.map.Flag` or :const:`nil`.
      :returns: :class:`boolean`
*/
int LuaEconomy::needs(lua_State* L) {
	const std::string wname = luaL_checkstring(L, 2);
	Widelands::Flag* flag = nullptr;
	if (lua_gettop(L) > 2) {
		flag = (*get_user_class<LuaMaps::LuaFlag>(L, 3))->get(L, get_egbase(L));
		if (flag->get_economy(get()->type()) != get()) {
			report_error(L, "Flag does not belong to this economy.");
		}
	}

	switch (get()->type()) {
	case Widelands::wwWARE: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().ware_index(wname);
		if (get_egbase(L).descriptions().ware_exists(index)) {
			lua_pushboolean(L, static_cast<int>(get()->needs_ware_or_worker(index, flag)));
		} else {
			report_error(L, "There is no ware '%s'.", wname.c_str());
		}
		break;
	}
	case Widelands::wwWORKER: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().worker_index(wname);
		if (get_egbase(L).descriptions().worker_exists(index)) {
			lua_pushboolean(L, static_cast<int>(get()->needs_ware_or_worker(index, flag)));
		} else {
			report_error(L, "There is no worker '%s'.", wname.c_str());
		}
		break;
	}
	default:
		NEVER_HERE();
	}
	return 1;
}

}  // namespace LuaMaps
