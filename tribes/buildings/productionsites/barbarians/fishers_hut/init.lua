dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_fishers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Fisher’s Hut",
   size = "small",

   buildcost = {
		log = 4
	},
	return_on_dismantle = {
		log = 2
	},

	-- #TRANSLATORS: Helptext for a building: Fisher’s Hut
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 39, 40 },
		},
	},

   aihints = {
		needs_water = true,
		prohibited_till = 900
   },

	working_positions = {
		barbarians_fisher = 1
	},

   outputs = {
		"fish"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
			descname = _"fishing",
			actions = {
				"sleep=18000",
				"worker=fish"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Fish",
		message = _"The fisher working out of this fisher’s hut can’t find any fish in his working radius.",
		delay_attempts = 0
	},
}
