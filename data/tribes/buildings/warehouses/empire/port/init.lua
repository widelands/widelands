push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_warehouse_type {
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

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 74, 96 },
         fps = 10
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 74, 96 },
      }
   },

   aihints = {
      prohibited_till = 1000
   },

   conquers = 5,
   heal_per_second = 170,
}

pop_textdomain()
