push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_sawmill_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Sawmill"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_sawmill_advanced",
        enhancement_cost = {
          marble_column = 2,
          quartz = 1,
          diamond = 1
        },
        enhancement_return_on_dismantle = {
          marble = 2,
          quartz = 1,
          diamond = 1
        },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 60 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 53, 60 },
         fps = 25
      }
   },

   aihints = {
      prohibited_till = 7200
   },

   working_positions = {
      europeans_carpenter_normal = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=saw_log_basic",
            "call=saw_log",
         }
      },
      saw_log_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
         descname = _"sawing logs",
         actions = {
            "return=skipped when economy needs planks",
            "return=skipped when not site has log:2",
            "consume=log:2",
            "sleep=duration:30s",
            "playsound=sound/atlanteans/saw/benchsaw priority:50% allow_multiple",
            "animate=working duration:30s", 
            "produce=planks"
         }
      },
      saw_log = {
         -- TRANSLATORS: Completed/Skipped/Did not start sawing logs because ...
         descname = _"sawing logs",
         actions = {
            "return=skipped unless economy needs planks",
            "return=skipped when economy needs log",
            "consume=log:4",
            "sleep=duration:15s",
            "playsound=sound/atlanteans/saw/benchsaw priority:50% allow_multiple",
            "animate=working duration:20s",
            "produce=planks:3"
         }
      },
   },
}

pop_textdomain()
