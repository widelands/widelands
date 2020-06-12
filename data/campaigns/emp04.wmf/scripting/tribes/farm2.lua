dirname = "tribes/buildings/productionsites/empire/farm/"

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_farm2",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   enhancement_cost = {
      planks = 1,
      marble = 1,
      marble_column = 2
   },
   return_on_dismantle_on_enhanced = {
      planks = 1,
      granite = 1,
      marble = 1,
      marble_column = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 82, 74 },
      },
   },

   aihints = {
   },

   working_positions = {
      empire_farmer = 1
   },

   outputs = {
      "wheat"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting wheat because ...
         descname = _"planting wheat",
         actions = {
            "sleep=14000",
            "callworker=plant"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting wheat because ...
         descname = _"harvesting wheat",
         actions = {
            "sleep=4000",
            "callworker=harvest"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fields",
      heading = _"Out of Fields",
      message = pgettext("empire_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}
