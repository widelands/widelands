dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_battlearena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Battle Arena"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 6,
      granite = 4,
      grout = 6,
      gold = 4,
      reed = 3
   },
   return_on_dismantle = {
      log = 3,
      granite = 3,
      grout = 3,
      gold = 2,
      reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 110, 72 },
         fps = 10
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 110, 72 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 110, 72 }
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 110, 72 },
         fps = 1
      }
   },

   aihints = {
      trainingsites_max_percent = 25,
      prohibited_till = 900,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      barbarians_trainer = 1
   },

   inputs = {
      { name = "meat", amount = 6 },
      { name = "fish", amount = 6 },
      { name = "barbarians_bread", amount = 8 },
      { name = "beer_strong", amount = 8 }
   },

   ["soldier evade"] = {
      food = {
         {"fish", "meat"},
         {"beer_strong"},
         {"barbarians_bread"}
      }
   },

   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _"sleeping",
         actions = {
            "sleep=duration:5s",
            "return=skipped",
         }
      },
      upgrade_soldier_evade_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier evade from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:evade level:0", -- Fails when aren't any soldier of level 0 evade
            "return=failed unless site has barbarians_bread",
            "return=failed unless site has fish,meat",
            "return=failed unless site has beer_strong",
            "sleep=duration:15s",
            "animate=working duration:15s",
            "checksoldier=soldier:evade level:0", -- Because the soldier can be expelled by the player
            "consume=barbarians_bread fish,meat beer_strong",
            "train=soldier:evade level:1"
         }
      },
      upgrade_soldier_evade_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier evade from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:evade level:1", -- Fails when aren't any soldier of level 1 evade
            "return=failed unless site has barbarians_bread",
            "return=failed unless site has fish,meat",
            "return=failed unless site has beer_strong",
            "sleep=duration:15s",
            "animate=working duration:15s",
            "checksoldier=soldier:evade level:1", -- Because the soldier can be expulsed by the player
            "consume=barbarians_bread fish,meat beer_strong",
            "train=soldier:evade level:2"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 3
}
