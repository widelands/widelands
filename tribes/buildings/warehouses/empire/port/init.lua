dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   msgctxt = "empire_building",
   name = "empire_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Port"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "port",

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
         template = "idle_??",
         directory = dirname,
         hotspot = { 74, 96 },
         fps = 10
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 74, 96 },
         fps = 1
      }
   },

   aihints = {
      prohibited_till = 900
   },

   conquers = 5,
   heal_per_second = 170,
}
