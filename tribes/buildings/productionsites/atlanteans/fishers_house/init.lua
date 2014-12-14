dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_fishers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Fisher’s House",
   size = "small",

   buildcost = {
		log = 1,
		planks = 2
	},
	return_on_dismantle = {
		planks = 1
	},

	-- #TRANSLATORS: Helptext for a building: Fisher’s House
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 34, 42 },
		},
	},

   aihints = {
		needs_water = true,
		prohibited_till = 600
   },

	working_positions = {
		atlanteans_fisher = 1
	},

   outputs = {
		"fish"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
			descname = _"fishing",
			actions = {
				"sleep=16000",
				"worker=fish"
			}
		},

	},
	out_of_resource_notification = {
		title = _"Out of Fish",
		message = _"The fisher working out of this fisher’s house can’t find any fish in his working radius." .. " " .. _"Remember that you can increase the number of existing fish by building a fishbreeder’s house.",
		delay_attempts = 10
	},
}
