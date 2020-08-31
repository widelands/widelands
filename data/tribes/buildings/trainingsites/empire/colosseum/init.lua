push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   name = "empire_colosseum",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Colosseum"),
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
      trainingsites_max_percent = 25,
      prohibited_till = 1200,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_trainer = 1
   },

   inputs = {
      { name = "fish", amount = 8 },
      { name = "meat", amount = 8 },
      { name = "empire_bread", amount = 8 }
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
      upgrade_soldier_evade_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("empire_building", "upgrading soldier evade from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:evade level:1", -- Fails when aren't any soldier of level 1 evade
            "return=failed unless site has empire_bread",
            "return=failed unless site has fish,meat:2",
            "sleep=duration:30s",
            "checksoldier=soldier:evade level:1", -- Because the soldier can be expelled by the player
            "consume=empire_bread fish,meat:2",
            "train=soldier:evade level:2"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 9
}

pop_textdomain()
