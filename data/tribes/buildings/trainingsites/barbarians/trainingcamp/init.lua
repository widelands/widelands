dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_trainingcamp",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Training Camp"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 6,
      granite = 4,
      grout = 6,
      gold = 4,
      reed = 3
   },
   return_on_dismantle = {
      log = 3,
      granite = 2,
      grout = 2,
      gold = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 110, 75 },
         fps = 10
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 110, 77 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 110, 75 }
      }
   },

   aihints = {
      prohibited_till = 1500,
      very_weak_ai_limit = 0,
      weak_ai_limit = 1
   },

   working_positions = {
      barbarians_trainer = 1
   },

   inputs = {
      { name = "fish", amount = 6 },
      { name = "meat", amount = 6 },
      { name = "barbarians_bread", amount = 10 },
      { name = "ax_sharp", amount = 2 },
      { name = "ax_broad", amount = 2 },
      { name = "ax_bronze", amount = 2 },
      { name = "ax_battle", amount = 2 },
      { name = "ax_warriors", amount = 2 },
      { name = "helmet", amount = 2 },
      { name = "helmet_mask", amount = 2 },
      { name = "helmet_warhelm", amount = 2 }
   },

   ["soldier attack"] = {
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
   ["soldier health"] = {
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
            "sleep=duration:5s",
            "return=skipped",
         }
      },
      upgrade_soldier_attack_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier attack from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:attack level:0", -- Fails when aren't any soldier of level 0 attack
            "return=failed unless site has ax_sharp",
            "return=failed unless site has fish,meat",
            "return=failed unless site has barbarians_bread",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:0", -- Because the soldier can be expelled by the player
            "consume=ax_sharp fish,meat barbarians_bread",
            "train=soldier:attack level:1"
         }
      },
      upgrade_soldier_attack_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier attack from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:attack level:1",
            "return=failed unless site has ax_broad",
            "return=failed unless site has fish,meat",
            "return=failed unless site has barbarians_bread",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:1",
            "consume=ax_broad fish,meat barbarians_bread",
            "train=soldier:attack level:2"
         }
      },
      upgrade_soldier_attack_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier attack from level 2 to level 3"),
         actions = {
            "checksoldier=soldier:attack level:2",
            "return=failed unless site has ax_bronze",
            "return=failed unless site has fish,meat",
            "return=failed unless site has barbarians_bread",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:2",
            "consume=ax_bronze fish,meat barbarians_bread",
            "train=soldier:attack level:3"
         }
      },
      upgrade_soldier_attack_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier attack from level 3 to level 4"),
         actions = {
            "checksoldier=soldier:attack level:3",
            "return=failed unless site has ax_battle",
            "return=failed unless site has fish,meat:2",
            "return=failed unless site has barbarians_bread",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:3",
            "consume=ax_battle fish,meat:2 barbarians_bread",
            "train=soldier:attack level:4"
         }
      },
      upgrade_soldier_attack_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier attack from level 4 to level 5"),
         actions = {
            "checksoldier=soldier:attack level:4",
            "return=failed unless site has ax_warriors",
            "return=failed unless site has fish,meat",
            "return=failed unless site has barbarians_bread:2",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:4",
            "consume=ax_warriors fish,meat barbarians_bread:2",
            "train=soldier:attack level:5"
         }
      },
      upgrade_soldier_health_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier health from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:health level:0",
            "return=failed unless site has helmet",
            "return=failed unless site has barbarians_bread,fish,meat",
            "sleep=duration:30s",
            "checksoldier=soldier:health level:0",
            "consume=helmet barbarians_bread,fish,meat",
            "train=soldier:health level:1"
         }
      },
      upgrade_soldier_health_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier health from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:health level:1",
            "return=failed unless site has helmet_mask",
            "return=failed unless site has fish,meat",
            "return=failed unless site has barbarians_bread",
            "sleep=duration:30s",
            "checksoldier=soldier:health level:1",
            "consume=helmet_mask fish,meat barbarians_bread",
            "train=soldier:health level:2"
         }
      },
      upgrade_soldier_health_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier health from level 2 to level 3"),
         actions = {
            "checksoldier=soldier:health level:2",
            "return=failed unless site has helmet_warhelm",
            "return=failed unless site has fish,meat:2",
            "return=failed unless site has barbarians_bread",
            "sleep=duration:30s",
            "checksoldier=soldier:health level:2",
            "consume=helmet_warhelm fish,meat:2 barbarians_bread",
            "train=soldier:health level:3"
         }
      },
   },

   soldier_capacity = 12,
   trainer_patience = 5
}
