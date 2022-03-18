push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_trainingsite_type {
   name = "amazons_warriors_gathering",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Warriors’ Gathering"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      balsa = 2,
      ironwood = 2,
      log = 3,
      granite = 2,
      rubber = 5,
      rope = 2,
      gold = 1
   },
   return_on_dismantle = {
      balsa = 1,
      ironwood = 1,
      log = 1,
      granite = 2,
      rubber = 3,
      rope = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {43, 54}},
      unoccupied = {hotspot = {43, 54}},
   },
   spritesheets = {
      working = {
         hotspot = {43, 54},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      }
   },

   aihints = {
      trainingsites_max_percent = 25,
      prohibited_till = 900,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      amazons_trainer = 1
   },

   inputs = {
      { name = "fish", amount = 4 },
      { name = "meat", amount = 4 },
      { name = "amazons_bread", amount = 2 },
      { name = "chocolate", amount = 5 },
      { name = "spear_stone_tipped", amount = 3 },
      { name = "spear_hardened", amount = 3 },
   },

   ["soldier attack"] = {
      food = {
         {"fish", "meat"},
         {"chocolate"},
         {"amazons_bread"}
      },
      weapons = {
         "spear_stone_tipped",
         "spear_hardened",
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
      upgrade_soldier_attack_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("amazons_building", "upgrading soldier attack from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:attack level:0", -- Fails when aren't any soldier of level 0 attack
            "return=failed unless site has fish,meat",
            "return=failed unless site has chocolate",
            "return=failed unless site has spear_stone_tipped",
            "animate=working duration:30s",
            "checksoldier=soldier:attack level:0", -- Because the soldier can be expelled by the player
            "consume=fish,meat chocolate spear_stone_tipped",
            "train=soldier:attack level:1"
         }
      },
      upgrade_soldier_attack_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("amazons_building", "upgrading soldier attack from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:attack level:1", -- Fails when aren't any soldier of level 1 attack
            "return=failed unless site has amazons_bread",
            "return=failed unless site has fish,meat",
            "return=failed unless site has chocolate",
            "return=failed unless site has spear_hardened",
            "animate=working duration:30s",
            "checksoldier=soldier:attack level:1", -- Because the soldier can be expulsed by the player
            "consume=amazons_bread fish,meat chocolate spear_hardened",
            "train=soldier:attack level:2"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 18,

   messages = {
      -- TRANSLATORS: Amazon training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("amazons_building", "No soldier to train!"),
      -- TRANSLATORS: Amazon training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("amazons_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
