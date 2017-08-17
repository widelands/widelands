dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_battlearena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Battle Arena"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 6,
      granite = 4,
      grout = 6,
      gold = 4,
      thatch_reed = 3
   },
   return_on_dismantle = {
      log = 3,
      granite = 3,
      grout = 3,
      gold = 2,
      thatch_reed = 1
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
      prohibited_till = 900,
      forced_after = 1500,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      barbarians_trainer = 1
   },

   inputs = {
      { name = "fish", amount = 6 },
      { name = "meat", amount = 6 },
      { name = "barbarians_bread", amount = 10 },
      { name = "beer_strong", amount = 6 }
   },
   outputs = {
      "barbarians_soldier",
   },

   ["soldier evade"] = {
      min_level = 0,
      max_level = 1,
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
            "sleep=5000",
            "check_soldier=soldier attack 9", -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_evade_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier evade from level 0 to level 1"),
         actions = {
            "check_soldier=soldier evade 0", -- Fails when aren't any soldier of level 0 evade
            "sleep=15000",
            "check_soldier=soldier evade 0", -- Because the soldier can be expelled by the player
            "consume=barbarians_bread fish,meat beer_strong",
            "animate=working 15000",
            "train=soldier evade 0 1"
         }
      },
      upgrade_soldier_evade_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier evade from level 1 to level 2"),
         actions = {
            "check_soldier=soldier evade 1", -- Fails when aren't any soldier of level 1 evade
            "sleep=15000",
            "check_soldier=soldier evade 1", -- Because the soldier can be expulsed by the player
            "consume=barbarians_bread:2 fish,meat beer_strong",
            "animate=working 15000",
            "train=soldier evade 1 2"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 3
}
