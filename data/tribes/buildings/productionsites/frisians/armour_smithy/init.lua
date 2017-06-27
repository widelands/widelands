dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_armour_smithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Armour Smithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 6,
      granite = 4,
      log = 2,
      thatch_reed = 3
   },
   return_on_dismantle = {
      brick = 3,
      granite = 2,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 63 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 50, 63 },
      },
   },

   aihints = {
      forced_after = 900
   },

   working_positions = {
      frisians_blacksmith = 2
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 8 },
   },
   outputs = {
      "sword_basic",
      "sword_long",
      "sword_curved",
      "sword_double",
      "helmet",
      "helmet_golden",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"working",
         actions = {
            "call=produce_s1",
            "call=produce_s2",
            "call=produce_s3",
            "call=produce_s4",
            "call=produce_h1",
            "call=produce_h2",
            "return=skipped",
         },
      },
      produce_s1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a basic sword",
         actions = {
            "sleep=14000",
            "return=skipped unless economy needs sword_basic",
            "consume=coal iron",
            "animate=working 16000",
            "produce=sword_basic"
         },
      },
      produce_s2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a long sword",
         actions = {
            "sleep=14000",
            "return=skipped unless economy needs sword_long",
            "consume=coal iron:2",
            "animate=working 19000",
            "produce=sword_long"
         },
      },
      produce_s3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a curved sword",
         actions = {
            "sleep=14000",
            "return=skipped unless economy needs sword_curved",
            "consume=coal:2 iron gold",
            "animate=working 25000",
            "produce=sword_curved"
         },
      },
      produce_s4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a double-edged sword",
         actions = {
            "sleep=14000",
            "return=skipped unless economy needs sword_double",
            "consume=coal:2 iron:2 gold:2",
            "animate=working 38000",
            "produce=sword_double"
         },
      },
      produce_h1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a helmet",
         actions = {
            "sleep=14000",
            "return=skipped unless economy needs helmet",
            "consume=coal iron:2",
            "animate=working 21000",
            "produce=helmet"
         },
      },
      produce_h2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a golden helmet",
         actions = {
            "sleep=14000",
            "return=skipped unless economy needs helmet_golden",
            "consume=coal:2 iron:2 gold",
            "animate=working 32000",
            "produce=helmet_golden"
         },
      },
   },
}
