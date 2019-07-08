dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_big_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Big Inn"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      log = 1,
      grout = 3,
      reed = 2
   },
   return_on_dismantle_on_enhanced = {
      grout = 2
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

   aihints = {
      prohibited_till = 930,
   },

   working_positions = {
      barbarians_innkeeper = 2
   },

   inputs = {
      { name = "fish", amount = 4 },
      { name = "meat", amount = 4 },
      { name = "barbarians_bread", amount = 4 },
      { name = "beer", amount = 4 },
      { name = "beer_strong", amount = 4 }
   },
   outputs = {
      "ration",
      "snack",
      "meal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ration",
            "call=produce_snack",
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
            "consume=barbarians_bread,fish,meat",
            "playsound=sound/barbarians/taverns/tavern 100",
            "animate=working 23000",
            "sleep=10000",
            "produce=ration"
         }
      },
      produce_snack = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            -- time total: 37
            "return=skipped unless economy needs snack",
            "consume=barbarians_bread fish,meat beer",
            "playsound=sound/barbarians/taverns/biginn 100",
            "animate=working 27000",
            "sleep=10000",
            "produce=snack"
         }
      },
      produce_meal = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _"preparing a meal",
         actions = {
            -- time total: 40
            "return=skipped unless economy needs meal",
            "consume=barbarians_bread fish,meat beer_strong",
            "playsound=sound/barbarians/taverns/biginn 100",
            "animate=working 30000",
            "sleep=10000",
            "produce=meal"
         }
      },
   },
}
