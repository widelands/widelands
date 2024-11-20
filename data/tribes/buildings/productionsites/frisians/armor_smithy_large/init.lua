push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_armor_smithy_large",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Large Armor Smithy"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   spritesheets = {
      idle = {
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
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
            -- "return=skipped" causes 10 sec delay
            -- time total: 58.8 + 58.8 + 68.8 + 58.8 + 58.8 + 10 = 314 sec
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
            -- time: 22.2 + 24 + 9 + 3.6 = 58.8 sec
            "return=skipped unless economy needs sword_broad",
            "consume=coal iron:2 gold",
            "sleep=duration:22s200ms",
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
            -- time: 22.2 + 24 + 9 + 3.6 = 58.8 sec
            "return=skipped unless economy needs sword_double",
            "consume=coal:2 iron:2 gold",
            "sleep=duration:22s200ms",
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
            -- time: 28.2 + 37 + 3.6 = 68.8 sec
            "return=skipped unless economy needs helmet_golden",
            "consume=coal:2 iron:2 gold",
            "sleep=duration:28s200ms",
            "animate=working duration:37s",
            "produce=helmet_golden"
         },
      },
   },
}

pop_textdomain()
