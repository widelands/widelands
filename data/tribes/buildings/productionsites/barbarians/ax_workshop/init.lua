push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_ax_workshop",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Ax Workshop"),
   helptext_script = dirname .. "helptexts.lua",
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

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 76 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 57, 76 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 57, 76 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 76 },
         fps = 10
      },
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
         descname = _"working",
         actions = {
            "call=produce_ax",
            "call=produce_ax_sharp",
            "call=produce_ax_broad",
         }
      },
      produce_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an ax because ...
         descname = _"forging an ax",
         actions = {
            -- time total: 57 + 3.6
            "return=skipped unless economy needs ax",
            "consume=coal iron",
            "sleep=duration:26s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:22s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=ax"
         }
      },
      produce_ax_sharp = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a sharp ax because ...
         descname = _"forging a sharp ax",
         actions = {
            -- time total: 57 + 3.6
            "return=skipped unless economy needs ax_sharp",
            "consume=coal iron:2",
            "sleep=duration:26s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:22s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=ax_sharp"
         }
      },
      produce_ax_broad = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a broad ax because ...
         descname = _"forging a broad ax",
         actions = {
            -- time total: 57 + 3.6
            "return=skipped unless economy needs ax_broad",
            "consume=coal:2 iron:2",
            "sleep=duration:26s",
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
