dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Brewery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "frisians_mead_brewery",

   buildcost = {
      brick = 3,
      log = 2,
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
         hotspot = {56, 92},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 92},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 69},
      },
   },

   aihints = {
      prohibited_till = 530,
      forced_after = 720,
   },

   working_positions = {
      frisians_brewer = 1
   },

   inputs = {
      { name = "barley", amount = 7 },
      { name = "water", amount = 7 },
   },
   outputs = {
      "beer"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _"brewing beer",
         actions = {
            "sleep=30000",
            "return=skipped unless economy needs beer or workers need experience",
            "consume=water barley",
            "animate=working 30000",
            "produce=beer"
         },
      },
   },
}
