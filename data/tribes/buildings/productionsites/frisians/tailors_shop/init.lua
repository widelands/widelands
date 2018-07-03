dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_tailors_shop",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Tailor’s Shop"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      brick = 1,
      granite = 2,
      log = 2,
      thatch_reed = 1
   },
   return_on_dismantle_on_enhanced = {
      granite = 1,
      log = 1
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
      prohibited_till = 890
   },

   working_positions = {
      frisians_seamstress = 1,
      frisians_seamstress_master = 1,
   },

   inputs = {
      { name = "fur_garment", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 4 },
   },
   outputs = {
      "fur_garment_studded",
      "fur_garment_golden"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=weave_studded",
            "call=weave_gold",
            "return=skipped",
         },
      },
      weave_studded = {
         -- TRANSLATORS: Completed/Skipped/Did not start sewing studded fur garment because ...
         descname = _"sewing studded fur garment",
         actions = {
            "return=skipped unless economy needs fur_garment_studded",
            "sleep=45000",
            "consume=fur_garment iron",
            "animate=working 45000",
            "produce=fur_garment_studded"
         },
      },
      weave_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start sewing golden fur garment because ...
         descname = _"sewing golden fur garment",
         actions = {
            "return=skipped unless economy needs fur_garment_golden",
            "sleep=47000",
            "consume=fur_garment iron gold",
            "animate=working 61000",
            "produce=fur_garment_golden"
         },
      },
   },
}
