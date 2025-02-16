-- RST
-- .. _lua_tribes_buildings_markets:
--
-- Markets
-- -------
--
-- Markets are used to trade wares with other tribes.
--
-- Markets are defined in
-- ``data/tribes/buildings/markets/<tribe_name>/<building_name>/init.lua``.
-- The market will also need its :ref:`help texts <lua_tribes_tribes_helptexts>`,
-- which are defined in ``data/tribes/initialization/<tribe_name>/units.lua``
--
-- .. function:: new_market_type{table}
--
--    This function adds the definition of a market building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                In addition to the :ref:`lua_tribes_buildings_common`, it contains the following entries:
--
--    **local_carrier**
--        *Mandatory*. The name of the worker who works in the building.
--
--    **trade_carrier**
--        *Mandatory*. The name of the worker that carries traded wares across the map.
--
-- For making the UI texts translateable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
--    push_textdomain("tribes")
--
--    local dirname = path.dirname (__file__)
--
--    wl.Descriptions():new_market_type {
--       name = "atlanteans_market",
--       descname = pgettext("atlanteans_building", "Market"),
--       animation_directory = dirname,
--       icon = dirname .. "menu.png",
--       size = "big",
--
--       buildcost = {
--          log = 2,
--          planks = 2,
--          granite = 2,
--          quartz = 1,
--          spidercloth = 1
--       },
--       return_on_dismantle = {
--          log = 1,
--          planks = 1,
--          granite = 1,
--          quartz = 1
--       },
--
--       local_carrier = "atlanteans_carrier",
--       trade_carrier = "atlanteans_horse",
--
--       animation_directory = dirname,
--       animations = {
--          idle = {
--             hotspot = { 63, 68 }
--          }
--       },
--
--       aihints = {
--          prohibited_till = 1000,
--          forced_after = 1500,
--       },
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_market_type {
   name = "atlanteans_market",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Market"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      planks = 2,
      granite = 2,
      quartz = 1,
      spidercloth = 1
   },
   return_on_dismantle = {
      log = 1,
      planks = 1,
      granite = 1,
      quartz = 1
   },

   local_carrier = "atlanteans_carrier",
   trade_carrier = "atlanteans_horse",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 63, 68 }
      }
   },

   aihints = {
      prohibited_till = 1000,
      forced_after = 1500,
   },
}

pop_textdomain()
