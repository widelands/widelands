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
         hotspot = {56, 84},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 66},
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 84},
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
      "sword_short",
      "sword_long",
      "helmet",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_s1",
            "call=produce_s2",
            "call=produce_h",
            "call=produce_s2",
            "return=skipped",
         },
      },
      produce_s1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a short sword because ...
         descname = _"forging a short sword",
         actions = {
            -- time total: 76
            "return=skipped unless economy needs sword_short",
            "sleep=37000",
            "consume=coal iron",
            "playsound=sound/smiths smith 192",
            "animate=working 30000",
            "playsound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=sword_short"
         },
      },
      produce_s2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a long sword because ...
         descname = _"forging a long sword",
         actions = {
            -- time total: 79
            "return=skipped unless economy needs sword_long",
            "consume=coal iron:2",
            "sleep=38000",
            "playsound=sound/smiths smith 192",
            "animate=working 32000",
            "playsound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=sword_long"
         },
      },
      produce_h = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
         descname = _"forging a helmet",
         actions = {
            -- time total: 100
            "return=skipped unless economy needs helmet",
            "consume=coal iron:2",
            "sleep=50000",
            "animate=working 50000",
            "produce=helmet"
         },
      },
   },
}
