dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_armor_smithy_small",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Small Armor Smithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "frisians_armor_smithy_large",

   buildcost = {
      brick = 3,
      granite = 1,
      log = 1,
      thatch_reed = 3
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 87},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 63},
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 87},
         fps = 10,
      },
   },

   aihints = {
      prohibited_till = 800,
   },

   working_positions = {
      frisians_blacksmith = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
   },
   outputs = {
      "sword_basic",
      "sword_long",
      "helmet",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_s1",
            "call=produce_h",
            "call=produce_s2",
            "return=skipped",
         },
      },
      produce_s1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a basic sword because ...
         descname = _"forging a basic sword",
         actions = {
            "return=skipped unless economy needs sword_basic",
            "sleep=36000",
            "consume=coal iron",
            "animate=working 28000",
            "produce=sword_basic"
         },
      },
      produce_s2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a long sword because ...
         descname = _"forging a long sword",
         actions = {
            "return=skipped unless economy needs sword_long",
            "sleep=56000",
            "consume=coal iron:2",
            "animate=working 46000",
            "produce=sword_long"
         },
      },
      produce_h = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
         descname = _"forging a helmet",
         actions = {
            "return=skipped unless economy needs helmet",
            "sleep=50000",
            "consume=coal iron:2",
            "animate=working 60000",
            "produce=helmet"
         },
      },
   },
}
