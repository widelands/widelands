dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_cassava_root_plantation",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Cassava Root Plantation"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 4,
      rope = 2,
      granite = 2,
   },
   return_on_dismantle = {
      log = 2,
      rope = 1,
      granite = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {105, 138},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {105, 138},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {105, 111},
      },
      build = {
         pictures = path.list_files (dirname .. "build_?.png"),
         hotspot = {105, 111},
      },
   },

   aihints = {
      space_consumer = true,
      prohibited_till = 220,
   },

   working_positions = {
      amazons_cassava_farmer = 1
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant_cassava",
            "call=harvest_cassava",
         }
      },
      plant_cassava = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting barley because ...
         descname = _"planting cassava root",
         actions = {
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest_cassava = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting barley because ...
         descname = _"harvesting cassava root",
         actions = {
            "callworker=harvest",
            "sleep=4000",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fields",
      heading = _"Out of Fields",
      message = pgettext ("amazons_building", "The farmer working at this cassava root plantation has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}
