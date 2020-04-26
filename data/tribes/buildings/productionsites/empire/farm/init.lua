dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 2,
      granite = 2,
      marble = 2,
      marble_column = 2
   },
   return_on_dismantle = {
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

   indicate_workarea_overlaps = {
      empire_farm = false,
      empire_vineyard = false,
      empire_foresters_house = false,
   },

   aihints = {
      basic_amount = 1,
      space_consumer = true,
      prohibited_till = 510,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
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
            "callworker=plant",
            "sleep=10000"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting wheat because ...
         descname = _"harvesting wheat",
         actions = {
            "callworker=harvest",
            "sleep=4000"
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
