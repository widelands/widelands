dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_barracks",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Barracks"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 4,
      granite = 2,
      log = 3,
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      log = 1,
      reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {43, 84},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {43, 84},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {43, 69},
      },
   },

   aihints = {
      prohibited_till = 920,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      frisians_trainer = 1
   },

   inputs = {
      { name = "sword_short", amount = 8 },
      { name = "fur_garment", amount = 8 },
      { name = "frisians_carrier", amount = 8 }
   },
   outputs = {
      "frisians_soldier",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("frisians_building", "recruiting soldier"),
         actions = {
            "sleep=15000",
            "return=skipped unless economy needs frisians_soldier",
            "consume=sword_short fur_garment frisians_carrier",
            "animate=working 15000",
            "recruit=frisians_soldier"
         }
      },
   }
}
