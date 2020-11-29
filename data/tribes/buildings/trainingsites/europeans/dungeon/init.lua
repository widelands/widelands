push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "europeans_building",
   name = "europeans_dungeon",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Dungeon"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      planks = 6,
      spidercloth = 4,
      brick = 4,
      grout = 4,
      quartz = 2,
      diamond = 2
   },
   return_on_dismantle = {
      log = 4,
      spidercloth = 2,
      granite = 3,
      quartz = 1,
      diamond = 1,
      scrap_metal_mixed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 47, 47 },
      }
   },

   aihints = {
      prohibited_till = 7200,
      very_weak_ai_limit = 1,
      weak_ai_limit = 1,
     --normal_ai_limit = 2
   },

   working_positions = {
      europeans_trainer = 1
   },

   inputs = {
      { name = "europeans_bread", amount = 12 },
      { name = "smoked_fish", amount = 6 },
      { name = "smoked_meat", amount = 6 },
      { name = "shield_steel", amount = 4 }
   },
   outputs = {
      "europeans_soldier",
      "scrap_iron",
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
            "checksoldier=soldier defense 9" -- dummy check to get sleep rated as skipped - else it will change statistics
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

   soldier_capacity = 4,
   trainer_patience = 24,
   
   messages = {
      -- TRANSLATORS: Empire training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("europeans_building", "No soldier to train!"),
      -- TRANSLATORS: Empire training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("europeans_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
