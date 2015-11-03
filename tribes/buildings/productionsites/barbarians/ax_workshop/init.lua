dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_ax_workshop",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Ax Workshop"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_warmill",

   enhancement_cost = {
      log = 1,
      blackwood = 1,
      granite = 2,
      grout = 1,
      thatch_reed = 1
   },
   return_on_dismantle_on_enhanced = {
      blackwood = 1,
      granite = 1,
      grout = 1
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

   aihints = {},

   working_positions = {
      barbarians_blacksmith = 1
   },

   inputs = {
      iron = 8,
      coal = 8
   },
   outputs = {
      "ax",
      "ax_sharp",
      "ax_broad"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ax",
            "call=produce_ax_sharp",
            "call=produce_ax_broad",
            "return=skipped"
         }
      },
      produce_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an ax because ...
         descname = _"forging an ax",
         actions = {
            "return=skipped unless economy needs ax",
            "sleep=32000",
            "consume=coal iron",
            "playFX=sound/smiths smith 192",
            "animate=working 25000",
            "playFX=sound/smiths sharpening 192",
            "produce=ax"
         }
      },
      produce_ax_sharp = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a sharp ax because ...
         descname = _"forging a sharp ax",
         actions = {
            "return=skipped unless economy needs ax_sharp",
            "sleep=32000",
            "consume=coal iron:2",
            "playFX=sound/smiths smith 192",
            "animate=working 25000",
            "playFX=sound/smiths sharpening 192",
            "produce=ax_sharp"
         }
      },
      produce_ax_broad = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a broad ax because ...
         descname = _"forging a broad ax",
         actions = {
            "return=skipped unless economy needs ax_broad",
            "sleep=32000",
            "consume=coal:2 iron:2",
            "playFX=sound/smiths smith 192",
            "animate=working 25000",
            "playFX=sound/smiths sharpening 192",
            "produce=ax_broad"
         }
      },
   },
}
