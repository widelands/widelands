push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_inn_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Inn"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_inn_level_1",
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
         hotspot = { 42, 65 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 42, 65 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 43, 65 },
         fps = 2
      },
   },

   aihints = {
      prohibited_till = 7200
   },

   working_positions = {
      europeans_baker_advanced = 1,
      europeans_smoker_advanced = 1
   },

   inputs = {
      { name = "water", amount = 10 },
      { name = "cornmeal", amount = 8 },
      { name = "flour", amount = 8 },
      { name = "fish", amount = 6 },
      { name = "meat", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ration_basic",
            "call=produce_ration",
            "call=produce_snack_basic",
            "call=produce_snack",
            "call=produce_meal_basic",
            "call=produce_meal"
         }
      },
      produce_ration_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "sleep=duration:30s",
            "consume=water cornmeal fish,meat",
            "animate=working duration:30s",
            "produce=ration"
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "return=skipped unless economy needs ration",
            "return=skipped when economy needs water",
            "sleep=duration:35s",
            "consume=water:2 cornmeal:2 fish,meat:2",
            "animate=working duration:35s",
            "produce=ration:2"
         }
      },
      produce_snack_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            "sleep=duration:30s",
            "consume=water:2 cornmeal flour fish,meat:2",
            "animate=working duration:30s",
            "produce=snack"
         }
      },
      produce_snack = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            "return=skipped unless economy needs snack",
            "return=skipped when economy needs water",
            "sleep=duration:45s",
            "consume=water:4 cornmeal:3 flour fish,meat:4",
            "animate=working duration:45s",
            "produce=snack:3"
         }
      },
      produce_meal_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _"preparing a meal",
         actions = {
            "sleep=duration:30s",
            "consume=water:3 cornmeal flour:2 fish,meat:3",
            "animate=working duration:30s",
            "produce=meal"
         }
      },
      produce_meal = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _"preparing a meal",
         actions = {
            "return=skipped unless economy needs meal",
            "return=skipped when economy needs water",
            "sleep=duration:45s",
            "consume=water:4 cornmeal flour:3 fish,meat:4",
            "animate=working duration:45s",
            "produce=meal:2"
         }
      },
   },
}

pop_textdomain()
