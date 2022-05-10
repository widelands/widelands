push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_trainingsite_type {
   name = "empire_colosseum",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Colosseum"),
   icon = dirname .. "menu.png",
   size = "big",

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 91, 114 }
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
   trainer_patience = 18,

   messages = {
      -- TRANSLATORS: Empire training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("empire_building", "No soldier to train!"),
      -- TRANSLATORS: Empire training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("empire_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
