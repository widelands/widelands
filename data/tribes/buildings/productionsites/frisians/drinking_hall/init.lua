dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_drinking_hall",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Drinking Hall"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      brick = 1,
      granite = 2,
      log = 1,
      reed = 1
   },
   return_on_dismantle_on_enhanced = {
      brick = 1,
      granite = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {50, 72}
      }
   },

   aihints = {
      prohibited_till = 930
   },

   working_positions = {
      frisians_landlady = 1
   },

   inputs = {
      { name = "fruit", amount = 4 },
      { name = "honey_bread", amount = 4 },
      { name = "beer", amount = 4 },
      { name = "smoked_fish", amount = 4 },
      { name = "smoked_meat", amount = 4 },
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
            "return=no_stats"
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            -- time total: 33
            "return=skipped unless economy needs ration",
            "consume=fruit,smoked_fish,smoked_meat",
            "sleep=5000",
            "animate=working 18000",
            "sleep=10000",
            "produce=ration"
         }
      },
      produce_meal = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _"preparing a meal",
         actions = {
            --time total: 40
            "return=skipped unless economy needs meal",
            "consume=beer honey_bread smoked_fish,smoked_meat",
            "animate=working 30000",
            "sleep=10000",
            "produce=meal"
         }
      },
   },
}
