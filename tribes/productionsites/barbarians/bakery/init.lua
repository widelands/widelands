dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Bakery",
   size = "medium",

   buildcost = {
		log = 2,
		blackwood = 2,
		granite = 2,
		thatch_reed = 2
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 2
	},

	-- TRANSLATORS: Helptext for a building: Bakery
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 41, 58 },
		},
		unoccupied = {
			pictures = { dirname .. "unoccupied_\\d+.png" },
			hotspot = { 41, 58 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 41, 58 },
		},
	},

   aihints = {
		prohibited_till = 600
   },

	working_positions = {
		barbarians_baker = 1
	},

   inputs = {
		wheat = 6,
		water = 6
	},
   outputs = {
		"bread_barbarians"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"sleep=20000",
				"return=skipped unless economy needs bread_barbarians",
				"consume=water:3 wheat:3",
				"animate=working 20000",
				"produce=bread_barbarians",
				"animate=working 20000",
				"produce=bread_barbarians"
			}
		},
	},
}
