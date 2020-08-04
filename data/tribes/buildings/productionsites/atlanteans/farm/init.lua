dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 3,
      granite = 2,
      planks = 2,
      spidercloth = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 1,
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 74, 60 },
      },
   },

   aihints = {
      space_consumer = true,
      basic_amount = 1,
       -- Farm needs spidercloth to be built and spidercloth needs corn for production
       -- -> farm should be built ASAP!
      prohibited_till = 250,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      atlanteans_farmer = 1
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
         -- TRANSLATORS: Completed/Skipped/Did not start planting corn because ...
         descname = _"planting corn",
         actions = {
            "callworker=plant",
            "sleep=10000"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting corn because ...
         descname = _"harvesting corn",
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
      message = pgettext("atlanteans_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}
