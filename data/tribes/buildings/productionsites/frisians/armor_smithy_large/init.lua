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
      log = 1,
      thatch_reed = 1
   },
   return_on_dismantle_on_enhanced = {
      brick = 2,
      granite = 1
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
         hotspot = {56, 82},
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
      "sword_broad",
      "sword_double",
      "helmet_golden",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_s3",
            "call=produce_s4",
            "call=produce_hg",
            "call=produce_s3",
            "call=produce_s4",
            "return=skipped",
         },
      },
      produce_s3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a broadsword because ...
         descname = _"forging a broadsword",
         actions = {
            -- time total: 82
            "return=skipped unless economy needs sword_broad",
            "sleep=39000",
            "consume=coal iron:2 gold",
            "playsound=sound/smiths smith 192",
            "animate=working 34000",
            "playsound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=sword_broad"
         },
      },
      produce_s4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a double-edged sword because ...
         descname = _"forging a double-edged sword",
         actions = {
            -- time total: 87
            "return=skipped unless economy needs sword_double",
            "consume=coal:2 iron:2 gold",
            "sleep=41000",
            "playsound=sound/smiths smith 192",
            "animate=working 37000",
            "playsound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=sword_double"
         },
      },
      produce_hg = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a golden helmet because ...
         descname = _"forging a golden helmet",
         actions = {
            -- time total: 115
            "return=skipped unless economy needs helmet_golden",
            "consume=coal:2 iron:2 gold",
            "sleep=51000",
            "animate=working 64000",
            "produce=helmet_golden"
         },
      },
   },
}
