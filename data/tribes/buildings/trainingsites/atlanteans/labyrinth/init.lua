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
      gold = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 3,
      planks = 2,
      spidercloth = 2,
      diamond = 1,
      gold = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 80, 88 },
      }
   },

   aihints = {
      prohibited_till = 900,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      atlanteans_trainer = 1
   },

   inputs = {
      { name = "smoked_fish", amount = 6 },
      { name = "smoked_meat", amount = 6 },
      { name = "atlanteans_bread", amount = 10 },
      { name = "tabard_golden", amount = 5 },
      { name = "shield_steel", amount = 4 },
      { name = "shield_advanced", amount = 4 },
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
   ["soldier health"] = {
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
            "return=no_stats",
         }
      },
      upgrade_soldier_defense_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier defense from level 0 to level 1"),
         actions = {
            "checksoldier=soldier defense 0", -- Fails when aren't any soldier of level 0 defense
            "return=failed unless site has shield_steel",
            "return=failed unless site has atlanteans_bread",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=30000",
            "checksoldier=soldier defense 0", -- Because the soldier can be expulsed by the player
            "consume=atlanteans_bread smoked_fish,smoked_meat shield_steel",
            "train=soldier defense 0 1"
         }
      },
      upgrade_soldier_defense_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier defense from level 1 to level 2"),
         actions = {
            "checksoldier=soldier defense 1", -- Fails when aren't any soldier of level 1 defense
            "return=failed unless site has shield_advanced",
            "return=failed unless site has atlanteans_bread",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=30000",
            "checksoldier=soldier defense 1", -- Because the soldier can be expelled by the player
            "consume=atlanteans_bread smoked_fish,smoked_meat shield_advanced",
            "train=soldier defense 1 2"
         }
      },
      upgrade_soldier_health_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier health from level 0 to level 1"),
         actions = {
            "checksoldier=soldier health 0", -- Fails when aren't any soldier of level 0 health
            "return=failed unless site has tabard_golden",
            "return=failed unless site has smoked_fish,smoked_meat:2",
            "sleep=30000",
            "checksoldier=soldier health 0", -- Because the soldier can be expelled by the player
            "consume=smoked_fish,smoked_meat:2 tabard_golden",
            "train=soldier health 0 1"
         }
      },
      upgrade_soldier_evade_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier evade from level 0 to level 1"),
         actions = {
            "checksoldier=soldier evade 0", -- Fails when aren't any soldier of level 0 evade
            "return=failed unless site has atlanteans_bread",
            "return=failed unless site has smoked_fish,smoked_meat:2",
            "sleep=30000",
            "checksoldier=soldier evade 0", -- Because the soldier can be expelled by the player
            "consume=atlanteans_bread smoked_fish,smoked_meat:2",
            "train=soldier evade 0 1"
         }
      },
      upgrade_soldier_evade_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier evade from level 1 to level 2"),
         actions = {
            "checksoldier=soldier evade 1", -- Fails when aren't any soldier of level 1 evade
            "return=failed unless site has atlanteans_bread:2",
            "return=failed unless site has smoked_fish,smoked_meat:2",
            "sleep=30000",
            "checksoldier=soldier evade 1", -- Because the soldier can be expelled by the player
            "consume=atlanteans_bread:2 smoked_fish,smoked_meat:2",
            "train=soldier evade 1 2"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 20
}
