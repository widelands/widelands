dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_crystalmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Crystal Mine",
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
		log = 7,
		planks = 4,
		spidercloth = 1
	},
	return_on_dismantle = {
		log = 3,
		planks = 2
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Carves precious stones out of the rock in mountain terrain.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 50, 56 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 50, 56 },
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 50, 56 },
		},
	},

   aihints = {
		mines = "granite",
		prohibited_till = 600
   },

	working_positions = {
		atlanteans_miner = 3
	},

   inputs = {
		atlanteans_bread = 10,
		smoked_fish = 10,
		smoked_meat = 6
	},
   outputs = {
		"diamond",
		"quartz",
		"granite"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=mine_granite",
				"call=mine_quartz",
				"call=mine_diamond",
				"return=skipped"
			}
		},
		mine_granite = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
			descname = _"mining granite",
			actions = {
				"return=skipped unless economy needs granite or economy needs quartz or economy needs diamond",
				"sleep=45000",
				"consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
				"animate=working 20000",
				"mine=granite 4 100 5 2",
				"produce=granite:2",
				"animate=working 20000",
				"mine=granite 4 100 5 2",
				"produce=granite:2",
				"animate=working 20000",
				"mine=granite 4 100 5 2",
				"produce=granite",
				"animate=working 20000",
				"mine=granite 4 100 5 2",
				"produce=quartz"
			}
		},
		mine_quartz = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining quartz because ...
			descname = _"mining quartz",
			actions = {
				"return=skipped unless economy needs granite or economy needs quartz or economy needs diamond",
				"sleep=45000",
				"consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
				"animate=working 20000",
				"mine=granite 4 100 5 2",
				"produce=granite:2",
				"animate=working 20000",
				"mine=granite 4 100 5 2",
				"produce=granite quartz",
				"animate=working 20000",
				"mine=granite 4 100 5 2",
				"produce=quartz:2"
			}
		},
		mine_diamond = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining diamonds because ...
			descname = _"mining diamonds",
			actions = {
				"return=skipped unless economy needs granite or economy needs quartz or economy needs diamond",
				"sleep=45000",
				"consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
				"animate=working 20000",
				"mine=granite 4 100 5 2",
				"produce=diamond",
				"animate=working 20000",
				"mine=granite 4 100 5 2",
				"produce=diamond",
				"animate=working 20000",
				"mine=granite 4 100 5 2",
				"produce=granite"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Crystal Vein Exhausted",
		message =
			_"This crystal mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider dismantling or destroying it.",
	},
}
