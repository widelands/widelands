dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Inn"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_big_inn",

   enhancement_cost = {
      log = 2,
      grout = 2,
      thatch_reed = 1
   },
   return_on_dismantle_on_enhanced = {
      log = 1,
      grout = 1
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
      barbarians_innkeeper = 1
   },

   inputs = {
      fish = 4,
      barbarians_bread = 4,
      meat = 4,
      beer = 4
   },
   outputs = {
      "ration",
      "snack"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ration",
            "call=produce_snack",
            "return=skipped"
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "return=skipped unless economy needs ration",
            "sleep=5000",
            "consume=barbarians_bread,fish,meat",
            "play_sound=sound/barbarians/taverns inn 100",
            "animate=working 18000",
            "sleep=10000",
            "produce=ration"
         }
      },
      produce_snack = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            "return=skipped unless economy needs snack",
            "sleep=5000",  -- keeping first sleep low reduces futile wait time for resources
            "consume=barbarians_bread fish,meat beer",
            "play_sound=sound/barbarians/taverns inn 100",
            "animate=working 20000",
            "sleep=10000",
            "produce=snack"
         }
      },
   },
}
