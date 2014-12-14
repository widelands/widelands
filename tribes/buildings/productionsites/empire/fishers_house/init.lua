dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_fishers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Fisher’s House",
   size = "small",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1
	},
	return_on_dismantle = {
		granite = 1
	},

	-- #TRANSLATORS: Helptext for a building: Fisher’s House
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
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
		message = _"The fisher working out of this fisher’s house can’t find any fish in his working radius.",
		delay_attempts = 10
	},
}
