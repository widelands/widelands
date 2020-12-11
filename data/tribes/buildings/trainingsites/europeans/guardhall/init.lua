push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_trainingsite_type {
   name = "europeans_guardhall",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Guardhall"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      planks = 6,
      spidercloth = 4,
      brick = 4,
      grout = 4,
      quartz = 2,
      diamond = 2
   },
   return_on_dismantle = {
      log = 4,
      spidercloth = 2,
      granite = 3,
      quartz = 1,
      diamond = 1,
      scrap_metal_mixed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 47, 47 },
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
      { name = "europeans_bread", amount = 12 },
      { name = "smoked_fish", amount = 6 },
      { name = "smoked_meat", amount = 6 },
      { name = "armor_chain", amount = 4 },
      { name = "armor_helmet", amount = 4 },
      { name = "helmet_mask", amount = 4 },
   },

   ["soldier health"] = {
      food = {
         {"smoked_fish", "smoked_meat"},
         {"europeans_bread"}
      },
      weapons = {"armor_chain", "armor_helmet", "helmet_mask"}
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
      upgrade_soldier_health_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier health from level 0 to level 1"),
         actions = {
            "return=skipped when economy needs armor_chain",
            "checksoldier=soldier:health level:0", -- Fails when aren't any soldier of level 0 health
            "sleep=duration:30s",
            "checksoldier=soldier:health level:0", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat armor_chain",
            "train=soldier:health level:1",
            "produce=scrap_iron"
         }
      },
      upgrade_soldier_health_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier health from level 1 to level 2"),
         actions = {
            "return=skipped when economy needs armor_helmet and economy needs helmet",
            "checksoldier=soldier:health level:1", -- Fails when aren't any soldier of level 0 health
            "sleep=duration:30s",
            "checksoldier=soldier:health level:1", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat armor_helmet",
            "train=soldier:health level:2",
            "produce=scrap_iron"
         }
      },
      upgrade_soldier_health_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("europeans_building", "upgrading soldier health from level 2 to level 3"),
         actions = {
            "return=skipped when economy needs helmet_mask",
            "checksoldier=soldier:health level:2", -- Fails when aren't any soldier of level 0 health
            "sleep=duration:30s",
            "checksoldier=soldier:health level:2", -- Because the soldier can be expelled by the player
            "consume=europeans_bread smoked_fish,smoked_meat helmet_mask",
            "train=soldier:health level:3",
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
