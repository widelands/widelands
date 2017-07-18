dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_armour_smithy_small",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Small Armour Smithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "frisians_armour_smithy_large",

   buildcost = {
      brick = 6,
      granite = 2,
      log = 1,
      thatch_reed = 3
   },
   return_on_dismantle = {
      brick = 3,
      granite = 1,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 62 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 50, 62 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 50, 62 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 50, 62 },
      },
   },

   aihints = {
      prohibited_till = 700,
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
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
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
            "sleep=14000",
            "consume=coal iron",
            "animate=working 16000",
            "produce=sword_basic"
         },
      },
      produce_s2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a long sword because ...
         descname = _"forging a long sword",
         actions = {
            "return=skipped unless economy needs sword_long",
            "sleep=14000",
            "consume=coal iron:2",
            "animate=working 19000",
            "produce=sword_long"
         },
      },
      produce_h = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
         descname = _"forging a helmet",
         actions = {
            "return=skipped unless economy needs helmet",
            "sleep=14000",
            "consume=coal iron:2",
            "animate=working 21000",
            "produce=helmet"
         },
      },
   },
}
