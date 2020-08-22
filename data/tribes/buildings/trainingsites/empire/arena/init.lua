dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "empire_building",
   name = "empire_arena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Arena"),
   icon = dirname .. "menu.png",
   size = "big",
   enhancement = "empire_colosseum",

   buildcost = {
      log = 2,
      granite = 4,
      marble = 5,
      planks = 5,
      marble_column = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 3,
      marble = 3,
      planks = 2,
      marble_column = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 81, 82 }
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 82, 83 },
      }
   },

   aihints = {
      trainingsites_max_percent = 10,
      prohibited_till = 900,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_trainer = 1
   },

   inputs = {
      { name = "fish", amount = 6 },
      { name = "meat", amount = 6 },
      { name = "empire_bread", amount = 10 }
   },

   ["soldier evade"] = {
      food = {
         {"fish", "meat"},
         {"empire_bread"}
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
         descname = pgettext("empire_building", "upgrading soldier evade from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:evade level:0", -- Fails when aren't any soldier of level 0 evade
            "return=failed unless site has empire_bread",
            "return=failed unless site has fish,meat",
            "sleep=duration:30s",
            "checksoldier=soldier:evade level:0", -- Because the soldier can be expelled by the player
            "consume=empire_bread fish,meat",
            "train=soldier:evade level:1"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 8
}
