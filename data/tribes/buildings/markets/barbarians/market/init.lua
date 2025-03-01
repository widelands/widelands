push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_market_type {
   name = "barbarians_market",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Market"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 2,
      grout = 3,
      reed = 1
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 1,
      grout = 1
   },

   local_carrier = "barbarians_carrier",
   trade_carrier = "barbarians_ox",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 107, 102 }
      },
   },
   spritesheets = {
      build = {
         frames = 2,
         columns = 2,
         rows = 1,
         hotspot = { 107, 102 }
      }
   },

   aihints = {
      prohibited_till = 1000,
      forced_after = 1500,
   },
}

pop_textdomain()
