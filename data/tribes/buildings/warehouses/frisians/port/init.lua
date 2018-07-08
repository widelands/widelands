dirname = path.dirname (__file__)

tribes:new_warehouse_type {
   msgctxt = "frisians_building",
   name = "frisians_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Port"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "port",
   needs_seafaring = true,

   buildcost = {
      brick = 6,
      granite = 4,
      log = 6,
      cloth = 2,
      gold = 2,
      thatch_reed = 4
   },
   return_on_dismantle = {
      brick = 3,
      granite = 2,
      log = 3,
      gold = 1,
      cloth = 1,
      thatch_reed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {86, 152},
         fps = 10,
      },
      build = {
         pictures = path.list_files (dirname .. "build_?.png"),
         hotspot = {86, 152},
      },
   },

   aihints = {
      prohibited_till = 1000
   },

   conquers = 5,
   heal_per_second = 170,
}
