dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_shipvard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Shipyard",
   size = "medium",

   buildcost = {
		log = 3,
		planks = 2,
		granite = 3,
		cloth = 2
	},
	return_on_dismantle = {
		log = 1,
		granite = 2,
		cloth = 1
	},

	-- TRANSLATORS: Helptext for a building: Shipyard
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 50, 63 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 50, 63 },
		},
		unoccupied = {
			pictures = { dirname .. "unoccupied_\\d+.png" },
			hotspot = { 50, 63 },
		},
		working = {
			pictures = { dirname .. "working_\\d+.png" },
			hotspot = { 50, 63 },
		},
	},

   aihints = {
		needs_water = true
   },

	working_positions = {
		empire_shipwright = 1
	},

   inputs = {
		planks = 10,
		log = 2,
		cloth = 4
	},
   outputs = {
		"beer"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"sleep=20000",
				"call=ship",
				"return=skipped"
			}
		},
		ship = {
			-- TRANSLATORS: Completed/Skipped/Did not start constructing a ship because ...
			descname = _"constructing a ship",
			actions = {
				"check_map=seafaring",
				"construct=shipconstruction buildship 6",
				"animate=working 35000",
				"return=completed"
			}
		},
	},
}
