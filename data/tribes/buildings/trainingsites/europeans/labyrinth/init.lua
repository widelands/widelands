push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_trainingsite_type {
   msgctxt = "europeans_building",
   name = "europeans_labyrinth",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Labyrinth"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   enhancement = {
        name = "europeans_arena",
        enhancement_cost = {
          planks = 6,
          brick = 4,
          grout = 4,
          marble_column = 4,
          quartz = 3,
          diamond = 3
        },
        enhancement_return_on_dismantle = {
          log = 5,
          granite = 3,
          marble = 3,
          quartz = 2,
          diamond = 2,
          scrap_metal_mixed = 2
        },
   },

   buildcost = {
      planks = 10,
      spidercloth = 4,
      brick = 4,
      grout = 4,
      quartz = 2,
      diamond = 2
   },
   return_on_dismantle = {
      log = 6,
      spidercloth = 2,
      granite = 3,
      quartz = 1,
      diamond = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 80, 88 },
      }
   },

   aihints = {
      prohibited_till = 7200,
      very_weak_ai_limit = 1,
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
      { name = "mead", amount = 6 }
   },
   outputs = {
      "europeans_soldier"
   },

   ["soldier evade"] = {
      min_level = 0,
      max_level = 1,
      food = {
         {"smoked_fish", "smoked_meat"},
       {"europeans_bread"},
       {"beer", "mead"}
      }
   },

   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _"sleeping",
         actions = {
            "sleep=5000",
            "checksoldier=soldier evade 9" -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_evade_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier evade from level 0 to level 1"),
         actions = {
            "return=skipped when economy needs beer",
            "checksoldier=soldier evade 0", -- Fails when aren't any soldier of level 0 evade
            "sleep=90000",
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
            "sleep=90000",
            "checksoldier=soldier evade 1", -- Because the soldier can be expelled by the player
            "consume=europeans_bread:2 smoked_fish,smoked_meat:2 mead:3",
            "train=soldier evade 1 2"
         }
      },
   },

   soldier_capacity = 12,
   trainer_patience = 18,
   
   messages = {
      -- TRANSLATORS: Empire training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("europeans_building", "No soldier to train!"),
      -- TRANSLATORS: Empire training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("europeans_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
