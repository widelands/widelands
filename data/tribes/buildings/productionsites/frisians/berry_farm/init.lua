dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_berry_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Berry Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 2,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {40, 76},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {40, 64},
      },
   },

   aihints = {
      supports_production_of = { "fruit", "honey" },
      space_consumer = true,
      prohibited_till = 460
   },

   working_positions = {
      frisians_berry_farmer = 1
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting bushes because ...
         descname = _"planting bushes",
         actions = {
            "sleep=21000",
            "worker=plant"
         }
      },
   },
}
