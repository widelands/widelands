push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "europeans_building",
   name = "europeans_colosseum",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Colosseum"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   enhancement_cost = {
      planks = 2,
      brick = 4,
      grout = 4,
      marble_column = 6,
      quartz = 3,
      diamond = 3,
      gold = 3
   },
   return_on_dismantle_on_enhanced = {
      log = 2,
      granite = 2,
      marble = 4,
      quartz = 2,
      diamond = 2,
      scrap_metal_mixed = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 81, 106 }
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
      { name = "europeans_bread", amount = 12 },
      { name = "smoked_fish", amount = 6 },
      { name = "smoked_meat", amount = 6 },
      { name = "beer", amount = 6 },
      { name = "mead", amount = 6 },
      { name = "beer_strong", amount = 6 },
      { name = "wine", amount = 6 },
      { name = "tabard_golden", amount = 4 },
      { name = "armor_gilded", amount = 4 },
      { name = "helmet_golden", amount = 4 },
      { name = "helmet_warhelm", amount = 4 },
      { name = "shield_advanced", amount = 4 },
      { name = "ax_warriors", amount = 4 },
      { name = "trident_heavy_double", amount = 4 },  
      
   },
   outputs = {
      "empire_soldier",
      "scrap_metal_mixed",
   },

   ["soldier evade"] = {
      min_level = 0,
      max_level = 5,
      food = {
         {"smoked_fish", "smoked_meat"},
       {"europeans_bread"},
       {"beer", "mead", "beer_strong", "wine"}
      }
   },
   
   ["soldier health"] = {
      min_level = 3,
      max_level = 5,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"europeans_bread"}
      },
      weapons = {
        "tabard_golden", "armor_gilded", "helmet_golden", "helmet_warhelm"
      }
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
            "checksoldier=soldier health 9" -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_evade_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier evade from level 0 to level 1"),
         actions = {
            "return=skipped when economy needs beer",
            "checksoldier=soldier evade 0", -- Fails when aren't any soldier of level 0 evade
            "sleep=60000",
            "checksoldier=soldier evade 0", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat beer:3",
            "train=soldier evade 0 1"
         }
      },
      upgrade_soldier_evade_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier evade from level 1 to level 2"),
         actions = {
            "return=skipped when economy needs mead",
            "checksoldier=soldier evade 1", -- Fails when aren't any soldier of level 1 evade
            "sleep=60000",
            "checksoldier=soldier evade 1", -- Because the soldier can be expelled by the player
            "consume=europeans_bread:2 smoked_fish,smoked_meat:2 mead:3",
            "train=soldier evade 1 2"
         }
      },
      upgrade_soldier_evade_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier evade from level 2 to level 3"),
         actions = {
            "return=skipped when economy needs beer_strong",
            "checksoldier=soldier evade 2", -- Fails when aren't any soldier of level 0 evade
            "sleep=60000",
            "checksoldier=soldier evade 2", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat beer_strong:3",
            "train=soldier evade 2 3"
         }
      },
      upgrade_soldier_evade_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier evade from level 3 to level 4"),
         actions = {
            "return=skipped when economy needs wine",
            "checksoldier=soldier evade 3", -- Fails when aren't any soldier of level 1 evade
            "sleep=60000",
            "checksoldier=soldier evade 3", -- Because the soldier can be expelled by the player
            "consume=europeans_bread:2 smoked_fish,smoked_meat:2 wine:3",
            "train=soldier evade 3 4"
         }
      },
      upgrade_soldier_evade_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier evade from level 4 to level 5"),
         actions = {
            "return=skipped when economy needs beer",
            "return=skipped when economy needs mead",
            "checksoldier=soldier evade 4", -- Fails when aren't any soldier of level 1 evade
            "sleep=60000",
            "checksoldier=soldier evade 4", -- Because the soldier can be expelled by the player
            "consume=europeans_bread:2 smoked_fish,smoked_meat:2 beer:3 mead:3",
            "train=soldier evade 4 5"
         }
      },
      upgrade_soldier_evade_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier evade from level 5 to level 6"),
         actions = {
            "return=skipped when economy needs beer_strong",
            "return=skipped when economy needs wine",
            "checksoldier=soldier evade 5", -- Fails when aren't any soldier of level 1 evade
            "sleep=60000",
            "checksoldier=soldier evade 5", -- Because the soldier can be expelled by the player
            "consume=europeans_bread:2 smoked_fish,smoked_meat:2 beer_strong:3 wine:3",
            "train=soldier evade 5 6"
         }
      },
      upgrade_soldier_health_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier health from level 3 to level 4"),
         actions = {
            "return=skipped when economy needs tabard_golden and economy needs armor_gilded",
            "checksoldier=soldier health 3", -- Fails when aren't any soldier of level 0 health
            "sleep=30000",
            "checksoldier=soldier health 3", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat armor_gilded,tabard_golden",
            "train=soldier health 3 4",
            "produce=scrap_metal_mixed"
         }
      },
      upgrade_soldier_health_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier health from level 4 to level 5"),
         actions = {
            "return=skipped when economy needs helmet_golden",
            "checksoldier=soldier health 4", -- Fails when aren't any soldier of level 0 health
            "sleep=30000",
            "checksoldier=soldier health 4", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat helmet_golden",
            "train=soldier health 4 5",
            "produce=scrap_metal_mixed"
         }
      },
      upgrade_soldier_health_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier health from level 5 to level 6"),
         actions = {
            "return=skipped when economy needs helmet_warhelm",
            "checksoldier=soldier health 5", -- Fails when aren't any soldier of level 0 health
            "sleep=30000",
            "checksoldier=soldier health 5", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat helmet_warhelm",
            "train=soldier health 5 6",
            "produce=scrap_metal_mixed"
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
            "checksoldier=soldier defense 4", -- Fails when aren't any soldier of level 0 defense
            "sleep=60000",
            "checksoldier=soldier defense 4", -- Because the soldier can be expulsed by the player
            "consume=europeans_bread smoked_fish,smoked_meat",
            "train=soldier defense 4 5"
         }
      },
      upgrade_soldier_defense_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier defense from level 5 to level 6"),
         actions = {
            "checksoldier=soldier defense 5", -- Fails when aren't any soldier of level 0 defense
            "sleep=60000",
            "checksoldier=soldier defense 5", -- Because the soldier can be expulsed by the player
            "consume=europeans_bread smoked_fish,smoked_meat",
            "train=soldier defense 5 6"
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
   
   soldier_capacity = 12,
   trainer_patience = 24,
   
   messages = {
      -- TRANSLATORS: Empire training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("europeans_building", "No soldier to train!"),
      -- TRANSLATORS: Empire training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("europeans_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
