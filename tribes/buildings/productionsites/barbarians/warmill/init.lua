dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_warmill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"War Mill",
   icon = dirname .. "menu.png",
   size = "medium",
   buildable = false,
   enhanced_building = true

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
		lore_author = _[[An old Barbarian proverb<br> meaning that you need to take some risks sometimes.]],
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"The war mill produces all the axes that the barbarians use for warfare.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The barbarian war mill is their most advanced production site for weapons. As such it needs to be upgraded from an axfactory.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If all needed wares are delivered in time, this building can produce each type of ax in about %s on average.":bformat(ngettext("%d second", "%d seconds", 57):bformat(57)) .. " " ..	_"All weapons require the same time for making, but the consumption of the same raw materials increases from step to step. The last two need gold."
   }

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

	working_positions = {
		barbarians_blacksmith = 1,
		barbarians_blacksmith_master = 1
	},

   inputs = {
		iron = 8,
		coal = 8,
		gold = 8
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
				"return=skipped unless economy needs ax",
				"sleep=32000",
				"consume=coal iron",
				"playFX=../../../sound/smiths/smith 192",
				"animate=working 25000",
				"playFX=../../../sound/smiths/sharpening 192",
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
				"playFX=../../../sound/smiths/smith 192",
				"animate=working 25000",
				"playFX=../../../sound/smiths/sharpening 192",
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
				"playFX=../../../sound/smiths/smith 192",
				"animate=working 25000",
				"playFX=../../../sound/smiths/sharpening 192",
				"produce=ax_broad"
			}
		},
		produce_ax_bronze = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a bronze ax because ...
			descname = _"forging a bronze ax",
			actions = {
				"return=skipped unless economy needs ax_bronze",
				"sleep=32000",
				"consume=coal:2 iron:2",
				"playFX=../../../sound/smiths/smith 192",
				"animate=working 25000",
				"playFX=../../../sound/smiths/sharpening 192",
				"produce=ax_bronze"
			}
		},
		produce_ax_battle = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a battle ax because ...
			descname = _"forging a battle ax",
			actions = {
				"return=skipped unless economy needs ax_battle",
				"sleep=32000",
				"consume=coal gold iron:2",
				"playFX=../../../sound/smiths/smith 192",
				"animate=working 25000",
				"playFX=../../../sound/smiths/sharpening 192",
				"produce=ax_battle"
			}
		},
		produce_ax_warriors = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a warrior’s ax because ...
			descname = _"forging a warrior’s ax",
			actions = {
				"return=skipped unless economy needs ax_warriors",
				"sleep=32000",
				"consume=coal:2 gold:2 iron:2",
				"playFX=../../../sound/smiths/smith 192",
				"animate=working 25000",
				"playFX=../../../sound/smiths/sharpening 192",
				"produce=ax_warriors"
			}
		},
	},
}
