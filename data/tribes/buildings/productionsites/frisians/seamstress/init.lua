dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_seamstress",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Seamstress"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "frisians_seamstress_master",

   buildcost = {
      brick = 3,
      granite = 1,
      log = 1,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 90},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 90},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 80},
      },
   },

   aihints = {
      prohibited_till = 500,
   },

   working_positions = {
      frisians_seamstress = 1
   },

   inputs = {
      { name = "fur", amount = 8 },
   },
   outputs = {
      "fur_clothes"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start sewing fur clothes because ...
         descname = _"sewing fur clothes",
         actions = {
            "sleep=20000",
            "return=skipped unless economy needs fur_clothes or workers need experience",
            "consume=fur:2",
            "animate=working 25000",
            "produce=fur_clothes"
         },
      },
   },
}
