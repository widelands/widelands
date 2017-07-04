dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_drinking_hall",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Drinking Hall"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      brick = 2,
      granite = 3,
      log = 2,
      thatch_reed = 2
   },
   return_on_dismantle_on_enhanced = {
      brick = 4,
      granite = 1,
      log = 2,
      thatch_reed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 88 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 57, 88 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 88 },
      },
   },

   aihints = {},

   working_positions = {
      frisians_landlady = 1
   },

   inputs = {
      --{ name = "mead", amount = 4 },
      { name = "smoked_fish", amount = 4 },
      { name = "smoked_meat", amount = 4 },
      { name = "fruit", amount = 4 },
      { name = "sweetbread", amount = 4 },
      { name = "beer", amount = 4 },
   },
   outputs = {
      "ration",
      "meal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ration",
            "call=produce_meal",
            "return=skipped"
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "sleep=8000",
            "return=skipped unless economy needs ration",
            "consume=smoked_fish,smoked_meat,fruit",
            "animate=working 16000",
            "produce=ration"
         }
      },
      produce_meal = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a meal",
         actions = {
            "sleep=8000",
            "return=skipped unless economy needs meal",
            "consume=beer sweetbread smoked_fish,smoked_meat,fruit",
            "animate=working 20000",
            "produce=meal"
         }
      },
   },
}
