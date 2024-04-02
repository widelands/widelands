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

#include "scripting/map/lua_market.h"

#include "logic/player.h"
#include "scripting/globals.h"

namespace LuaMaps {

// TODO(kaputtnik): Readd RST once this get implemented
/*
Market
---------

.. class:: Market

   A Market used for trading with other players.

   For functions see:
      * :ref:`has_wares`
      * :ref:`has_workers`

   More properties are available through this object's
   :class:`MarketDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaMarket::className[] = "Market";
const MethodType<LuaMarket> LuaMarket::Methods[] = {
   METHOD(LuaMarket, propose_trade),
   // TODO(sirver,trading): Implement and fix documentation.
   // METHOD(LuaMarket, set_wares),
   // METHOD(LuaMarket, get_wares),
   // METHOD(LuaMarket, set_workers),
   // METHOD(LuaMarket, get_workers),
   {nullptr, nullptr},
};
const PropertyType<LuaMarket> LuaMarket::Properties[] = {
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// TODO(kaputtnik): Readd RST once this get implemented
/*
   .. method:: propose_trade(other_market, num_batches, items_to_send, items_to_receive)

      TODO(sirver,trading): document

      :returns: :const:`nil`
*/
int LuaMarket::propose_trade(lua_State* L) {
	if (lua_gettop(L) != 5) {
		report_error(L, "Takes 4 arguments.");
	}
	Widelands::Game& game = get_game(L);
	Widelands::Market* self = get(L, game);
	Widelands::Market* other_market = (*get_user_class<LuaMarket>(L, 2))->get(L, game);
	const int num_batches = luaL_checkinteger(L, 3);

	const Widelands::BillOfMaterials items_to_send =
	   parse_wares_as_bill_of_material(L, 4, self->owner().tribe());
	// TODO(sirver,trading): unsure if correct. Test inter-tribe trading, i.e.
	// Barbarians trading with Empire, but shipping Atlantean only wares.
	const Widelands::BillOfMaterials items_to_receive =
	   parse_wares_as_bill_of_material(L, 5, self->owner().tribe());
	const int trade_id = game.propose_trade(Widelands::Trade{
	   items_to_send, items_to_receive, num_batches, self->serial(), other_market->serial()});

	// TODO(sirver,trading): Wrap 'Trade' into its own Lua class?
	lua_pushint32(L, trade_id);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

}  // namespace LuaMaps
