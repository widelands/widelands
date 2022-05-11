push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_trainingsite_type {
   name = "empire_arena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Arena"),
   icon = dirname .. "menu.png",
   size = "big",

   enhancement = {
      name = "empire_colosseum",
      enhancement_cost = {
         planks = 2,
         granite = 4,
         marble = 4,
         cloth = 2,
         gold = 4,
         marble_column = 4
      },
      enhancement_return_on_dismantle = {
         planks = 1,
         granite = 2,
         marble = 2,
         gold = 2,
         marble_column = 2
      }
   },

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

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 91, 89 }
      },
      build = {
         frames = 4,
         columns = 4,
         rows = 1,
         hotspot = { 91, 89 }
      },
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
         descname = _("sleeping"),
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
   trainer_patience = 16,

   messages = {
      -- TRANSLATORS: Empire training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("empire_building", "No soldier to train!"),
      -- TRANSLATORS: Empire training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("empire_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
