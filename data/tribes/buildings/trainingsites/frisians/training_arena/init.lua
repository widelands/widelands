push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_trainingsite_type {
   name = "frisians_training_arena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Training Arena"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 6,
      granite = 3,
      log = 4,
      gold = 3,
      reed = 4
   },
   return_on_dismantle = {
      brick = 3,
      granite = 2,
      log = 2,
      gold = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {83, 90},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {83, 90},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {83, 71}
      }
   },

   aihints = {
      trainingsites_max_percent = 40,
      prohibited_till = 1500,
      very_weak_ai_limit = 0,
      weak_ai_limit = 1
   },

   working_positions = {
      frisians_trainer = 1
   },

   inputs = {
      { name = "smoked_fish", amount = 6 },
      { name = "smoked_meat", amount = 6 },
      { name = "mead", amount = 6 },
      { name = "honey_bread", amount = 6 },
      { name = "sword_long", amount = 3 },
      { name = "sword_broad", amount = 3 },
      { name = "sword_double", amount = 3 },
      { name = "helmet_golden", amount = 2 },
      { name = "fur_garment_golden", amount = 2 },
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
      upgrade_soldier_attack_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 3 to level 4"),
         actions = {
            "checksoldier=soldier:attack level:3",
            "return=failed unless site has sword_long",
            "return=failed unless site has honey_bread,mead",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:10s800ms",
            "animate=working duration:12s",
            "checksoldier=soldier:attack level:3", -- Because the soldier can be expelled by the player
            "consume=sword_long honey_bread,mead smoked_fish,smoked_meat",
            "train=soldier:attack level:4"
         }
      },
      upgrade_soldier_attack_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 4 to level 5"),
         actions = {
            "checksoldier=soldier:attack level:4",
            "return=failed unless site has sword_broad",
            "return=failed unless site has honey_bread,mead",
            "return=failed unless site has smoked_fish,smoked_meat:2",
            "sleep=duration:5s600ms",
            "animate=working duration:10s",
            "checksoldier=soldier:attack level:4", -- Because the soldier can be expelled by the player
            "consume=sword_broad honey_bread,mead smoked_fish,smoked_meat:2",
            "train=soldier:attack level:5",
            "produce=scrap_iron:2"
         }
      },
      upgrade_soldier_attack_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 5 to level 6"),
         actions = {
            "checksoldier=soldier:attack level:5",
            "return=failed unless site has sword_double",
            "return=failed unless site has honey_bread",
            "return=failed unless site has mead",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:5s600ms",
            "animate=working duration:10s",
            "checksoldier=soldier:attack level:5", -- Because the soldier can be expelled by the player
            "consume=sword_double honey_bread mead smoked_fish,smoked_meat",
            "train=soldier:attack level:6",
            "produce=scrap_iron scrap_metal_mixed"
         }
      },
      upgrade_soldier_defense_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier defense from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:defense level:1",
            "return=failed unless site has fur_garment_golden",
            "return=failed unless site has honey_bread,mead",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:10s800ms",
            "animate=working duration:12s",
            "checksoldier=soldier:defense level:1", -- Because the soldier can be expelled by the player
            "consume=fur_garment_golden honey_bread,mead smoked_fish,smoked_meat",
            "train=soldier:defense level:2",
            "produce=scrap_iron fur_garment_old"
         }
      },
      upgrade_soldier_health_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier health from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:health level:1",
            "return=failed unless site has helmet_golden",
            "return=failed unless site has honey_bread,mead",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:10s800ms",
            "animate=working duration:12s",
            "checksoldier=soldier:health level:1", -- Because the soldier can be expelled by the player
            "consume=helmet_golden honey_bread,mead smoked_fish,smoked_meat",
            "train=soldier:health level:2",
            "produce=scrap_iron"
         }
      },
   },

   soldier_capacity = 6,
   trainer_patience = 6,

   messages = {
      -- TRANSLATORS: Frisian training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("frisians_building", "No soldier to train!"),
      -- TRANSLATORS: Frisian training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("frisians_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
