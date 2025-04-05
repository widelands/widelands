push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_market_type {
   name = "frisians_market",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Market"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 6,
      granite = 5,
      log = 6,
      reed = 5
   },
   return_on_dismantle = {
      brick = 3,
      granite = 3,
      log = 3,
      reed = 2
   },

   local_carrier = "frisians_carrier",
   trade_carrier = "frisians_reindeer",

   spritesheets = {
      idle = {
         hotspot = {90, 95},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      build = {
         hotspot = {90, 95},
         frames = 3,
         columns = 3,
         rows = 1
      }
   },

   aihints = {
      prohibited_till = 1000,
      forced_after = 1500,
   },
}

pop_textdomain()
