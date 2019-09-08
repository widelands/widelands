dirname = path.dirname (__file__)

tribes:new_warehouse_type {
   msgctxt = "frisians_building",
   name = "frisians_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Warehouse"),
   helptext_script = dirname .. "helptexts.lua",
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

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 82},
         fps = 10,
      },
      build = {
         pictures = path.list_files (dirname .. "build_?.png"),
         hotspot = {56, 82},
      },
   },

   aihints = {},

   heal_per_second = 170,
}
