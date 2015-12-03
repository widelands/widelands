dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_blackroot_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Blackroot Farm"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 3,
      granite = 2,
      log = 4
   },
   return_on_dismantle = {
      planks = 1,
      granite = 2,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 74, 60 },
      },
   },

   aihints = {
      prohibited_till = 600,
      forced_after = 800,
      space_consumer = true
   },

   working_positions = {
      atlanteans_blackroot_farmer = 1
   },

   outputs = {
      "blackroot"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant_blackroot",
            "call=harvest_blackroot",
            "return=skipped"
         }
      },
      plant_blackroot = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting blackroot because ...
         descname = _"planting blackroot",
         actions = {
            "sleep=20000",
            "worker=plant"
         }
      },
      harvest_blackroot = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting blackroot because ...
         descname = _"harvesting blackroot",
         actions = {
            "sleep=5000",
            "worker=harvest"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fields",
      heading = _"Out of Fields",
      message = pgettext("atlanteans_building", "The blackroot farmer working at this blackroot farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}
