-- RST
-- .. _lua_tribes_buildings_markets:
--
-- Markets
-- -------
--
-- TODO(sirver, trading): Add useful documentation here

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
      prohibited_till = 1000
   },
}

pop_textdomain()
