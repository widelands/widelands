dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_honey_bread_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Honey Bread Bakery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      brick = 3,
      log = 3,
      granite = 1,
      thatch_reed = 1
   },
   return_on_dismantle_on_enhanced = {
      brick = 2,
      granite = 1,
      log = 1,
      thatch_reed = 2
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
         hotspot = {56, 80},
      },
   },

   aihints = {},

   working_positions = {
      frisians_baker = 1,
      frisians_baker_master = 1
   },

   inputs = {
      { name = "barley", amount = 8 },
      { name = "water", amount = 8 },
      { name = "honey", amount = 6 },
   },
   outputs = {
      "honey_bread",
      "bread_frisians"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=bake_honey",
            "call=bake_normal",
            "call=bake_honey",
            "return=skipped"
         }
      },
      bake_honey = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking honey bread because ...
         descname = _"baking honey bread",
         actions = {
            "return=skipped unless economy needs honey_bread or workers need experience",
            "sleep=35000",
            "consume=barley water honey",
            "animate=working 35000",
            "produce=honey_bread"
         }
      },
      bake_normal = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = _"baking bread",
         actions = {
            "return=skipped unless economy needs bread_frisians or workers need experience",
            "sleep=20000",
            "consume=barley water",
            "animate=working 20000",
            "produce=bread_frisians"
         }
      },
   },
}
