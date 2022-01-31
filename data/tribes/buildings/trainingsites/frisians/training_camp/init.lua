push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_trainingsite_type {
   name = "frisians_training_camp",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Training Camp"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 5,
      granite = 3,
      log = 3,
      gold = 1,
      reed = 3
   },
   return_on_dismantle = {
      brick = 3,
      granite = 1,
      log = 1,
      gold = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 79},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {50, 79},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {50, 60}
      }
   },

   aihints = {
      prohibited_till = 900,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      frisians_trainer = 1
   },

   inputs = {
      { name = "beer", amount = 6 },
      { name = "bread_frisians", amount = 6 },
      { name = "smoked_fish", amount = 6 },
      { name = "smoked_meat", amount = 6 },
      { name = "sword_long", amount = 2 },
      { name = "sword_broad", amount = 2 },
      { name = "sword_double", amount = 2 },
      { name = "helmet", amount = 2 },
      { name = "fur_garment_studded", amount = 2 },
   },

   ["soldier attack"] = {
      food = {
         {"smoked_fish", "smoked_meat"},
         {"beer"},
         {"bread_frisians"},
      },
      weapons = {
         "sword_long",
         "sword_broad",
         "sword_double",
      }
   },
   ["soldier health"] = {
      food = {
         {"bread_frisians", "beer"},
         {"smoked_fish", "smoked_meat"},
      },
      weapons = {
         "helmet",
      }
   },
   ["soldier defense"] = {
      food = {
         {"bread_frisians", "beer"},
         {"smoked_fish", "smoked_meat"},
      },
      weapons = {
         "fur_garment_studded",
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
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:attack level:0", -- Fails when aren't any soldier of level 0 attack
            "return=failed unless site has sword_long",
            "return=failed unless site has bread_frisians,beer,smoked_fish,smoked_meat",
            "sleep=duration:12s400ms",
            "animate=working duration:14s",
            "checksoldier=soldier:attack level:0", -- Because the soldier can be expelled by the player
            "consume=sword_long bread_frisians,beer,smoked_fish,smoked_meat",
            "train=soldier:attack level:1",
            "produce=scrap_iron"
         }
      },
      upgrade_soldier_attack_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:attack level:1",
            "return=failed unless site has sword_broad",
            "return=failed unless site has bread_frisians",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:10s800ms",
            "animate=working duration:12s",
            "checksoldier=soldier:attack level:1",
            "consume=sword_broad bread_frisians smoked_fish,smoked_meat",
            "train=soldier:attack level:2",
            "produce=scrap_iron:2"
         }
      },
      upgrade_soldier_attack_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 2 to level 3"),
         actions = {
            "checksoldier=soldier:attack level:2",
            "return=failed unless site has sword_double",
            "return=failed unless site has beer",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:10s800ms",
            "animate=working duration:12s",
            "checksoldier=soldier:attack level:2",
            "consume=sword_double beer smoked_fish,smoked_meat",
            "train=soldier:attack level:3",
            "produce=scrap_iron scrap_metal_mixed"
         }
      },
      upgrade_soldier_health_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier health from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:health level:0",
            "return=failed unless site has helmet",
            "return=failed unless site has bread_frisians,beer",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:15s",
            "animate=working duration:15s",
            "checksoldier=soldier:health level:0",
            "consume=helmet bread_frisians,beer smoked_fish,smoked_meat",
            "train=soldier:health level:1"
         }
      },
      upgrade_soldier_defense_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier defense from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:defense level:0",
            "return=failed unless site has fur_garment_studded",
            "return=failed unless site has bread_frisians,beer",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:12s400ms",
            "animate=working duration:14s",
            "checksoldier=soldier:defense level:0",
            "consume=fur_garment_studded bread_frisians,beer smoked_fish,smoked_meat",
            "train=soldier:defense level:1",
            "produce=fur_garment_old"
         }
      },
   },

   soldier_capacity = 10,
   trainer_patience = 10,

   messages = {
      -- TRANSLATORS: Frisian training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("frisians_building", "No soldier to train!"),
      -- TRANSLATORS: Frisian training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("frisians_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
