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

#include "scripting/map/lua_market.h"

#include "logic/player.h"
#include "scripting/globals.h"
#include "scripting/lua_game.h"

namespace LuaMaps {

/* RST
Market
---------

.. class:: Market

   .. versionadded:: 1.3

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
   METHOD(LuaMarket, accept_trade),
   // TODO(sirver,trading): Implement and fix documentation.
   // METHOD(LuaMarket, set_wares),
   // METHOD(LuaMarket, get_wares),
   // METHOD(LuaMarket, set_workers),
   // METHOD(LuaMarket, get_workers),
   {nullptr, nullptr},
};
const PropertyType<LuaMarket> LuaMarket::Properties[] = {
   PROP_RW(LuaMarket, marketname),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: marketname

   (RW) The name of the market as :class:`string`.
*/
int LuaMarket::get_marketname(lua_State* L) {
	Widelands::Market* market = get(L, get_egbase(L));
	lua_pushstring(L, market->get_market_name().c_str());
	return 1;
}
int LuaMarket::set_marketname(lua_State* L) {
	Widelands::Market* market = get(L, get_egbase(L));
	market->set_market_name(luaL_checkstring(L, -1));
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. method:: propose_trade(player, num_batches, items_to_send, items_to_receive)

      Propose a trade from this market to another player.

      :arg player: The player to make the trade offer to.
      :type player: :class:`wl.game.Player`
      :arg num_batches: Total number of trading batches to send.
      :type num_batches: :class:`integer`
      :arg items_to_send: A table of warename to amount of items to send in each batch.
      :type items_to_send: :class:`table`
      :arg items_to_receive: A table of warename to amount of items to receive in each batch.
      :type items_to_receive: :class:`table`

      :returns: The unique ID for the new trade offer.
      :rtype: :class:`integer`
*/
int LuaMarket::propose_trade(lua_State* L) {
	if (lua_gettop(L) != 5) {
		report_error(L, "Takes 4 arguments.");
	}

	Widelands::TradeInstance trade;
	Widelands::Game& game = get_game(L);
	Widelands::Market* self = get(L, game);

	trade.initiator = self;
	trade.sending_player = self->owner().player_number();
	trade.receiving_player =
	   (*get_user_class<LuaGame::LuaPlayer>(L, 2))->get(L, game).player_number();

	trade.num_batches = luaL_checkinteger(L, 3);
	trade.items_to_send = parse_wares_as_bill_of_material(L, 4, self->owner().tribe());
	trade.items_to_receive = parse_wares_as_bill_of_material(L, 5, self->owner().tribe());

	const Widelands::TradeID trade_id = game.propose_trade(trade);
	lua_pushint32(L, trade_id);
	return 1;
}

/* RST
   .. method:: accept_trade(id)

      Accept the proposed trade with the provided ID.

      Only proposed trade offers can be accepted.
      Only the recipient of the offer may accept it.
      The offer can be accepted by any market with a land connection to the initiating market.

      :arg id: Unique ID of the trade to accept.
      :type id: :class:`integer`

      :see also: :attr:`wl.Game.trades`
*/
int LuaMarket::accept_trade(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes one argument.");
	}
	Widelands::Game& game = get_game(L);
	game.accept_trade(luaL_checkinteger(L, 2), *get(L, game));
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

}  // namespace LuaMaps
