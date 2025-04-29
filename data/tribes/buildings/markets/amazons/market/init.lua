push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_market_type {
   name = "amazons_market",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Market"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      balsa = 3,
      granite = 2,
      quartz = 1,
      log = 4,
      rubber = 3,
      rope = 3
   },
   return_on_dismantle = {
      balsa = 1,
      granite = 1,
      quartz = 1,
      log = 2,
      rubber = 1,
      rope = 1
   },

   local_carrier = "amazons_carrier",
   trade_carrier = "amazons_tapir",

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {92, 107}},
   },
   spritesheets = {
      build = {
         hotspot = {92, 107},
         frames = 3,
         columns = 3,
         rows = 1
      },
   },

   aihints = {
      prohibited_till = 1000,
      forced_after = 1500,
   },
}

pop_textdomain()
