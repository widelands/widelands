push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_market_type {
   name = "empire_market",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Market"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      planks = 2,
      granite = 2,
      marble = 3,
      marble_column = 2
   },
   return_on_dismantle = {
      planks = 1,
      granite = 2,
      marble = 2,
      marble_column = 1
   },

   local_carrier = "empire_carrier",
   trade_carrier = "empire_donkey",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 58, 58 }
      }
   },

   aihints = {
      prohibited_till = 1000,
      forced_after = 1500,
   },
}

pop_textdomain()
