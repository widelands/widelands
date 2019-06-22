dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_sewing_room",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Sewing Room"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "frisians_tailors_shop",

   buildcost = {
      brick = 3,
      granite = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
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
         hotspot = {56, 73},
      },
   },

   aihints = {
      prohibited_till = 750,
   },

   working_positions = {
      frisians_seamstress = 1
   },

   inputs = {
      { name = "fur", amount = 8 },
   },
   outputs = {
      "fur_garment"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start sewing fur garment because ...
         descname = _"sewing fur garment",
         actions = {
            -- time total: 40 + 3.6
            "return=skipped unless economy needs fur_garment or workers need experience",
            "consume=fur:2",
            "sleep=20000",
            "animate=working 20000",
            "produce=fur_garment"
         },
      },
   },
}
