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

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {40, 73},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {40, 55}
      }
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
