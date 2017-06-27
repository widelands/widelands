dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_reindeer_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Reindeer Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 8,
      granite = 8,
      log = 7,
      thatch_reed = 4
   },
   return_on_dismantle = {
      brick = 4,
      granite = 4,
      log = 3,
      thatch_reed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 80 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 80 },
      },
   },

   aihints = {
      recruitment = true
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
      "fur"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start rearing cattle because ...
         descname = pgettext("frisians_building", "working"),
         actions = {
            "call=make_fur",
            "call=recruit_deer",
            "return=skipped",
         }
      },
      recruit_deer = {
         -- TRANSLATORS: Completed/Skipped/Did not start rearing cattle because ...
         descname = pgettext("frisians_building", "rearing reindeer"),
         actions = {
            "sleep=5000",
            "return=skipped unless economy needs frisians_reindeer",
            "consume=barley water",
            "animate=working 25000",
            "recruit=frisians_reindeer"
         }
      },
      make_fur = {
         -- TRANSLATORS: Completed/Skipped/Did not start rearing cattle because ...
         descname = pgettext("frisians_building", "producing fur"),
         actions = {
            "sleep=5000",
            "return=skipped unless economy needs fur",
            "consume=barley water",
            "animate=working 30000",
            "produce=fur"
         }
      },
   },
}
