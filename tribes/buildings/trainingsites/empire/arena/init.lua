dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "empire_building",
   name = "empire_arena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Arena"),
   helptext_script = dirname .. "helptexts.lua",
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
         template = "idle_??",
         directory = dirname,
         hotspot = { 81, 82 }
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 82, 83 },
         fps = 1
      }
   },

   aihints = {
      trainingsite_type = "basic",
      prohibited_till = 900,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_trainer = 1
   },

   inputs = {
      empire_bread = 10,
      fish = 6,
      meat = 6
   },
   outputs = {
      "empire_soldier",
   },

   ["soldier evade"] = {
      min_level = 0,
      max_level = 0,
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
            "sleep=5000",
            "check_soldier=soldier attack 9", -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_evade_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier evade from level 0 to level 1",
         actions = {
            "check_soldier=soldier evade 0", -- Fails when aren't any soldier of level 0 evade
            "sleep=30000",
            "check_soldier=soldier evade 0", -- Because the soldier can be expelled by the player
            "consume=empire_bread:2 fish,meat",
            "train=soldier evade 0 1"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 8
}
