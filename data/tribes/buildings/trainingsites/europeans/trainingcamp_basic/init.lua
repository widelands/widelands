push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "europeans_building",
   name = "europeans_trainingscamp_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Trainingscamp"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   
   buildcost = {
      planks = 6,
      spidercloth = 4,
      brick = 4,
      grout = 4,
      quartz = 1,
      diamond = 1
   },
   return_on_dismantle = {
      log = 4,
      spidercloth = 2,
      granite = 3,
      quartz = 1,
      diamond = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 44, 65 },
      },
      working = { -- TODO(GunChleoc): make animations
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 44, 65 },
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
      { name = "europeans_bread", amount = 16 },
      { name = "smoked_fish", amount = 8 },
      { name = "smoked_meat", amount = 8 },
      { name = "spear_advanced", amount = 2 },
      { name = "ax_broad", amount = 2 }
   },

   outputs = {
      "europeans_soldier",
      "scrap_iron"
   },

   ["soldier attack"] = {
      min_level = 0,
      max_level = 1,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"europeans_bread"}
      },
      weapons = {
         "spear_advanced", "ax_broad"
      }
   },
   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _"sleeping",
         actions = {
            "sleep=5000",
            "checksoldier=soldier attack 9", -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_attack_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier attack from level 0 to level 1"),
         actions = {
            "return=skipped when economy needs spear",
            "checksoldier=soldier attack 0",
            "sleep=90000",
            "checksoldier=soldier attack 0",
            "consume=europeans_bread smoked_fish,smoked_meat spear_advanced",
            "train=soldier attack 0 1",
            "produce=scrap_iron"
         }
      },
      upgrade_soldier_attack_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier attack from level 1 to level 2"),
         actions = {
            "return=skipped when economy needs spear_advanced",
            "checksoldier=soldier attack 1",
            "sleep=90000",
            "checksoldier=soldier attack 1",
            "consume=europeans_bread smoked_fish,smoked_meat ax_broad",
            "train=soldier attack 1 2",
            "produce=scrap_iron"
         }
      },
   },

   soldier_capacity = 4,
   trainer_patience = 18,
   
   messages = {
      -- TRANSLATORS: Empire training site tooltip when it has no soldiers assigned
      no_soldier = pgettext("europeans_building", "No soldier to train!"),
      -- TRANSLATORS: Empire training site tooltip when none of the present soldiers match the current training program
      no_soldier_for_level = pgettext("europeans_building", "No soldier found for this training level!"),
   },
}

pop_textdomain()

