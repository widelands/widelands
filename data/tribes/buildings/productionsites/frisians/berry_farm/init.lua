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
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {49, 90},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {49, 68},
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

   indicate_workarea_overlaps = {
      frisians_clay_pit = false,
      frisians_berry_farm = false,
      frisians_reed_farm = false,
      frisians_farm = false,
      frisians_foresters_house = false,
      frisians_beekeepers_house = true,
      frisians_collectors_house = true,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting bushes because ...
         descname = _"planting bushes",
         actions = {
            "callworker=plant",
            "sleep=21000"
         }
      },
   },
}
