push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_trainingsite_type {
   name = "barbarians_battlearena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Battle Arena"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 6,
      granite = 4,
      grout = 6,
      gold = 4,
      reed = 3
   },
   return_on_dismantle = {
      log = 3,
      granite = 3,
      grout = 3,
      gold = 2,
      reed = 1
   },

   animation_directory = dirname,
   animations = {
      unoccupied = {
         hotspot = { 110, 72 }
      },
   },

   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 110, 72 }
      },
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 110, 72 }
      },
      working = {
         fps = 1,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 110, 72 }
      },
   },

   aihints = {
      trainingsites_max_percent = 25,
      prohibited_till = 900,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      barbarians_trainer = 1
   },

   inputs = {
      { name = "meat", amount = 6 },
      { name = "fish", amount = 6 },
      { name = "barbarians_bread", amount = 8 },
      { name = "beer_strong", amount = 8 }
   },

   ["soldier evade"] = {
      food = {
         {"fish", "meat"},
         {"beer_strong"},
         {"barbarians_bread"}
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
         descname = pgettext("barbarians_building", "upgrading soldier evade from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:evade level:0", -- Fails when aren't any soldier of level 0 evade
            "return=failed unless site has barbarians_bread",
            "return=failed unless site has fish,meat",
            "return=failed unless site has beer_strong",
            "sleep=duration:15s",
            "animate=working duration:15s",
            "checksoldier=soldier:evade level:0", -- Because the soldier can be expelled by the player
            "consume=barbarians_bread fish,meat beer_strong",
            "train=soldier:evade level:1"
         }
      },
      upgrade_soldier_evade_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("barbarians_building", "upgrading soldier evade from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:evade level:1", -- Fails when aren't any soldier of level 1 evade
            "return=failed unless site has barbarians_bread",
            "return=failed unless site has fish,meat",
            "return=failed unless site has beer_strong",
            "sleep=duration:15s",
            "animate=working duration:15s",
            "checksoldier=soldier:evade level:1", -- Because the soldier can be expulsed by the player
            "consume=barbarians_bread fish,meat beer_strong",
            "train=soldier:evade level:2"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 6,

   messages = {
      -- TRANSLATORS: Barbarian training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("barbarians_building", "No soldier to train!"),
      -- TRANSLATORS: Barbarian training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("barbarians_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()
