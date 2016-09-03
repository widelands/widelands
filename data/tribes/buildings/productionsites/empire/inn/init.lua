dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Inn"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      planks = 2,
      marble = 2,
      marble_column = 1
   },
   return_on_dismantle_on_enhanced = {
      planks = 1,
      marble = 2
   },

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
      forced_after = 900,
      prohibited_till = 600
   },

   working_positions = {
      empire_innkeeper = 1
   },

   inputs = {
      { name = "fish", amount = 6 },
      { name = "meat", amount = 6 },
      { name = "empire_bread", amount = 6 }
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
            "return=skipped unless economy needs ration",
            "sleep=14000",
            "consume=empire_bread,fish,meat",
            "animate=working 19000",
            "produce=ration"
         }
      },
      produce_meal = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _"preparing a meal",
         actions = {
            "return=skipped unless economy needs meal",
            "sleep=15000",
            "consume=empire_bread fish,meat",
            "animate=working 20000",
            "produce=meal"
         }
      },
   },
}
