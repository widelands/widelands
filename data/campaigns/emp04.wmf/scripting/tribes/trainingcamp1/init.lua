dirname = "tribes/buildings/trainingsites/empire/trainingcamp/"

push_textdomain("scenario_emp04.wmf")

descriptions:new_trainingsite_type {
   name = "empire_trainingcamp1",
   descname = pgettext("empire_building", "Training Camp"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 3,
      granite = 6,
      planks = 6,
      marble = 6,
      gold = 5,
      marble_column = 5,
      cloth = 4
   },

   return_on_dismantle = {
      granite = 3,
      planks = 2,
      marble = 3,
      marble_column = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 82, 105 }
      }
   },

   aihints = {
      trainingsites_max_percent = 1
   },

   working_positions = {
      empire_trainer = 1
   },

   inputs = {
      { name = "fish", amount = 2 },
      { name = "meat", amount = 2 },
      { name = "empire_bread", amount = 2 },
      { name = "spear", amount = 1 },
      { name = "spear_advanced", amount = 1 },
      { name = "spear_heavy", amount = 1 },
      { name = "spear_war", amount = 1 },
      { name = "armor_helmet", amount = 1 },
      { name = "armor", amount = 1 },
      { name = "armor_chain", amount = 1 },
      { name = "armor_gilded", amount = 1 },
   },

   ["soldier attack"] = {
      food = {
         {"fish", "meat"},
         {"empire_bread"}
      },
      weapons = {
         "spear",
         "spear_advanced",
         "spear_heavy",
         "spear_war"
      }
   },
   ["soldier health"] = {
      food = {
         {"fish", "meat"},
         {"empire_bread"}
      },
      weapons = {
         "armor_helmet",
         "armor",
         "armor_chain",
         "armor_gilded"
      }
   },

   programs = {
      sleep = {
         descname = pgettext("empire_building", "sleeping"),
         actions = {
            "sleep=duration:5s",
            "return=skipped"
         }
      },
      upgrade_soldier_attack_0 = {
         descname = pgettext("empire_building", "upgrading soldier attack from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:attack level:0", -- Fails when aren't any soldier of level 0 attack
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:0", -- Because the soldier can be expelled by the player
            "consume=spear empire_bread fish,meat",
            "train=soldier:attack level:1"
         }
      },
      upgrade_soldier_attack_1 = {
         descname = pgettext("empire_building", "upgrading soldier attack from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:attack level:1",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:1",
            "consume=spear_advanced empire_bread fish,meat",
            "train=soldier:attack level:2"
         }
      },
      upgrade_soldier_attack_2 = {
         descname = pgettext("empire_building", "upgrading soldier attack from level 2 to level 3"),
         actions = {
            "checksoldier=soldier:attack level:2",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:2",
            "consume=spear_heavy empire_bread fish,meat:2",
            "train=soldier:attack level:3"
         }
      },
      upgrade_soldier_attack_3 = {
         descname = pgettext("empire_building", "upgrading soldier attack from level 3 to level 4"),
         actions = {
            "checksoldier=soldier:attack level:3",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:3",
            "consume=spear_war empire_bread:2 fish,meat",
            "train=soldier:attack level:4"
         }
      },
      upgrade_soldier_health_0 = {
         descname = pgettext("empire_building", "upgrading soldier health from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:health level:0",
            "sleep=duration:30s",
            "checksoldier=soldier:health level:0",
            "consume=armor_helmet empire_bread,fish,meat",
            "train=soldier:health level:1"
         }
      },
      upgrade_soldier_health_1 = {
         descname = pgettext("empire_building", "upgrading soldier health from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:health level:1",
            "sleep=duration:30s",
            "checksoldier=soldier:health level:1",
            "consume=armor empire_bread fish,meat",
            "train=soldier:health level:2"
         }
      },
      upgrade_soldier_health_2 = {
         descname = pgettext("empire_building", "upgrading soldier health from level 2 to level 3"),
         actions = {
            "checksoldier=soldier:health level:2",
            "sleep=duration:30s",
            "checksoldier=soldier:health level:2",
            "consume=armor_chain empire_bread:2 fish,meat:2",
            "train=soldier:health level:3"
         }
      },
      upgrade_soldier_health_3 = {
         descname = pgettext("empire_building", "upgrading soldier health from level 3 to level 4"),
         actions = {
            "checksoldier=soldier:health level:3",
            "sleep=duration:30s",
            "checksoldier=soldier:health level:3",
            "consume=armor_gilded empire_bread:2 fish,meat:2",
            "train=soldier:health level:4"
         }
      },
   },

   soldier_capacity = 2,
   trainer_patience = 12,

   messages = {
      -- TRANSLATORS: Empire training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("empire_building", "No soldier to train!"),
      -- TRANSLATORS: Empire training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("empire_building", "No soldier found for this training level!"),
   },
}
pop_textdomain()
