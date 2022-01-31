push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "frisians_armor_smithy_large",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Large Armor Smithy"),
   icon = dirname .. "menu.png",
   size = "medium",

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

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=produce_s3",
            "call=produce_s4",
            "call=produce_hg",
            "call=produce_s3",
            "call=produce_s4",
            "return=skipped"
         },
      },
      produce_s3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a broadsword because ...
         descname = _("forging a broadsword"),
         actions = {
            -- time total: 57 + 3.6
            "return=skipped unless economy needs sword_broad",
            "consume=coal iron:2 gold",
            "sleep=duration:24s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:24s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=sword_broad"
         },
      },
      produce_s4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a double-edged sword because ...
         descname = _("forging a double-edged sword"),
         actions = {
            -- time total: 57 + 3.6
            "return=skipped unless economy needs sword_double",
            "consume=coal:2 iron:2 gold",
            "sleep=duration:24s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:24s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=sword_double"
         },
      },
      produce_hg = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a golden helmet because ...
         descname = _("forging a golden helmet"),
         actions = {
            -- time total: 67 + 3.6
            "return=skipped unless economy needs helmet_golden",
            "consume=coal:2 iron:2 gold",
            "sleep=duration:30s",
            "animate=working duration:37s",
            "produce=helmet_golden"
         },
      },
   },
}

pop_textdomain()
