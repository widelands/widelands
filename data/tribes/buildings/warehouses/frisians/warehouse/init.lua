push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_warehouse_type {
   name = "frisians_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Warehouse"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 5,
      granite = 2,
      log = 2,
      reed = 3
   },
   return_on_dismantle = {
      brick = 3,
      granite = 1,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {50, 69},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      build = {
         hotspot = {50, 69}
      }
   },

   aihints = {},

   heal_per_second = 170,
}

pop_textdomain()
