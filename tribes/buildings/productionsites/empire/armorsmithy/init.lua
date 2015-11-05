dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_armorsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Armor Smithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 2,
      marble = 2,
      marble_column = 3
   },
   return_on_dismantle = {
      granite = 1,
      marble = 1,
      marble_column = 2
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 50, 62 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 50, 62 },
      },
      unoccupied = {
         template = "unoccupied_??",
         directory = dirname,
         hotspot = { 50, 62 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 50, 62 },
         fps = 5
      },
   },

   aihints = {
      prohibited_till = 700,
      forced_after = 900
   },

   working_positions = {
      empire_armorsmith = 1
   },

   inputs = {
      iron = 8,
      gold = 8,
      coal = 8,
      cloth = 8
   },
   outputs = {
      "armor_helmet",
      "armor",
      "armor_chain",
      "armor_gilded"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_armor_helmet",
            "call=produce_armor",
            "call=produce_armor_chain",
            "call=produce_armor_gilded",
            "return=skipped"
         }
      },
      produce_armor_helmet = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
         descname = _"forging a helmet",
         actions = {
            "return=skipped unless economy needs armor_helmet",
            "sleep=32000",
            "consume=iron coal",
            "animate=working 35000",
            "produce=armor_helmet"
         }
      },
      produce_armor = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a suit of armor because ...
         descname = _"forging a suit of armor",
         actions = {
            "return=skipped unless economy needs armor",
            "sleep=32000",
            "consume=iron coal cloth",
            "animate=working 45000",
            "produce=armor"
         }
      },
      produce_armor_chain = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a suit of chain armor because ...
         descname = _"forging a suit of chain armor",
         actions = {
            "return=skipped unless economy needs armor_chain",
            "sleep=32000",
            "consume=iron:2 coal cloth",
            "animate=working 45000",
            "produce=armor_chain"
         }
      },
      produce_armor_gilded = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a suit of gilded armor because ...
         descname = _"forging a suit of gilded armor",
         actions = {
            "return=skipped unless economy needs armor_gilded",
            "sleep=32000",
            "consume=iron:2 coal:2 cloth gold",
            "animate=working 45000",
            "produce=armor_gilded"
         }
      },
   },
}
