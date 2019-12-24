dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_reindeer_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Reindeer Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 1,
      granite = 2,
      log = 2,
      reed = 2
   },
   return_on_dismantle = {
      brick = 1,
      granite = 1,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {84, 81},
         frames = 20,
         columns = 5,
         rows = 4,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {84, 81},
         frames = 20,
         columns = 5,
         rows = 4,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {84, 59}
      }
   },

   aihints = {
      prohibited_till = 1200
   },

   working_positions = {
      frisians_reindeer_breeder = 1
   },

   inputs = {
      { name = "barley", amount = 8 },
      { name = "water", amount = 8 }
   },
   outputs = {
      "frisians_reindeer",
      "fur",
      "meat",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=recruit_deer",
            "call=make_fur",
            "call=recruit_deer",
            "call=make_fur",
            "call=recruit_deer",
            "call=make_fur_meat",
            "return=no_stats",
         }
      },
      recruit_deer = {
         -- TRANSLATORS: Completed/Skipped/Did not start rearing reindeer because ...
         descname = pgettext("frisians_building", "rearing reindeer"),
         actions = {
            "return=skipped unless economy needs frisians_reindeer",
            "consume=barley water",
            "sleep=15000",
            "animate=working 15000",
            "recruit=frisians_reindeer"
         }
      },
      make_fur = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing fur because ...
         descname = pgettext("frisians_building", "producing fur"),
         actions = {
            "return=skipped unless economy needs fur",
            "consume=barley water",
            "sleep=15000",
            "animate=working 20000",
            "produce=fur"
         }
      },
      make_fur_meat = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing fur because ...
         descname = pgettext("frisians_building", "producing fur"),
         actions = {
            "return=skipped unless economy needs fur",
            "consume=barley water",
            "sleep=15000",
            "animate=working 20000",
            "produce=fur meat"
         }
      },
   },
}
