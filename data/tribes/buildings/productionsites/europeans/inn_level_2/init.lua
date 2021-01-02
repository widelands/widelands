push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_inn_level_2",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Inn Level 2"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 75 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 52, 75 },
      },
   },

   aihints = {
      prohibited_till = 14400
   },

   working_positions = {
      europeans_baker_advanced = 1,
      europeans_smoker_advanced = 1
   },


   inputs = {
      { name = "water", amount = 12 },
      { name = "flour", amount = 12 },
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
            "return=skipped when economy needs ration",
            "return=skipped when economy needs snack",
            "return=skipped when economy needs meal",
            "sleep=duration:60s",
            "consume=water flour fish,meat",
            "animate=working duration:60s",
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
            "consume=water:2 flour:2 fish,meat:2",
            "animate=working duration:35s",
            "produce=ration:2"
         }
      },
      produce_snack_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            "return=skipped when economy needs ration",
            "return=skipped when economy needs snack",
            "return=skipped when economy needs meal",
            "sleep=duration:60s",
            "consume=water:2 flour:2 fish,meat:2",
            "animate=working duration:60s",
            "produce=snack"
         }
      },
      produce_snack = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            "return=skipped unless economy needs snack",
            "return=skipped when economy needs water",
            "sleep=duration:35s",
            "consume=water:4 flour:4 fish,meat:4",
            "animate=working duration:35s",
            "produce=snack:3"
         }
      },
      produce_meal_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _"preparing a meal",
         actions = {
            "return=skipped when economy needs ration",
            "return=skipped when economy needs snack",
            "return=skipped when economy needs meal",
            "sleep=duration:60s",
            "consume=water:3 flour:3 fish,meat:3",
            "animate=working duration:60s",
            "produce=meal"
         }
      },
      produce_meal = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _"preparing a meal",
         actions = {
            "return=skipped unless economy needs meal",
            "return=skipped when economy needs water",
            "sleep=duration:35s",
            "consume=water:4 flour:4 fish,meat:4",
            "animate=working duration:35s",
            "produce=meal:2"
         }
      },
   },
}

pop_textdomain()
