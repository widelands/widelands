dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_seamstress_master",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Master Seamstress"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   enhancement_cost = {
      granite = 2,
      log = 2,
      thatch_reed = 2
   },
   return_on_dismantle_on_enhanced = {
      brick = 2,
      granite = 1,
      log = 2,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 44, 47 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 44, 47 },
      },
   },

   aihints = {
      prohibited_till = 900
   },

   working_positions = {
      frisians_seamstress = 1,
      frisians_seamstress_master = 1,
   },

   inputs = {
      { name = "fur_clothes", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 4 },
   },
   outputs = {
      "fur_clothes_studded",
      "fur_clothes_golden"
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
         -- TRANSLATORS: Completed/Skipped/Did not start sewing studded fur clothes because ...
         descname = _"sewing studded fur clothes",
         actions = {
            "return=skipped unless economy needs fur_clothes_studded",
            "sleep=8000",
            "consume=fur_clothes iron",
            "animate=working 28000",
            "produce=fur_clothes_studded"
         },
      },
      weave_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start sewing golden fur clothes because ...
         descname = _"sewing golden fur clothes",
         actions = {
            "return=skipped unless economy needs fur_clothes_golden",
            "sleep=8000",
            "consume=fur_clothes iron gold",
            "animate=working 34000",
            "produce=fur_clothes_golden"
         },
      },
   },
}
