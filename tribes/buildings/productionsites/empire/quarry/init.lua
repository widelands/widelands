dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_quarry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Quarry",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 2,
		planks = 1
	},
	return_on_dismantle = {
		log = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Cuts blocks of granite and marble out of rocks in the vicinity.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The quarry needs rocks to cut within the working radius.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 42, 57 },
		},
	},

   aihints = {
		forced_after = 0,
		stoneproducer = true
   },

	working_positions = {
		empire_stonemason = 1
	},

   outputs = {
		"granite",
		"marble"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=mine_granite",
				"call=mine_granite",
				"call=mine_granite",
				"call=mine_marble", -- This will find marble 2 out of 7 times
				"call=mine_granite",
				"call=mine_granite",
				"call=mine_marble", -- This will find marble 2 out of 7 times
				"return=skipped"
			}
		},
		mine_granite = {
			-- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
			descname = _"quarrying granite",
			actions = {
			  -- This order is on purpose so that the productivity
			  -- drops fast once all rocks are gone.
				"worker=cut_granite",
				"sleep=25000"
			}
		},
		mine_marble = {
			-- TRANSLATORS: Completed/Skipped/Did not start quarrying marble because ...
			descname = _"quarrying marble",
			actions = {
			  -- This order is on purpose so that the productivity
			  -- drops fast once all rocks are gone.
				"worker=cut_marble",
				"sleep=25000"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Rocks",
		message = _"The stonemason working at this quarry can’t find any rocks in his working radius.",
		delay_attempts = 10
	},
}
