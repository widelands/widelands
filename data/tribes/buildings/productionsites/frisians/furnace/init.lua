dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_furnace",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Furnace"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 4,
      granite = 2,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      thatch_reed = 1
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
      working_iron = {
         pictures = path.list_files (dirname .. "working_iron_??.png"),
         hotspot = {56, 80},
         fps = 10,
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
      basic_amount = 1
   },

   working_positions = {
      frisians_smelter = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron_ore", amount = 8 },
      { name = "gold_ore", amount = 8 },
   },
   outputs = {
      "iron",
      "gold"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=smelt_iron",
            "call=smelt_gold",
            "call=smelt_iron",
            "return=skipped",
         },
      },
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _"smelting iron",
         actions = {
            "return=skipped unless economy needs iron",
            "sleep=5000",
            "consume=coal iron_ore",
            "sleep=22000",
            "animate=working_iron 35000",
            "produce=iron"
         },
      },
      smelt_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting gold because ...
         descname = _"smelting gold",
         actions = {
            "return=skipped unless economy needs gold",
            "sleep=10000",
            "consume=coal gold_ore",
            "sleep=17000",
            "animate=working_gold 35000",
            "produce=gold"
         },
      },
   },
}
