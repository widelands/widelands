push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_warehouse_type {
   name = "frisians_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Headquarters"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   spritesheets = {
      idle = {
         hotspot = {92, 102},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
   max_garrison = 20,
}

pop_textdomain()
