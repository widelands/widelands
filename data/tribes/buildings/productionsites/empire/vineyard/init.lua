dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_vineyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Vineyard"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      planks = 2,
      granite = 1,
      marble = 2,
      marble_column = 2
   },
   return_on_dismantle = {
      planks = 1,
      marble = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 53 },
      },
   },

   aihints = {
      space_consumer = true,
      forced_after = 300
   },

   working_positions = {
      empire_vinefarmer = 1
   },

   outputs = {
      "grape"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant_vine",
            "call=harvest_vine",
            "return=skipped"
         }
      },
      plant_vine = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting grapevines because ...
         descname = _"planting grapevines",
         actions = {
            "sleep=20000",
            "worker=plantvine"
         }
      },
      harvest_vine = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting grapevines because ...
         descname = _"harvesting grapes",
         actions = {
            "sleep=5000",
            "worker=harvestvine"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fields",
      heading = _"Out of Fields",
      message = pgettext("empire_building", "The vine farmer working at this vineyard has no cleared soil to plant his grapevines."),
      productivity_threshold = 30
   },
}
