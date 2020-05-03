dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_furnace",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Furnace"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      granite = 5,
      ironwood = 2
   },
   return_on_dismantle = {
      granite = 3,
      ironwood = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 80},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 66},
      },
      working_gold = {
         pictures = path.list_files (dirname .. "working_gold_??.png"),
         hotspot = {56, 80},
         fps = 10,
      },
   },

   aihints = {
      prohibited_till = 700,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      --basic_amount = 1
   },

   working_positions = {
      amazons_gold_smelter = 1
   },

   inputs = {
      { name = "charcoal", amount = 8 },
      { name = "gold_dust", amount = 8 },
   },
   outputs = {
      "gold"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=smelt_gold",
         },
      },
      smelt_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting gold because ...
         descname = _"smelting gold",
         actions = {
            "return=skipped unless economy needs gold",
            "consume=charcoal gold_dust",
            "sleep=30000",
            "animate=working_gold 42000",
            "produce=gold"
         },
      },
   },
}
