push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_trainingsite_type {
   name = "amazons_training_glade",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Training Glade"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      ironwood = 4,
      log = 5,
      granite = 4,
      rubber = 4,
      rope = 2,
      gold = 3
   },
   return_on_dismantle = {
      ironwood = 2,
      log = 3,
      granite = 2,
      rubber = 2,
      rope = 1,
      gold = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {92, 92}},
      unoccupied = {hotspot = {92, 92}}
   },
   spritesheets = { working = {
      hotspot = {92, 92},
      fps = 15,
      frames = 30,
      columns = 6,
      rows = 5
   }},

   aihints = {
      trainingsites_max_percent = 75,
      prohibited_till = 1500,
      very_weak_ai_limit = 0,
      weak_ai_limit = 1
   },

   working_positions = {
      amazons_trainer = 1
   },

   inputs = {
      { name = "fish", amount = 5 },
      { name = "meat", amount = 5 },
      { name = "amazons_bread", amount = 6 },
      { name = "chocolate", amount = 6 },
      { name = "boots_sturdy", amount = 2 },
      { name = "boots_swift", amount = 2 },
      { name = "boots_hero", amount = 2 },
      { name = "armor_wooden", amount = 2 },
      { name = "helmet_wooden", amount = 2 },
      { name = "warriors_coat", amount = 2 },
      { name = "vest_padded", amount = 2 },
      { name = "protector_padded", amount = 2 },
   },

   ["soldier evade"] = {
      food = {
         {"fish", "meat"},
         {"chocolate"},
         {"amazons_bread"}
      },
      weapons = {
         "boots_sturdy",
         "boots_swift",
         "boots_hero",
      }
   },
   ["soldier health"] = {
      food = {
         {"fish", "meat"},
         {"chocolate"},
         {"amazons_bread"}
      },
      weapons = {
         "armor_wooden",
         "helmet_wooden",
         "warriors_coat",
      }
   },
   ["soldier defense"] = {
      food = {
         {"fish", "meat"},
         {"chocolate"},
         {"amazons_bread"}
      },
      weapons = {
         "vest_padded",
         "protector_padded"
      }
   },

   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _("sleeping"),
         actions = {
            "sleep=duration:5s",
            "return=skipped",
         }
      },
      upgrade_soldier_evade_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("amazons_building", "upgrading soldier evade from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:evade level:0",
            "return=failed unless site has boots_sturdy",
            "return=failed unless site has chocolate",
            "return=failed unless site has fish,meat",
            "animate=working duration:22s800ms",
            "checksoldier=soldier:evade level:0", -- Because the soldier can be expelled by the player
            "consume=boots_sturdy chocolate fish,meat",
            "train=soldier:evade level:1"
         }
      },
      upgrade_soldier_evade_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("amazons_building", "upgrading soldier evade from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:evade level:1",
            "return=failed unless site has boots_swift",
            "return=failed unless site has chocolate",
            "return=failed unless site has amazons_bread",
            "animate=working duration:15s600ms",
            "checksoldier=soldier:evade level:1", -- Because the soldier can be expelled by the player
            "consume=boots_swift amazons_bread chocolate",
            "train=soldier:evade level:2",
         }
      },
      upgrade_soldier_evade_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("amazons_building", "upgrading soldier evade from level 2 to level 3"),
         actions = {
            "checksoldier=soldier:evade level:2",
            "return=failed unless site has boots_hero",
            "return=failed unless site has chocolate:2",
            "return=failed unless site has amazons_bread:2",
            "return=failed unless site has fish,meat:2",
            "animate=working duration:15s600ms",
            "checksoldier=soldier:evade level:2", -- Because the soldier can be expelled by the player
            "consume=boots_hero amazons_bread:2 chocolate:2 fish,meat:2",
            "train=soldier:evade level:3",
         }
      },
      upgrade_soldier_health_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("amazons_building", "upgrading soldier health from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:health level:0",
            "return=failed unless site has armor_wooden",
            "return=failed unless site has chocolate",
            "return=failed unless site has fish,meat",
            "animate=working duration:22s800ms",
            "checksoldier=soldier:health level:0", -- Because the soldier can be expelled by the player
            "consume=armor_wooden chocolate fish,meat",
            "train=soldier:health level:1"
         }
      },
      upgrade_soldier_health_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("amazons_building", "upgrading soldier health from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:health level:1",
            "return=failed unless site has helmet_wooden",
            "return=failed unless site has chocolate",
            "return=failed unless site has amazons_bread",
            "animate=working duration:15s600ms",
            "checksoldier=soldier:health level:1", -- Because the soldier can be expelled by the player
            "consume=helmet_wooden amazons_bread chocolate",
            "train=soldier:health level:2",
         }
      },
      upgrade_soldier_health_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("amazons_building", "upgrading soldier health from level 2 to level 3"),
         actions = {
            "checksoldier=soldier:health level:2",
            "return=failed unless site has warriors_coat",
            "return=failed unless site has chocolate",
            "return=failed unless site has amazons_bread",
            "return=failed unless site has fish,meat",
            "animate=working duration:15s600ms",
            "checksoldier=soldier:health level:2", -- Because the soldier can be expelled by the player
            "consume=warriors_coat amazons_bread chocolate fish,meat",
            "train=soldier:health level:3",
         }
      },
      upgrade_soldier_defense_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("amazons_building", "upgrading soldier defense from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:defense level:0",
            "return=failed unless site has vest_padded",
            "return=failed unless site has chocolate",
            "return=failed unless site has amazons_bread",
            "animate=working duration:22s800ms",
            "checksoldier=soldier:defense level:0", -- Because the soldier can be expelled by the player
            "consume=vest_padded amazons_bread chocolate",
            "train=soldier:defense level:1",
         }
      },
      upgrade_soldier_defense_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("amazons_building", "upgrading soldier defense from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:defense level:1",
            "return=failed unless site has protector_padded",
            "return=failed unless site has chocolate",
            "return=failed unless site has amazons_bread",
            "return=failed unless site has fish,meat",
            "animate=working duration:22s800ms",
            "checksoldier=soldier:defense level:1", -- Because the soldier can be expelled by the player
            "consume=protector_padded amazons_bread chocolate fish,meat",
            "train=soldier:defense level:2",

         }
      },
   },

   soldier_capacity = 6,
   trainer_patience = 22,

   messages = {
      -- TRANSLATORS: Amazon training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("amazons_building", "No soldier to train!"),
      -- TRANSLATORS: Amazon training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("amazons_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
