push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "frisians_berry_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Berry Farm"),
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
      space_consumer = true,
      prohibited_till = 460
   },

   working_positions = {
      frisians_berry_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting bushes because ...
         descname = _("planting bushes"),
         actions = {
            "callworker=plant",
            "sleep=duration:21s"
         }
      },
   },
}

pop_textdomain()
