push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_ax_workshop",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Ax Workshop"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "barbarians_warmill",
      enhancement_cost = {
         log = 1,
         blackwood = 1,
         granite = 2,
         grout = 1,
         reed = 1
      },
      enhancement_return_on_dismantle = {
         blackwood = 1,
         granite = 1,
         grout = 1
      }
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 57, 76 },
      },
      unoccupied = {
         hotspot = { 57, 76 },
      },
   },
   spritesheets = {
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 57, 64 }
      },
      working = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 57, 75 }
      }
   },

   aihints = {
      prohibited_till = 800
   },

   working_positions = {
      barbarians_blacksmith = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- "return=skipped" causes 10 sec delay
            -- time total: 3 * 57.667 + 10 = 183 sec
            "call=produce_ax",
            "call=produce_ax_sharp",
            "call=produce_ax_broad",
            "return=skipped"
         }
      },
      produce_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an ax because ...
         descname = _("forging an ax"),
         actions = {
            -- time: 23.067 + 22 + 9 + 3.6 = 57.667 sec
            "return=skipped unless economy needs ax",
            "consume=coal iron",
            "sleep=duration:23s067ms",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:22s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=ax"
         }
      },
      produce_ax_sharp = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a sharp ax because ...
         descname = _("forging a sharp ax"),
         actions = {
            -- time: 23.067 + 22 + 9 + 3.6 = 57.667 sec
            "return=skipped unless economy needs ax_sharp",
            "consume=coal iron:2",
            "sleep=duration:23s067ms",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:22s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=ax_sharp"
         }
      },
      produce_ax_broad = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a broad ax because ...
         descname = _("forging a broad ax"),
         actions = {
            -- time: 23.067 + 22 + 9 + 3.6 = 57.667 sec
            "return=skipped unless economy needs ax_broad",
            "consume=coal:2 iron:2",
            "sleep=duration:23s067ms",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:22s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=ax_broad"
         }
      },
   },
}

pop_textdomain()
