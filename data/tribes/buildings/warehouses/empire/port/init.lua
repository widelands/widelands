push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_warehouse_type {
   name = "empire_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Port"),
   icon = dirname .. "menu.png",
   size = "port",
   map_check = {"seafaring"},

   buildcost = {
      log = 3,
      planks = 4,
      granite = 4,
      marble = 2,
      marble_column = 1,
      cloth = 3,
      gold = 2
   },
   return_on_dismantle = {
      log = 1,
      planks = 1,
      granite = 2,
      marble = 2,
      cloth = 1,
      gold = 1
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         columns = 10,
         rows = 2,
         hotspot = { 87, 116 }
      },
      build = {
         frames = 4,
         columns = 4,
         rows = 1,
         hotspot = { 87, 116 }
      },
   },

   aihints = {
      prohibited_till = 1000
   },

   conquers = 5,
   heal_per_second = 170,
}

pop_textdomain()
