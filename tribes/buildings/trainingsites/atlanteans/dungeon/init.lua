dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_dungeon",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Dungeon"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      planks = 2,
      granite = 4,
      diamond = 2,
      quartz = 2,
      spidercloth = 2,
      gold = 2,
      log = 4
   },
   return_on_dismantle = {
      planks = 1,
      granite = 3,
      diamond = 1,
      quartz = 1,
      gold = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 47, 47 },
      }
   },

   aihints = {
      trainingsite_type = "advanced"
   },

   working_positions = {
      atlanteans_trainer = 1
   },

   inputs = {
      atlanteans_bread = 10,
      smoked_fish = 6,
      smoked_meat = 6,
      trident_long = 4,
      trident_steel = 4,
      trident_double = 4,
      trident_heavy_double = 4
   },
   outputs = {
      "atlanteans_soldier",
   },

   ["soldier attack"] = {
      min_level = 0,
      max_level = 3,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"atlanteans_bread"}
      },
      weapons = {
         "trident_long",
         "trident_steel",
         "trident_double",
         "trident_heavy_double"
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
         descname = _"upgrading soldier attack from level 0 to level 1",
         actions = {
            "check_soldier=soldier attack 0",
            "sleep=30000",
            "check_soldier=soldier attack 0",
            "consume=atlanteans_bread smoked_fish,smoked_meat trident_long",
            "train=soldier attack 0 1"
         }
      },
      upgrade_soldier_attack_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier attack from level 1 to level 2",
         actions = {
            "check_soldier=soldier attack 1",
            "sleep=30000",
            "check_soldier=soldier attack 1",
            "consume=atlanteans_bread smoked_fish,smoked_meat trident_steel",
            "train=soldier attack 1 2"
         }
      },
      upgrade_soldier_attack_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier attack from level 2 to level 3",
         actions = {
            "check_soldier=soldier attack 2",
            "sleep=30000",
            "check_soldier=soldier attack 2",
            "consume=atlanteans_bread smoked_fish,smoked_meat trident_double",
            "train=soldier attack 2 3"
         }
      },
      upgrade_soldier_attack_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _"upgrading soldier attack from level 3 to level 4",
         actions = {
            "check_soldier=soldier attack 3",
            "sleep=30000",
            "check_soldier=soldier attack 3",
            "consume=atlanteans_bread smoked_fish,smoked_meat trident_heavy_double",
            "train=soldier attack 3 4"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 16
}
