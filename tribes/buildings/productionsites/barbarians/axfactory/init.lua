dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_axfactory",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Axfactory",
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

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"‘A new warrior’s ax brings forth the best in its wielder – or the worst in its maker.’",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"An old Barbarian proverb<br> meaning that you need to take some risks sometimes.",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Produces axes, sharp axes and broad axes.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The barbarian axfactory is the intermediate production site in a series of three buildings. It is an upgrade from the metal workshop but doesn’t require additional qualification for the worker.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If all needed wares are delivered in time, this building can produce each type of ax in about %s on average.":bformat(ngettext("%d second", "%d seconds", 57):bformat(57)) .. " " .. _"All three weapons take the same time for making, but the required raw materials vary."
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 57, 76 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 57, 76 },
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 57, 76 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
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
				"playFX=sound/smiths/smith 192",
				"animate=working 25000",
				"playFX=sound/smiths/sharpening 192",
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
				"playFX=sound/smiths/smith 192",
				"animate=working 25000",
				"playFX=sound/smiths/sharpening 192",
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
				"playFX=sound/smiths/smith 192",
				"animate=working 25000",
				"playFX=sound/smiths/sharpening 192",
				"produce=ax_broad"
			}
		},
	},
}
