dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_warmill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "War Mill"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

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
      barbarians_blacksmith = 1,
      barbarians_blacksmith_master = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 8 }
   },
   outputs = {
      "ax",
      "ax_sharp",
      "ax_broad",
      "ax_bronze",
      "ax_battle",
      "ax_warriors"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ax",
            "call=produce_ax_sharp",
            "call=produce_ax_broad",
            "call=produce_ax_bronze",
            "call=produce_ax_battle",
            "call=produce_ax_warriors",
            "return=skipped"
         }
      },
      produce_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an ax because ...
         descname = _"forging an ax",
         actions = {
            -- time total: 73
            "return=skipped unless economy needs ax",
            "sleep=36000", -- +10 enlarge
            "consume=coal iron",
            "play_sound=sound/smiths smith 192",
            "animate=working 28000",  -- +6 enlarge
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=ax"
         }
      },
      produce_ax_sharp = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a sharp ax because ...
         descname = _"forging a sharp ax",
         actions = {
            -- time total: 76
            "return=skipped unless economy needs ax_sharp",
            "consume=coal iron:2",
            "sleep=37000", -- +11 enlarge
            "play_sound=sound/smiths smith 192",
            "animate=working 30000", -- +8 enlarge
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=ax_sharp"
         }
      },
      produce_ax_broad = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a broad ax because ...
         descname = _"forging a broad ax",
         actions = {
            -- time total: 79
            "return=skipped unless economy needs ax_broad",
            "consume=coal:2 iron:2",
            "sleep=38000", -- +12 enlarge
            "play_sound=sound/smiths smith 192",
            "animate=working 32000", -- +10 enlarge
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=ax_broad"
         }
      },
      produce_ax_bronze = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a bronze ax because ...
         descname = _"forging a bronze ax",
         actions = {
            -- time total: 82
            "return=skipped unless economy needs ax_bronze",
            "consume=coal:2 iron:2",
            "sleep=39000", -- +13 enlarge
            "play_sound=sound/smiths smith 192",
            "animate=working 34000", -- +12 enlarge
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=ax_bronze"
         }
      },
      produce_ax_battle = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a battle ax because ...
         descname = _"forging a battle ax",
         actions = {
            -- time total: 87
            "return=skipped unless economy needs ax_battle",
            "consume=coal gold iron:2",
            "sleep=41000", -- +15 enlarge
            "play_sound=sound/smiths smith 192",
            "animate=working 37000", -- +15 enlarge
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=ax_battle"
         }
      },
      produce_ax_warriors = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a warrior’s ax because ...
         descname = _"forging a warrior’s ax",
         actions = {
            -- time total: 92
            "return=skipped unless economy needs ax_warriors",
            "consume=coal:2 gold:2 iron:2",
            "sleep=43000", -- +17 enlarge
            "play_sound=sound/smiths smith 192",
            "animate=working 40000", -- +18 enlarge
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=ax_warriors"
         }
      },
   },
}
