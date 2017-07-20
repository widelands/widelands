dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_reed_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Reed Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 5,
      granite = 1,
      log = 1,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 62, 58 },
      },
   },

   aihints = {
      space_consumer = true,
      prohibited_till = 200,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 1
   },

   working_positions = {
      frisians_reed_farmer = 1
   },

   outputs = {
      "thatch_reed"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant_reed",
            "call=harvest_reed",
            "return=skipped"
         }
      },
      plant_reed = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting reed because ...
         descname = _"planting reed",
         actions = {
            "sleep=16000",
            "worker=plantreed"
         }
      },
      harvest_reed = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting reed because ...
         descname = _"harvesting reed",
         actions = {
            "sleep=6000",
            "worker=harvestreed"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fields",
      heading = _"Out of Fields",
      message = pgettext("frisians_building", "The farmer working at this reed farm has no cleared soil to plant his seeds."),
      productivity_threshold = 20
   },
}
