dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Inn"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "barbarians_big_inn",
      buildcost = {
         log = 1,
         grout = 3,
         reed = 2
      },
      return_on_dismantle = {
         grout = 2
      }
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
      { name = "fish", amount = 4 },
      { name = "meat", amount = 4 },
      { name = "barbarians_bread", amount = 4 },
      { name = "beer", amount = 4 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ration",
            "call=produce_snack",
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            -- time total: 33
            "return=skipped unless economy needs ration",
            "consume=barbarians_bread,fish,meat",
            "playsound=sound/barbarians/taverns/inn priority:80%",
            "animate=working duration:23s",
            "sleep=duration:10s",
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
            "playsound=sound/barbarians/taverns/inn priority:80%",
            "animate=working duration:27s",
            "sleep=duration:10s",
            "produce=snack"
         }
      },
   },
}
