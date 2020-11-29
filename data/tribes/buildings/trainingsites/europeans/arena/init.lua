push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "europeans_building",
   name = "europeans_arena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Arena"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   enhancement = "europeans_colosseum",

   enhancement_cost = {
      planks = 6,
      brick = 4,
      grout = 4,
      marble_column = 4,
      quartz = 3,
      diamond = 3
   },
   return_on_dismantle_on_enhanced = {
      log = 5,
      granite = 3,
      marble = 3,
      quartz = 2,
      diamond = 2,
      scrap_metal_mixed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 81, 82 }
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 82, 83 },
      }
   },

   aihints = {
      prohibited_till = 9000,
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
      { name = "armor_chain", amount = 4 },
      { name = "armor_helmet", amount = 4 },
      { name = "helmet_mask", amount = 4 },
      { name = "shield_steel", amount = 4 }
   },
   outputs = {
      "empire_soldier",
      "scrap_iron"
   },

   ["soldier evade"] = {
      min_level = 0,
      max_level = 3,
      food = {
       {"smoked_fish", "smoked_meat"},
       {"europeans_bread"},
       {"beer", "mead", "beer_strong", "wine"}
      }
   },

   ["soldier health"] = {
      min_level = 0,
      max_level = 2,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"europeans_bread"}
      },
      weapons = {
         "armor_chain", "armor_helmet", "helmet_mask"
      }
   },
   
   ["soldier defense"] = {
      min_level = 0,
      max_level = 2,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"europeans_bread"}
      },
      weapons = {
         "shield_steel"
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
      upgrade_soldier_health_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier health from level 0 to level 1"),
         actions = {
            "return=skipped when economy needs armor_chain",
            "checksoldier=soldier health 0", -- Fails when aren't any soldier of level 0 health
            "sleep=60000",
            "checksoldier=soldier health 0", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat armor_chain",
            "train=soldier health 0 1",
            "produce=scrap_iron"
         }
      },
      upgrade_soldier_health_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier health from level 1 to level 2"),
         actions = {
            "return=skipped when economy needs armor_helmet and economy needs helmet",
            "checksoldier=soldier health 1", -- Fails when aren't any soldier of level 0 health
            "sleep=60000",
            "checksoldier=soldier health 1", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat armor_helmet",
            "train=soldier health 1 2",
            "produce=scrap_iron"
         }
      },
      upgrade_soldier_health_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier health from level 2 to level 3"),
         actions = {
            "return=skipped when economy needs helmet_mask",
            "checksoldier=soldier health 2", -- Fails when aren't any soldier of level 0 health
            "sleep=60000",
            "checksoldier=soldier health 2", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat helmet_mask",
            "train=soldier health 2 3",
            "produce=scrap_iron"
         }
      },
      upgrade_soldier_defense_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier defense from level 0 to level 1"),
         actions = {
            "return=skipped when economy needs shield_steel",
            "checksoldier=soldier defense 0", -- Fails when aren't any soldier of level 0 defense
            "sleep=90000",
            "checksoldier=soldier defense 0", -- Because the soldier can be expulsed by the player
            "consume=europeans_bread smoked_fish,smoked_meat shield_steel",
            "train=soldier defense 0 1",
            "produce=scrap_iron"
         }
      },
      upgrade_soldier_defense_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier defense from level 1 to level 2"),
         actions = {
            "checksoldier=soldier defense 1", -- Fails when aren't any soldier of level 0 defense
            "sleep=90000",
            "checksoldier=soldier defense 1", -- Because the soldier can be expulsed by the player
            "consume=europeans_bread smoked_fish,smoked_meat",
            "train=soldier defense 1 2"
         }
      },
      upgrade_soldier_defense_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier defense from level 2 to level 3"),
         actions = {
            "checksoldier=soldier defense 2", -- Fails when aren't any soldier of level 0 defense
            "sleep=90000",
            "checksoldier=soldier defense 2", -- Because the soldier can be expulsed by the player
            "consume=europeans_bread smoked_fish,smoked_meat",
            "train=soldier defense 2 3"
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
