dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_marblemine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Marble Mine",
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "empire_marblemine_deep",

   buildcost = {
		log = 4,
		planks = 2
	},
	return_on_dismantle = {
		log = 2,
		planks = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Carves marble and granite out of the rock in mountain terrain.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 49, 49 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 49, 49 },
			fps = 10
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 49, 49 },
		},
	},

   aihints = {
		mines = "granite",
		mines_percent = 50,
		prohibited_till = 450
   },

	working_positions = {
		empire_miner = 1
	},

   inputs = {
		ration = 6,
		wine = 6
	},
   outputs = {
		"marble",
		"granite"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=mine_marble",
				"call=mine_granite",
				"return=skipped"
			}
		},
		mine_marble = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining marble because ...
			descname = _"mining marble",
			actions = {
				"sleep=20000",
				"return=skipped unless economy needs marble or economy needs granite",
				"consume=wine ration",
				"animate=working 20000",
				"mine=granite 2 50 5 17",
				"produce=marble:2",
				"animate=working 20000",
				"mine=granite 2 50 5 17",
				"produce=marble granite"
			}
		},
		mine_granite = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
			descname = _"mining granite",
			actions = {
				"sleep=20000",
				"return=skipped unless economy needs marble or economy needs granite",
				"consume=ration wine",
				"animate=working 20000",
				"mine=granite 2 50 5 17",
				"produce=granite marble",
				"animate=working 20000",
				"mine=granite 2 50 5 17",
				"produce=granite:2"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Marble Vein Exhausted",
		message =
			_"This marble mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider enhancing, dismantling or destroying it.",
	},
}
