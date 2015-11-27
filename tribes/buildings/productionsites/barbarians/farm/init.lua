dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Farm"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 4,
      blackwood = 1,
      granite = 3
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 2
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 69, 76 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 69, 76 },
      },
      unoccupied = {
         template = "unoccupied_??",
         directory = dirname,
         hotspot = { 69, 76 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 69, 76 },
      },
   },

   aihints = {
      space_consumer = true,
      forced_after = 600
   },

   working_positions = {
      barbarians_farmer = 1
   },

   outputs = {
      "wheat"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant_wheat",
            "call=harvest_wheat",
            "return=skipped"
         }
      },
      plant_wheat = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting wheat because ...
         descname = _"planting wheat",
         actions = {
            "sleep=14000",
            "worker=plant"
         }
      },
      harvest_wheat = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting wheat because ...
         descname = _"harvesting wheat",
         actions = {
            "sleep=4000",
            "worker=harvest",
            "animate=working 30000"
         }
      },
   },
   out_of_resource_notification = {
      title = _"Out of Fields",
      message = pgettext("barbarians_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}
