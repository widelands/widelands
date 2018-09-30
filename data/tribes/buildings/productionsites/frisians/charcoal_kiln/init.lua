dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Charcoal Kiln"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 3,
      granite = 1,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 94},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 94},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 72},
      },
   },

   aihints = {
      basic_amount = 1
   },

   working_positions = {
      frisians_charcoal_burner = 1
   },

   inputs = {
      { name = "log", amount = 8 },
      { name = "clay", amount = 4 },
   },
   outputs = {
      "coal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
         descname = _"producing coal",
         actions = {
            "sleep=30000",
            "return=skipped unless economy needs coal",
            "consume=log:3 clay",
            "animate=working 30000",
            "produce=coal"
         },
      },
   },
}
