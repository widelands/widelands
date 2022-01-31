push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "frisians_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 3,
      granite = 1,
      log = 2,
      reed = 3
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {91, 111},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {91, 111},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      build = {
         directory = dirname,
         basename = "build",
         hotspot = {91, 90},
         frames = 2,
         columns = 2,
         rows = 1
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {91, 90}
      }
   },

   aihints = {
      space_consumer = true,
      prohibited_till = 220,
   },

   working_positions = {
      frisians_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=plant_barley",
            "call=harvest_barley",
         }
      },
      plant_barley = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting barley because ...
         descname = _("planting barley"),
         actions = {
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest_barley = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting barley because ...
         descname = _("harvesting barley"),
         actions = {
            "callworker=harvest",
            "animate=working duration:40s",
            "sleep=duration:4s",
            "produce=barley" --produces 2 barley per field
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext ("frisians_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}

pop_textdomain()
