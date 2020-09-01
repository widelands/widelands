push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_big_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Big Inn"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

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

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ration",
            "call=produce_snack",
            "call=produce_meal",
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            -- time total: 33
            "return=skipped unless economy needs ration",
            "consume=barbarians_bread,fish,meat",
            "playsound=sound/barbarians/taverns/tavern priority:80%",
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
            "playsound=sound/barbarians/taverns/biginn priority:80%",
            "animate=working duration:27s",
            "sleep=duration:10s",
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
            "playsound=sound/barbarians/taverns/biginn priority:80%",
            "animate=working duration:30s",
            "sleep=duration:10s",
            "produce=meal"
         }
      },
   },
}

pop_textdomain()
