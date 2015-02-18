dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_big_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Big Inn",
   icon = dirname .. "menu.png",
   size = "medium",
   buildable = false,
   enhanced_building = true,

   enhancement_cost = {
		log = 1,
		grout = 3,
		thatch_reed = 2
	},
	return_on_dismantle_on_enhanced = {
		grout = 2
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
			fps = 1
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 57, 88 },
		},
	},

   aihints = {
		prohibited_till = 600,
   },

	working_positions = {
		barbarians_innkeeper = 2
	},

   inputs = {
		fish = 4,
		bread_barbarians = 4,
		meat = 4,
		stout = 4,
		beer = 4
	},
   outputs = {
		"ration",
		"snack",
		"meal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=produce_ration",
				"call=produce_snack",
				"call=produce_meal",
				"return=skipped"
			}
		},
		produce_ration = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
			descname = _"preparing a ration",
			actions = {
				"return=skipped unless economy needs ration",
				"sleep=33000",
				"consume=fish,bread_barbarians,meat",
				"produce=ration"
			}
		},
		produce_snack = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
			descname = _"preparing a snack",
			actions = {
				"return=skipped unless economy needs snack",
				"sleep=35000",
				"consume=bread_barbarians fish,meat beer",
				"produce=snack"
			}
		},
		produce_meal = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
			descname = _"preparing a meal",
			actions = {
				"return=skipped unless economy needs meal",
				"sleep=37000",
				"consume=bread_barbarians fish,meat stout",
				"produce=meal"
			}
		},
	},
}
