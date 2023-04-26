push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_armor_smithy_small",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Small Armor Smithy"),
   animation_directory = dirname,
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
         hotspot = {50, 73},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {50, 73},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
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
            -- "return=skipped" causes 10 sec delay
            -- time total: 58.5 + 58.5 + 68.5 + 58.5 + 10 = 254 sec
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
            -- time: 21.9 + 24 + 9 + 3.6 = 58.5 sec
            "return=skipped unless economy needs sword_short",
            "consume=coal iron",
            "sleep=duration:21s900ms",
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
            -- time: 21.9 + 24 + 9 + 3.6 = 58.5 sec
            "return=skipped unless economy needs sword_long",
            "consume=coal iron:2",
            "sleep=duration:21s900ms",
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
            -- time: 27.9 + 37 + 3.6 = 68.5 sec
            "return=skipped unless economy needs helmet",
            "consume=coal iron",
            "sleep=duration:27s900ms",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:37s",
            "produce=helmet"
         },
      },
   },
}

pop_textdomain()
