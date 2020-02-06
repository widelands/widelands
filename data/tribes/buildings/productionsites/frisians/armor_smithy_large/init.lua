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
      brick = 2,
      granite = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle_on_enhanced = {
      brick = 2,
      granite = 1
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
         hotspot = {50, 71}
      }
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
            "return=no_stats",
         },
      },
      produce_s3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a broadsword because ...
         descname = _"forging a broadsword",
         actions = {
            -- time total: 57 + 3.6
            "return=skipped unless economy needs sword_broad",
            "consume=coal iron:2 gold",
            "sleep=24000",
            "playsound=sound/smiths/smith 192",
            "animate=working 24000",
            "playsound=sound/smiths/sharpening 120",
            "sleep=9000",
            "produce=sword_broad"
         },
      },
      produce_s4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a double-edged sword because ...
         descname = _"forging a double-edged sword",
         actions = {
            -- time total: 57 + 3.6
            "return=skipped unless economy needs sword_double",
            "consume=coal:2 iron:2 gold",
            "sleep=24000",
            "playsound=sound/smiths/smith 192",
            "animate=working 24000",
            "playsound=sound/smiths/sharpening 120",
            "sleep=9000",
            "produce=sword_double"
         },
      },
      produce_hg = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a golden helmet because ...
         descname = _"forging a golden helmet",
         actions = {
            -- time total: 67 + 3.6
            "return=skipped unless economy needs helmet_golden",
            "consume=coal:2 iron:2 gold",
            "sleep=30000",
            "animate=working 37000",
            "produce=helmet_golden"
         },
      },
   },
}
