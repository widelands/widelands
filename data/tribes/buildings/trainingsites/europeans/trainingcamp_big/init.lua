push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_trainingsite_type {
   msgctxt = "europeans_building",
   name = "europeans_trainingscamp_big",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Big Trainingscamp"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 8,
      spidercloth = 4,
      brick = 4,
      grout = 4,
      marble_column = 4,
      quartz = 3,
      diamond = 3,
      gold = 3
   },
   return_on_dismantle = {
      log = 3,
      spidercloth = 1,
      granite = 2,
      marble = 2,
      quartz = 2,
      diamond = 2,
      scrap_metal_mixed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 82, 105 }
      }
   },

   aihints = {
      prohibited_till = 18000,
      very_weak_ai_limit = 0,
      weak_ai_limit = 1,
     --normal_ai_limit = 4
   },

   working_positions = {
      europeans_trainer = 1
   },

   inputs = {
      { name = "europeans_bread", amount = 16 },
      { name = "smoked_fish", amount = 8 },
      { name = "smoked_meat", amount = 8 },
      { name = "shield_advanced", amount = 4 },
      { name = "ax_warriors", amount = 4 },
      { name = "trident_heavy_double", amount = 4 },
   },
   outputs = {
      "empire_soldier",
      "scrap_metal_mixed",
   },

   ["soldier defense"] = {
      min_level = 3,
      max_level = 5,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"europeans_bread"}
      },
      weapons = {
        "shield_advanced"
      }
   },

   ["soldier attack"] = {
      min_level = 4,
      max_level = 5,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"europeans_bread"}
      },
      weapons = {
         "ax_warriors", "trident_heavy_double"
      }
   },
   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _"sleeping",
         actions = {
            "sleep=5000",
            "checksoldier=soldier attack 9", -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_defense_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier defense from level 3 to level 4"),
         actions = {
            "return=skipped when economy needs shield_advanced",
            "checksoldier=soldier defense 3", -- Fails when aren't any soldier of level 0 defense
            "sleep=60000",
            "checksoldier=soldier defense 3", -- Because the soldier can be expulsed by the player
            "consume=europeans_bread smoked_fish,smoked_meat shield_advanced",
            "train=soldier defense 3 4",
            "produce=scrap_metal_mixed"
         }
      },
      upgrade_soldier_defense_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier defense from level 4 to level 5"),
         actions = {
            "return=skipped when economy needs shield_advanced",
            "checksoldier=soldier defense 4", -- Fails when aren't any soldier of level 0 defense
            "sleep=60000",
            "checksoldier=soldier defense 4", -- Because the soldier can be expulsed by the player
            "consume=europeans_bread smoked_fish,smoked_meat shield_advanced",
            "train=soldier defense 4 5",
            "produce=scrap_metal_mixed"
         }
      },
      upgrade_soldier_defense_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier defense from level 5 to level 6"),
         actions = {
            "return=skipped when economy needs shield_advanced",
            "checksoldier=soldier defense 5", -- Fails when aren't any soldier of level 0 defense
            "sleep=60000",
            "checksoldier=soldier defense 5", -- Because the soldier can be expulsed by the player
            "consume=europeans_bread smoked_fish,smoked_meat shield_advanced",
            "train=soldier defense 5 6",
            "produce=scrap_metal_mixed"
         }
      },
      upgrade_soldier_attack_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier attack from level 4 to level 5"),
         actions = {
            "return=skipped when economy needs ax_warriors",
            "checksoldier=soldier attack 4",
            "sleep=60000",
            "checksoldier=soldier attack 4",
            "consume=europeans_bread smoked_fish,smoked_meat ax_warriors",
            "train=soldier attack 4 5",
            "produce=scrap_metal_mixed:2"
         }
      },
      upgrade_soldier_attack_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier attack from level 5 to level 6"),
         actions = {
            "return=skipped when economy needs trident_heavy_double",
            "checksoldier=soldier attack 5",
            "sleep=60000",
            "checksoldier=soldier attack 5",
            "consume=europeans_bread smoked_fish,smoked_meat trident_heavy_double",
            "train=soldier attack 5 6",
            "produce=scrap_metal_mixed:2"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 24,
   
   messages = {
      -- TRANSLATORS: Empire training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("europeans_building", "No soldier to train!"),
      -- TRANSLATORS: Empire training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("europeans_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
