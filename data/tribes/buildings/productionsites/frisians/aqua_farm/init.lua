dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_aqua_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Aqua Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 8,
      water = 20,
      log = 5,
      thatch_reed = 5
   },
   return_on_dismantle = {
      brick = 4,
      log = 3,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 68, 76 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 68, 76 },
      },
   },

   aihints = {
      forced_after = 1500
   },

   working_positions = {
      frisians_fisher = 2
   },

   inputs = {
      { name = "fruit", amount = 8 },
      { name = "water", amount = 8 },
   },
   outputs = {
      "fish"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"breeding fish",
         actions = {
            "sleep=30000",
            "return=skipped unless economy needs fish",
            "consume=fruit water:3",
            "animate=working 30000",
            "produce=fish"
         },
      },
   },
}
