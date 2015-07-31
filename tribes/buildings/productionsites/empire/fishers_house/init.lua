dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_fishers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Fisher’s House",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1
	},
	return_on_dismantle = {
		granite = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Fishes on the coast near the house.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The fisher’s house needs water full of fish within the work area.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 42, 60 },
		},
	},

   aihints = {
		needs_water = true,
		prohibited_till = 600
   },

	working_positions = {
		empire_fisher = 1
	},

   outputs = {
		"fish"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
			descname = _"fishing",
			actions = {
				"sleep=17000",
				"worker=fish"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Fish",
		message = _"The fisher working out of this fisher’s house can’t find any fish in his work area.",
	},
}
