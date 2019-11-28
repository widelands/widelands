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

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {50, 64}
      }
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
