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
      brick = 7,
      granite = 2,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 4,
      granite = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 53 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 39, 53 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 39, 53 },
      },
   },

   aihints = {
      prohibited_till = 500
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
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"working",
         actions = {
            "call=smelt_iron",
            "call=smelt_gold",
            "return=skipped",
         },
      },
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"smelting iron",
         actions = {
            "sleep=8000",
            "return=skipped unless economy needs iron",
            "consume=coal iron_ore",
            "animate=working 28000",
            "produce=iron"
         },
      },
      smelt_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"smelting gold",
         actions = {
            "sleep=8000",
            "return=skipped unless economy needs gold",
            "consume=coal gold_ore",
            "animate=working 30000",
            "produce=gold"
         },
      },
   },
}
