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
      thatch_reed = 2
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
      prohibited_till = 600,
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
            "return=skipped"
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "return=skipped unless economy needs ration",
            "sleep=30000",
            "consume=barbarians_bread,fish,meat",
            "play_sound=sound/barbarians/taverns tavern 100",
            "sleep=3000",
            "produce=ration"
         }
      },
      produce_snack = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            "return=skipped unless economy needs snack",
            "sleep=30000",
            "consume=barbarians_bread fish,meat beer",
            "play_sound=sound/barbarians/taverns biginn 100",
            "sleep=5000",
            "produce=snack"
         }
      },
      produce_meal = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _"preparing a meal",
         actions = {
            "return=skipped unless economy needs meal",
            "sleep=32000",
            "consume=barbarians_bread fish,meat beer_strong",
            "play_sound=sound/barbarians/taverns biginn 100",
            "sleep=5000",
            "produce=meal"
         }
      },
   },
}
