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
         hotspot = {56, 83},
      },
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
      "bread_amazons",
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
            "return=skipped unless economy needs bread_amazons",
            "consume=cassavaroot water:2",
            "sleep=5000",
            "animate=working 18000",
            "sleep=10000",
            "produce=bread_amazons"
         }
      },
   },
}
