dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_cassava_root_cooker",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Cassava Root Cooker"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 4,
      rope = 2,
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      rope = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {43, 44}},
      unoccupied = {hotspot = {43, 44}},
   },
   spritesheets = {
      working = {
         hotspot = {43, 44},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      }
   },

   aihints = {
      prohibited_till = 930
   },

   working_positions = {
      amazons_cook = 1
   },

   inputs = {
      { name = "cassavaroot", amount = 4 },
      { name = "water", amount = 8 },
   },
   outputs = {
      "amazons_bread",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_bread",
         }
      },
      produce_bread = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"baking bread",
         actions = {
            -- time total: 33
            "return=skipped unless economy needs amazons_bread",
            "consume=cassavaroot water:2",
            "sleep=5000",
            "animate=working 18000",
            "sleep=10000",
            "produce=amazons_bread"
         }
      },
   },
}
