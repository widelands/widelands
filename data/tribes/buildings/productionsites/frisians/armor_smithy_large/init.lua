dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_armor_smithy_large",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Large Armor Smithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      brick = 4,
      granite = 2,
      log = 2,
      thatch_reed = 3
   },
   return_on_dismantle_on_enhanced = {
      brick = 2,
      granite = 1,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 72},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 72},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 61},
      },
   },

   aihints = {
      prohibited_till = 850
   },

   working_positions = {
      frisians_blacksmith = 1,
      frisians_blacksmith_master = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 8 },
   },
   outputs = {
      "sword_curved",
      "sword_double",
      "helmet_golden",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_s3",
            "call=produce_hg",
            "call=produce_s4",
            "return=skipped",
         },
      },
      produce_s3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a curved sword because ...
         descname = _"forging a curved sword",
         actions = {
            "return=skipped unless economy needs sword_curved",
            "sleep=59000",
            "consume=coal iron:2 gold",
            "animate=working 51000",
            "produce=sword_curved"
         },
      },
      produce_s4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a double-edged sword because ...
         descname = _"forging a double-edged sword",
         actions = {
            "return=skipped unless economy needs sword_double",
            "sleep=64000",
            "consume=coal:2 iron:2 gold:2",
            "animate=working 51000",
            "produce=sword_double"
         },
      },
      produce_hg = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a golden helmet because ...
         descname = _"forging a golden helmet",
         actions = {
            "return=skipped unless economy needs helmet_golden",
            "sleep=51000",
            "consume=coal:2 iron:2 gold",
            "animate=working 64000",
            "produce=helmet_golden"
         },
      },
   },
}
