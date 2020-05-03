dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_food_preserver",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Food Preserver"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 3,
      rope = 2,
      rubber =1
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      rope = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 84},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 84},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 66},
      },
   },

   aihints = {
      prohibited_till = 520,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      --basic_amount = 1
   },

   working_positions = {
      amazons_cook = 1
   },

   inputs = {
      { name = "charcoal", amount = 3 },
      { name = "bread_amazons", amount = 6 },
      { name = "fish", amount = 6 },
      { name = "meat", amount = 6 },
   },
   outputs = {
      "ration"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ration",
         },
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _"preparing a ration",
         actions = {
            -- time total: 33
            "return=skipped unless economy needs ration",
            "consume=bread_amazons:2 fish,meat:2 charcoal",
            "sleep=5000",
            "animate=working 18000",
            "sleep=10000",
            "produce=ration:2"
         },
      },
   },
}
