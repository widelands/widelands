push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_warehouse_type {
   name = "atlanteans_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Port"),
   icon = dirname .. "menu.png",
   size = "port",
   map_check = {"seafaring"},

   buildcost = {
      log = 3,
      planks = 3,
      granite = 4,
      diamond = 1,
      quartz = 1,
      spidercloth = 3,
      gold = 2
   },
   return_on_dismantle = {
      log = 1,
      planks = 1,
      granite = 2,
      spidercloth = 1,
      gold = 1
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         columns = 10,
         rows = 2,
         hotspot = { 98, 98 }
      },
      build = {
         frames = 4,
         columns = 4,
         rows = 1,
         hotspot = { 98, 98 }
      },
   },

   aihints = {
      prohibited_till = 1000
   },

   conquers = 5,
   heal_per_second = 170,
}

pop_textdomain()
