dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Bakery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "frisians_honey_bread_bakery",

   buildcost = {
      brick = 3,
      granite = 1,
      log = 2,
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 80},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 80},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 66},
      },
   },

   aihints = {
      prohibited_till = 500,
      forced_after = 710
   },

   working_positions = {
      frisians_baker = 1
   },

   inputs = {
      { name = "barley", amount = 7 },
      { name = "water", amount = 7 },
   },
   outputs = {
      "bread_frisians"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = _"baking bread",
         actions = {
            "sleep=20000",
            "return=skipped unless economy needs bread_frisians or workers need experience",
            "consume=water barley",
            "animate=working 20000",
            "produce=bread_frisians"
         },
      },
   },
}
