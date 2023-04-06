push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_warehouse_type {
   name = "frisians_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Port"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "port",
   map_check = {"seafaring"},

   buildcost = {
      brick = 6,
      granite = 4,
      log = 6,
      cloth = 2,
      gold = 2,
      reed = 4
   },
   return_on_dismantle = {
      brick = 3,
      granite = 2,
      log = 3,
      gold = 1,
      cloth = 1,
      reed = 2
   },

   spritesheets = {
      idle = {
         hotspot = {64, 117},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      build = {
         hotspot = {64, 117}
      }
   },

   aihints = {
      prohibited_till = 1000
   },

   conquers = 5,
   heal_per_second = 170,
}

pop_textdomain()
