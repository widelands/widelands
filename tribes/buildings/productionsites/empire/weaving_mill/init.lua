dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_weaving_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Weaving Mill"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 3,
      granite = 4,
      marble = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 3
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 65, 62 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 65, 62 },
      },
      unoccupied = {
         template = "unoccupied_??",
         directory = dirname,
         hotspot = { 65, 62 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 65, 62 },
         fps = 5
      },
   },

   aihints = {
      prohibited_till = 120
   },

   working_positions = {
      empire_weaver = 1
   },

   inputs = {
      wool = 8
   },
   outputs = {
      "cloth"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving because ...
         descname = _"weaving",
         actions = {
            "sleep=25000",
            "return=skipped unless economy needs cloth",
            "consume=wool",
            "animate=working 15000", -- Unsure of balancing CW
            "produce=cloth"
         }
      },
   },
}
