dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "empire_building",
   name = "empire_colosseum",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Colosseum"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   enhancement_cost = {
      planks = 2,
      granite = 4,
      marble = 4,
      cloth = 2,
      gold = 4,
      marble_column = 4
   },
   return_on_dismantle_on_enhanced = {
      planks = 1,
      granite = 2,
      marble = 2,
      gold = 2,
      marble_column = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 81, 106 }
      }
   },

   aihints = {
      prohibited_till = 1200,
      trainingsite_type = "basic",
      forced_after = 1800,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_trainer = 1
   },

   -- This table is nested so we can define the order in the building's UI.
   inputs = {
      { name = "fish", amount = 6 },
      { name = "meat", amount = 6 },
      { name = "empire_bread", amount = 10 }
   },
   outputs = {
      "empire_soldier",
   },

   ["soldier evade"] = {
      min_level = 0,
      max_level = 1,
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
      upgrade_soldier_evade_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier evade from level 1 to level 2",
         actions = {
            "check_soldier=soldier evade 1", -- Fails when aren't any soldier of level 1 evade
            "sleep=30000",
            "check_soldier=soldier evade 1", -- Because the soldier can be expelled by the player
            "consume=empire_bread:2 fish,meat:2",
            "train=soldier evade 1 2"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 9
}
