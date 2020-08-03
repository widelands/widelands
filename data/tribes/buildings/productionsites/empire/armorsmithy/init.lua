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
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 62 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 50, 62 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 50, 62 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 50, 62 },
         fps = 5
      },
   },

   aihints = {
      prohibited_till = 1400,
   },

   working_positions = {
      empire_armorsmith = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 8 },
      { name = "cloth", amount = 8 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_armor_helmet",
            "call=produce_armor",
            "call=produce_armor_chain",
            "call=produce_armor_helmet",
            "call=produce_armor_gilded",
         }
      },
      produce_armor_helmet = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
         descname = _"forging a helmet",
         actions = {
            -- time total: 67 + 3.6
            "return=skipped unless economy needs armor_helmet",
            "consume=iron coal",
            "sleep=47000",
            "animate=working duration:20s",
            "produce=armor_helmet"
         }
      },
      produce_armor = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a suit of armor because ...
         descname = _"forging a suit of armor",
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs armor",
            "consume=iron coal cloth",
            "sleep=32000",
            "animate=working duration:45s",
            "produce=armor"
         }
      },
      produce_armor_chain = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a suit of chain armor because ...
         descname = _"forging a suit of chain armor",
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs armor_chain",
            "consume=iron:2 coal cloth",
            "sleep=32000",
            "animate=working duration:45s",
            "produce=armor_chain"
         }
      },
      produce_armor_gilded = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a suit of gilded armor because ...
         descname = _"forging a suit of gilded armor",
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs armor_gilded",
            "consume=iron:2 coal:2 cloth gold",
            "sleep=32000",
            "animate=working duration:45s",
            "produce=armor_gilded"
         }
      },
   },
}
