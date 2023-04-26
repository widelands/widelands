-- RST
-- .. _lua_tribes_buildings_markets:
--
-- Markets
-- -------
--
-- TODO(sirver, trading): Add useful documentation here
dirname = "test/maps/market_trading.wmf/" .. path.dirname(__file__)

wl.Descriptions():new_market_type {
   name = "barbarians_market",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Market"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 8,
      blackwood = 6,
      granite = 4,
      grout = 3,
      iron = 1,
      reed = 4,
      gold = 2,
   },
   return_on_dismantle = {
      log = 4,
      blackwood = 3,
      granite = 3,
      grout = 1,
      iron = 1,
      reed = 2,
      gold = 1,
   },

   animations = {
      idle = {
         hotspot = { 67, 80 },
      },
      build = {
         hotspot = { 67, 80 },
      }
   },

   aihints = {
      prohibited_till = 1000
   },

   carrier = "barbarians_ox",
}
