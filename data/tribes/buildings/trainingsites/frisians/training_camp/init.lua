dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "frisians_building",
   name = "frisians_training_camp",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Small Training Camp"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 4,
      granite = 3,
      log = 3,
      gold = 1,
      thatch_reed = 4
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      log = 1,
      gold = 2,
      thatch_reed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 59, 60 }
      }
   },

   aihints = {
      prohibited_till = 900,
      forced_after = 1500,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      frisians_trainer = 1
   },

   inputs = {
      { name = "smoked_fish", amount = 6 },
      { name = "smoked_meat", amount = 6 },
      { name = "beer", amount = 6 },
      { name = "bread_frisians", amount = 6 },
      { name = "sword_long", amount = 2 },
      { name = "sword_curved", amount = 2 },
      { name = "sword_double", amount = 2 },
      { name = "helmet", amount = 2 },
      { name = "fur_clothes_studded", amount = 2 },
   },
   outputs = {
      "frisians_soldier",
      "scrap_metal_mixed",
      "scrap_metal_iron",
   },

   ["soldier attack"] = {
      min_level = 0,
      max_level = 2,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"beer"},
         {"bread_frisians"}
      },
      weapons = {
         "sword_long",
         "sword_curved",
         "sword_double",
      }
   },
   ["soldier health"] = {
      min_level = 0,
      max_level = 0,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"beer"},
         {"bread_frisians"}
      },
      weapons = {
         "helmet",
      }
   },
   ["soldier defense"] = {
      min_level = 0,
      max_level = 0,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"beer"},
         {"bread_frisians"}
      },
      weapons = {
         "fur_clothes_studded",
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
         descname = pgettext("frisians_building", "upgrading soldier attack from level 0 to level 1"),
         actions = {
            "check_soldier=soldier attack 0", -- Fails when aren't any soldier of level 0 attack
            "sleep=30000",
            "check_soldier=soldier attack 0", -- Because the soldier can be expelled by the player
            "consume=sword_long smoked_fish,smoked_meat beer,bread_frisians",
            "train=soldier attack 0 1",
            "produce=scrap_metal_iron"
         }
      },
      upgrade_soldier_attack_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("frisians_building", "upgrading soldier attack from level 1 to level 2"),
         actions = {
            "check_soldier=soldier attack 1",
            "sleep=30000",
            "check_soldier=soldier attack 1",
            "consume=sword_curved smoked_fish,smoked_meat beer,bread_frisians",
            "train=soldier attack 1 2",
            "produce=scrap_metal_iron:2"
         }
      },
      upgrade_soldier_attack_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("frisians_building", "upgrading soldier attack from level 2 to level 3"),
         actions = {
            "check_soldier=soldier attack 2",
            "sleep=30000",
            "check_soldier=soldier attack 2",
            "consume=sword_double smoked_fish,smoked_meat beer,bread_frisians",
            "train=soldier attack 2 3",
            "produce=scrap_metal_mixed"
         }
      },
      upgrade_soldier_health_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("frisians_building", "upgrading soldier health from level 0 to level 1"),
         actions = {
            "check_soldier=soldier health 0",
            "sleep=30000",
            "check_soldier=soldier health 0",
            "consume=helmet smoked_fish,smoked_meat beer,bread_frisians",
            "train=soldier health 0 1"
         }
      },
      upgrade_soldier_defense_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("frisians_building", "upgrading soldier defense from level 0 to level 1"),
         actions = {
            "check_soldier=soldier defense 0",
            "sleep=30000",
            "check_soldier=soldier defense 0",
            "consume=fur_clothes_studded smoked_fish,smoked_meat beer,bread_frisians",
            "train=soldier defense 0 1"
         }
      },
   },

   soldier_capacity = 10,
   trainer_patience = 5
}
