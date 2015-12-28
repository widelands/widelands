dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_labyrinth",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Labyrinth"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 3,
      granite = 4,
      planks = 5,
      spidercloth = 5,
      diamond = 2,
      quartz = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 3,
      planks = 2,
      spidercloth = 2,
      diamond = 1,
      quartz = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 80, 88 },
      }
   },

   aihints = {
      prohibited_till=900,
      forced_after=1500,
      trainingsite_type = "basic",
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      atlanteans_trainer = 1
   },

   inputs = {
      atlanteans_bread = 10,
      smoked_fish = 6,
      smoked_meat = 6,
      shield_steel = 4,
      shield_advanced = 4,
      tabard_golden = 5
   },
   outputs = {
      "atlanteans_soldier",
   },

   ["soldier defense"] = {
      min_level = 0,
      max_level = 1,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"atlanteans_bread"}
      },
      weapons = {
         "shield_steel",
         "shield_advanced"
      }
   },
   ["soldier hp"] = {
      min_level = 0,
      max_level = 0,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"atlanteans_bread"}
      },
      weapons = {
         "tabard_golden"
      }
   },
   ["soldier evade"] = {
      min_level = 0,
      max_level = 1,
      food = {
         {"smoked_fish", "smoked_meat"}
      }
   },

   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _"sleeping",
         actions = {
            "sleep=5000",
            "check_soldier=soldier attack 9" -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_defense_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier defense from level 0 to level 1",
         actions = {
            "check_soldier=soldier defense 0", -- Fails when aren't any soldier of level 0 defense
            "sleep=30000",
            "check_soldier=soldier defense 0", -- Because the soldier can be expulsed by the player
            "consume=atlanteans_bread smoked_fish,smoked_meat shield_steel",
            "train=soldier defense 0 1"
         }
      },
      upgrade_soldier_defense_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier defense from level 1 to level 2",
         actions = {
            "check_soldier=soldier defense 1", -- Fails when aren't any soldier of level 1 defense
            "sleep=30000",
            "check_soldier=soldier defense 1", -- Because the soldier can be expelled by the player
            "consume=atlanteans_bread smoked_fish,smoked_meat shield_advanced",
            "train=soldier defense 1 2"
         }
      },
      upgrade_soldier_hp_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier health from level 0 to level 1",
         actions = {
            "check_soldier=soldier hp 0", -- Fails when aren't any soldier of level 0 hp
            "sleep=30000",
            "check_soldier=soldier hp 0", -- Because the soldier can be expelled by the player
            "consume=smoked_fish,smoked_meat:2 tabard_golden",
            "train=soldier hp 0 1"
         }
      },
      upgrade_soldier_evade_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier evade from level 0 to level 1",
         actions = {
            "check_soldier=soldier evade 0", -- Fails when aren't any soldier of level 0 evade
            "sleep=30000",
            "check_soldier=soldier evade 0", -- Because the soldier can be expelled by the player
            "consume=atlanteans_bread smoked_fish,smoked_meat:2",
            "train=soldier evade 0 1"
         }
      },
      upgrade_soldier_evade_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier evade from level 1 to level 2",
         actions = {
            "check_soldier=soldier evade 1", -- Fails when aren't any soldier of level 1 evade
            "sleep=30000",
            "check_soldier=soldier evade 1", -- Because the soldier can be expelled by the player
            "consume=atlanteans_bread:2 smoked_fish,smoked_meat:2",
            "train=soldier evade 1 2"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 20
}
