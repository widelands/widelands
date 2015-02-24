dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Inn",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_big_inn",

   enhancement_cost = {
		log = 2,
		grout = 2,
		thatch_reed = 1
	},
	return_on_dismantle_on_enhanced = {
		log = 1,
		grout = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Text needed",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 57, 88 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 57, 88 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 57, 88 },
		},
	},

	aihints = {},

	working_positions = {
		barbarians_innkeeper = 1
	},

   inputs = {
		fish = 4,
		bread_barbarians = 4,
		meat = 4,
		beer = 4
	},
   outputs = {
		"ration",
		"snack"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=produce_ration",
				"call=produce_snack",
				"return=skipped"
			}
		},
		produce_ration = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
			descname = _"preparing a ration",
			actions = {
				"return=skipped unless economy needs ration",
				"sleep=14000",
				"consume=bread_barbarians,fish,meat",
				"animate=working 19000",
				"produce=ration"
			}
		},
		produce_snack = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
			descname = _"preparing a snack",
			actions = {
				"return=skipped unless economy needs snack",
				"sleep=15000",
				"consume=bread_barbarians fish,meat beer",
				"animate=working 20000",
				"produce=snack"
			}
		},
	},
}
