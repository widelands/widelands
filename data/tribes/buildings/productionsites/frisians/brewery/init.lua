push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "frisians_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Brewery"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "frisians_mead_brewery",
      enhancement_cost = {
         brick = 3,
         granite = 2,
         log = 1,
         reed = 1
      },
      enhancement_return_on_dismantle = {
         brick = 2,
         granite = 1
      }
   },

   buildcost = {
      brick = 3,
      log = 2,
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 80},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {50, 80},
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
         hotspot = {50, 60}
      }
   },

   aihints = {
      prohibited_till = 530,
      forced_after = 720,
   },

   working_positions = {
      frisians_brewer = 1
   },

   inputs = {
      { name = "barley", amount = 7 },
      { name = "water", amount = 7 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _("brewing beer"),
         actions = {
            "return=skipped unless economy needs beer or workers need experience",
            "consume=water barley",
            "sleep=duration:30s",
            "animate=working duration:30s",
            "produce=beer"
         },
      },
   },
}

pop_textdomain()
