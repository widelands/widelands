push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_trainingsite_type {
   name = "atlanteans_labyrinth",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Labyrinth"),
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 86, 96 },
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

   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _("sleeping"),
         actions = {
            "sleep=duration:5s",
            "return=skipped",
         }
      },
      upgrade_soldier_defense_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier defense from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:defense level:0", -- Fails when aren't any soldier of level 0 defense
            "return=failed unless site has shield_steel",
            "return=failed unless site has atlanteans_bread",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:30s",
            "checksoldier=soldier:defense level:0", -- Because the soldier can be expulsed by the player
            "consume=atlanteans_bread smoked_fish,smoked_meat shield_steel",
            "train=soldier:defense level:1"
         }
      },
      upgrade_soldier_defense_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier defense from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:defense level:1", -- Fails when aren't any soldier of level 1 defense
            "return=failed unless site has shield_advanced",
            "return=failed unless site has atlanteans_bread",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:30s",
            "checksoldier=soldier:defense level:1", -- Because the soldier can be expelled by the player
            "consume=atlanteans_bread smoked_fish,smoked_meat shield_advanced",
            "train=soldier:defense level:2"
         }
      },
      upgrade_soldier_health_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier health from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:health level:0", -- Fails when aren't any soldier of level 0 health
            "return=failed unless site has tabard_golden",
            "return=failed unless site has smoked_fish,smoked_meat:2",
            "sleep=duration:30s",
            "checksoldier=soldier:health level:0", -- Because the soldier can be expelled by the player
            "consume=smoked_fish,smoked_meat:2 tabard_golden",
            "train=soldier:health level:1"
         }
      },
      upgrade_soldier_evade_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier evade from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:evade level:0", -- Fails when aren't any soldier of level 0 evade
            "return=failed unless site has atlanteans_bread",
            "return=failed unless site has smoked_fish,smoked_meat:2",
            "sleep=duration:30s",
            "checksoldier=soldier:evade level:0", -- Because the soldier can be expelled by the player
            "consume=atlanteans_bread smoked_fish,smoked_meat:2",
            "train=soldier:evade level:1"
         }
      },
      upgrade_soldier_evade_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier evade from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:evade level:1", -- Fails when aren't any soldier of level 1 evade
            "return=failed unless site has atlanteans_bread:2",
            "return=failed unless site has smoked_fish,smoked_meat:2",
            "sleep=duration:30s",
            "checksoldier=soldier:evade level:1", -- Because the soldier can be expelled by the player
            "consume=atlanteans_bread:2 smoked_fish,smoked_meat:2",
            "train=soldier:evade level:2"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 40,

   messages = {
      -- TRANSLATORS: Atlantean training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("atlanteans_building", "No soldier to train!"),
      -- TRANSLATORS: Atlantean training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("atlanteans_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
