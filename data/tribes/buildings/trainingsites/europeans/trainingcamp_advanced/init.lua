push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_trainingsite_type {
   name = "europeans_trainingscamp_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Trainingscamp"),
   icon = dirname .. "menu.png",
   size = "medium",
   
   buildcost = {
      planks = 6,
      spidercloth = 4,
      brick = 4,
      grout = 4,
      marble_column = 3,
      quartz = 2,
      diamond = 2
   },
   return_on_dismantle = {
      log = 4,
      spidercloth = 2,
      granite = 3,
      marble = 3,
      quartz = 2,
      diamond = 2
   },
   
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 56, 68 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 56, 68 },
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
      { name = "trident_steel", amount = 2 },
      { name = "sword_broad", amount = 2 },
   },


   ["soldier attack"] = {
      food = {
         {"smoked_fish", "smoked_meat"},
         {"europeans_bread"}
      },
      weapons = {"trident_steel", "sword_broad"}
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
      upgrade_soldier_attack_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier attack from level 2 to level 3"),
         actions = {
            "return=skipped when economy needs trident_steel",
            "checksoldier=soldier:attack level:2",
            "sleep=duration:45s",
            "checksoldier=soldier:attack level:2",
            "consume=europeans_bread smoked_fish,smoked_meat trident_steel",
            "train=soldier:attack level:3",
            "produce=scrap_iron"
         }
      },
      upgrade_soldier_attack_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier attack from level 3 to level 4"),
         actions = {
            "return=skipped when economy needs trident_double",
            "checksoldier=soldier:attack level:3",
            "sleep=duration:45s",
            "checksoldier=soldier:attack level:3",
            "consume=europeans_bread smoked_fish,smoked_meat sword_broad",
            "train=soldier:attack level:4",
            "produce=scrap_iron"
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
