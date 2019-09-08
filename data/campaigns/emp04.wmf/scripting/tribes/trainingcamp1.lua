dirname = "tribes/buildings/trainingsites/empire/trainingcamp/"

tribes:new_trainingsite_type {
   msgctxt = "empire_building",
   name = "empire_trainingcamp1",
   descname = "Training Camp",
   helptext_script = dirname .. "helptexts.lua",
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

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
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
   outputs = {
      "empire_soldier",
   },

   ["soldier attack"] = {
      min_level = 0,
      max_level = 3,
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
      min_level = 0,
      max_level = 3,
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
         descname = "sleeping",
         actions = {
            "sleep=5000",
            "checksoldier=soldier attack 9", -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_attack_0 = {
         descname = "upgrading soldier attack from level 0 to level 1",
         actions = {
            "checksoldier=soldier attack 0", -- Fails when aren't any soldier of level 0 attack
            "sleep=30000",
            "checksoldier=soldier attack 0", -- Because the soldier can be expelled by the player
            "consume=spear empire_bread fish,meat",
            "train=soldier attack 0 1"
         }
      },
      upgrade_soldier_attack_1 = {
         descname = "upgrading soldier attack from level 1 to level 2",
         actions = {
            "checksoldier=soldier attack 1",
            "sleep=30000",
            "checksoldier=soldier attack 1",
            "consume=spear_advanced empire_bread fish,meat",
            "train=soldier attack 1 2"
         }
      },
      upgrade_soldier_attack_2 = {
         descname = "upgrading soldier attack from level 2 to level 3",
         actions = {
            "checksoldier=soldier attack 2",
            "sleep=30000",
            "checksoldier=soldier attack 2",
            "consume=spear_heavy empire_bread fish,meat:2",
            "train=soldier attack 2 3"
         }
      },
      upgrade_soldier_attack_3 = {
         descname = "upgrading soldier attack from level 3 to level 4",
         actions = {
            "checksoldier=soldier attack 3",
            "sleep=30000",
            "checksoldier=soldier attack 3",
            "consume=spear_war empire_bread:2 fish,meat",
            "train=soldier attack 3 4"
         }
      },
      upgrade_soldier_health_0 = {
         descname = "upgrading soldier health from level 0 to level 1",
         actions = {
            "checksoldier=soldier health 0",
            "sleep=30000",
            "checksoldier=soldier health 0",
            "consume=armor_helmet empire_bread,fish,meat",
            "train=soldier health 0 1"
         }
      },
      upgrade_soldier_health_1 = {
         descname = "upgrading soldier health from level 1 to level 2",
         actions = {
            "checksoldier=soldier health 1",
            "sleep=30000",
            "checksoldier=soldier health 1",
            "consume=armor empire_bread fish,meat",
            "train=soldier health 1 2"
         }
      },
      upgrade_soldier_health_2 = {
         descname = "upgrading soldier health from level 2 to level 3",
         actions = {
            "checksoldier=soldier health 2",
            "sleep=30000",
            "checksoldier=soldier health 2",
            "consume=armor_chain empire_bread:2 fish,meat:2",
            "train=soldier health 2 3"
         }
      },
      upgrade_soldier_health_3 = {
         descname = "upgrading soldier health from level 3 to level 4",
         actions = {
            "checksoldier=soldier health 3",
            "sleep=30000",
            "checksoldier=soldier health 3",
            "consume=armor_gilded empire_bread:2 fish,meat:2",
            "train=soldier health 3 4"
         }
      },
   },

   soldier_capacity = 2,
   trainer_patience = 12
}
