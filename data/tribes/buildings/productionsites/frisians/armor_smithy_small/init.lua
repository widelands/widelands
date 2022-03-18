push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_armor_smithy_small",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Small Armor Smithy"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "frisians_armor_smithy_large",
      enhancement_cost = {
         brick = 2,
         granite = 1,
         log = 1,
         reed = 1
      },
      enhancement_return_on_dismantle = {
         brick = 2,
         granite = 1
      }
   },

   buildcost = {
      brick = 3,
      granite = 1,
      log = 2,
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 73},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {50, 73},
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
         hotspot = {50, 58}
      }
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

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=produce_s1",
            "call=produce_s2",
            "call=produce_h",
            "call=produce_s2",
            "return=skipped"
         },
      },
      produce_s1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a short sword because ...
         descname = _("forging a short sword"),
         actions = {
            -- time total: 57 + 3.6
            "return=skipped unless economy needs sword_short",
            "consume=coal iron",
            "sleep=duration:24s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:24s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=sword_short"
         },
      },
      produce_s2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a long sword because ...
         descname = _("forging a long sword"),
         actions = {
            -- time total: 57 + 3.6
            "return=skipped unless economy needs sword_long",
            "consume=coal iron:2",
            "sleep=duration:24s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:24s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=sword_long"
         },
      },
      produce_h = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
         descname = _("forging a helmet"),
         actions = {
            -- time total: 67 + 3.6
            "return=skipped unless economy needs helmet",
            "consume=coal iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:37s",
            "produce=helmet"
         },
      },
   },
}

pop_textdomain()
