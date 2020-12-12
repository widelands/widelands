push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_trainingsite_type {
   name = "europeans_trainingscamp_big",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Big Trainingscamp"),
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
      { name = "meal", amount = 16 },
      { name = "shield_advanced", amount = 4 },
      { name = "ax_warriors", amount = 4 },
      { name = "trident_heavy_double", amount = 4 },
   },

   ["soldier defense"] = {
      food = {
         {"meal"}
      },
      weapons = {"shield_advanced"}
   },

   ["soldier attack"] = {
      food = {
         {"meal"}
      },
      weapons = {"ax_warriors", "trident_heavy_double"}
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
      upgrade_soldier_defense_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier defense from level 3 to level 4"),
         actions = {
            "return=skipped when economy needs shield_advanced",
            "checksoldier=soldier:defense level:3", -- Fails when aren't any soldier of level 0 defense
            "sleep=duration:45s",
            "checksoldier=soldier:defense level:3", -- Because the soldier can be expulsed by the player
            "consume=meal shield_advanced",
            "train=soldier:defense level:4",
            "produce=scrap_metal_mixed"
         }
      },
      upgrade_soldier_defense_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier defense from level 4 to level 5"),
         actions = {
            "return=skipped when economy needs shield_advanced",
            "checksoldier=soldier:defense level:4", -- Fails when aren't any soldier of level 0 defense
            "sleep=duration:45s",
            "checksoldier=soldier:defense level:4", -- Because the soldier can be expulsed by the player
            "consume=meal shield_advanced",
            "train=soldier:defense level:5",
            "produce=scrap_metal_mixed"
         }
      },
      upgrade_soldier_defense_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier defense from level 5 to level 6"),
         actions = {
            "return=skipped when economy needs shield_advanced",
            "checksoldier=soldier:defense level:5", -- Fails when aren't any soldier of level 0 defense
            "sleep=duration:45s",
            "checksoldier=soldier:defense level:5", -- Because the soldier can be expulsed by the player
            "consume=meal shield_advanced",
            "train=soldier:defense level:6",
            "produce=scrap_metal_mixed"
         }
      },
      upgrade_soldier_attack_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier attack from level 4 to level 5"),
         actions = {
            "return=skipped when economy needs ax_warriors",
            "checksoldier=soldier:attack level:4",
            "sleep=duration:45s",
            "checksoldier=soldier:attack level:4",
            "consume=meal ax_warriors",
            "train=soldier:attack level:5",
            "produce=scrap_metal_mixed:2"
         }
      },
      upgrade_soldier_attack_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier attack from level 5 to level 6"),
         actions = {
            "return=skipped when economy needs trident_heavy_double",
            "checksoldier=soldier:attack level:5",
            "sleep=duration:45s",
            "checksoldier=soldier:attack level:5",
            "consume=meal trident_heavy_double",
            "train=soldier:attack level:6",
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
