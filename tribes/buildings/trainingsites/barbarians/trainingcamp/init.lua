dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_trainingcamp",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Training Camp"),
   directory = dirname,
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
      granite = 2,
      grout = 2,
      gold = 2
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 110, 75 },
         fps = 10
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 110, 77 },
         fps = 1
      },
      unoccupied = {
         template = "unoccupied_??",
         directory = dirname,
         hotspot = { 110, 75 }
      }
   },

   aihints = {
      prohibited_till = 500,
      trainingsite_type = "advanced"
   },

   working_positions = {
      barbarians_trainer = 1
   },

   inputs = {
      barbarians_bread = 10,
      fish = 6,
      meat = 6,
      ax_sharp = 2,
      ax_broad = 2,
      ax_bronze = 2,
      ax_battle = 2,
      ax_warriors = 2,
      helmet = 2,
      helmet_mask = 2,
      helmet_warhelm = 2
   },
   outputs = {
      "barbarians_soldier",
   },

   ["soldier attack"] = {
      min_level = 0,
      max_level = 4,
      food = {
         {"fish", "meat"},
         {"barbarians_bread"}
      },
      weapons = {
         "ax_sharp",
         "ax_broad",
         "ax_bronze",
         "ax_battle",
         "ax_warriors"
      }
   },
   ["soldier hp"] = {
      min_level = 0,
      max_level = 2,
      food = {
         {"fish", "meat"},
         {"barbarians_bread"}
      },
      weapons = {
         "helmet",
         "helmet_mask",
         "helmet_warhelm"
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
            "consume=ax_sharp fish,meat barbarians_bread",
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
            "consume=ax_broad fish,meat barbarians_bread",
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
            "consume=ax_bronze fish,meat barbarians_bread",
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
            "consume=ax_battle fish,meat:2 barbarians_bread",
            "train=soldier attack 3 4"
         }
      },
      upgrade_soldier_attack_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier attack from level 4 to level 5",
         actions = {
            "check_soldier=soldier attack 4",
            "sleep=30000",
            "check_soldier=soldier attack 4",
            "consume=ax_warriors fish,meat barbarians_bread:2",
            "train=soldier attack 4 5"
         }
      },
      upgrade_soldier_hp_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier health from level 0 to level 1",
         actions = {
            "check_soldier=soldier hp 0",
            "sleep=30000",
            "check_soldier=soldier hp 0",
            "consume=helmet barbarians_bread,fish,meat",
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
            "consume=helmet_mask fish,meat barbarians_bread",
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
            "consume=helmet_warhelm fish,meat:2 barbarians_bread",
            "train=soldier hp 2 3"
         }
      },
   },

   soldier_capacity = 12,
   trainer_patience = 5
}