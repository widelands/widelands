dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "empire_building",
   name = "empire_trainingcamp",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Training Camp"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 5,
      planks = 4,
      marble = 5,
      gold = 4,
      marble_column = 3,
      cloth = 2
   },
   return_on_dismantle = {
      granite = 3,
      planks = 2,
      marble = 3,
      gold = 2,
      marble_column = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 82, 105 }
      }
   },

   aihints = {
      prohibited_till = 2700,
      trainingsite_type = "advanced",
      very_weak_ai_limit = 0,
      weak_ai_limit = 1
   },

   working_positions = {
      empire_trainer = 1
   },

   inputs = {
      empire_bread = 10,
      fish = 6,
      meat = 6,
      spear = 2,
      spear_advanced = 2,
      spear_heavy = 2,
      spear_war = 2,
      armor_helmet = 2,
      armor = 2,
      armor_chain = 2,
      armor_gilded = 2
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
   ["soldier hp"] = {
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
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _"sleeping",
         actions = {
            "sleep=5000",
            "check_soldier=soldier attack 9", -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_attack_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier attack from level 0 to level 1",
         actions = {
            "check_soldier=soldier attack 0", -- Fails when aren't any soldier of level 0 attack
            "sleep=30000",
            "check_soldier=soldier attack 0", -- Because the soldier can be expelled by the player
            "consume=spear empire_bread fish,meat",
            "train=soldier attack 0 1"
         }
      },
      upgrade_soldier_attack_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier attack from level 1 to level 2",
         actions = {
            "check_soldier=soldier attack 1",
            "sleep=30000",
            "check_soldier=soldier attack 1",
            "consume=spear_advanced empire_bread fish,meat",
            "train=soldier attack 1 2"
         }
      },
      upgrade_soldier_attack_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier attack from level 2 to level 3",
         actions = {
            "check_soldier=soldier attack 2",
            "sleep=30000",
            "check_soldier=soldier attack 2",
            "consume=spear_heavy empire_bread fish,meat:2",
            "train=soldier attack 2 3"
         }
      },
      upgrade_soldier_attack_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier attack from level 3 to level 4",
         actions = {
            "check_soldier=soldier attack 3",
            "sleep=30000",
            "check_soldier=soldier attack 3",
            "consume=spear_war empire_bread:2 fish,meat",
            "train=soldier attack 3 4"
         }
      },
      upgrade_soldier_hp_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier health from level 0 to level 1",
         actions = {
            "check_soldier=soldier hp 0",
            "sleep=30000",
            "check_soldier=soldier hp 0",
            "consume=armor_helmet empire_bread,fish,meat",
            "train=soldier hp 0 1"
         }
      },
      upgrade_soldier_hp_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier health from level 1 to level 2",
         actions = {
            "check_soldier=soldier hp 1",
            "sleep=30000",
            "check_soldier=soldier hp 1",
            "consume=armor empire_bread fish,meat",
            "train=soldier hp 1 2"
         }
      },
      upgrade_soldier_hp_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier health from level 2 to level 3",
         actions = {
            "check_soldier=soldier hp 2",
            "sleep=30000",
            "check_soldier=soldier hp 2",
            "consume=armor_chain empire_bread:2 fish,meat:2",
            "train=soldier hp 2 3"
         }
      },
      upgrade_soldier_hp_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier health from level 3 to level 4",
         actions = {
            "check_soldier=soldier hp 3",
            "sleep=30000",
            "check_soldier=soldier hp 3",
            "consume=armor_gilded empire_bread:2 fish,meat:2",
            "train=soldier hp 3 4"
         }
      },
   },

   soldier_capacity = 12,
   trainer_patience = 12
}
