dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Shipyard",
   size = "medium",

   buildcost = {
		log = 3,
		blackwood = 2,
		granite = 3,
		cloth = 2
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 2
	},

	-- #TRANSLATORS: Helptext for a building: Shipyard
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 62, 48 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 62, 48 },
		},
		unoccupied = {
			pictures = { dirname .. "unoccupied_\\d+.png" },
			hotspot = { 62, 48 },
		},
		working = {
			pictures = { dirname .. "working_\\d+.png" },
			hotspot = { 62, 48 },
		},
	},

   aihints = {
		needs_water = true
   },

	working_positions = {
		barbarians_shipwright = 1
	},

   inputs = {
		blackwood = 10,
		log = 2,
		cloth = 4
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
				"check_map=seafaring", -- NOCOM(GunChleoc): We should make this check on game creation as well and remove it from the allowed buildings
				"construct=shipconstruction buildship 6",
				"animate=working 35000",
				"return=completed"
			}
		},
	},
}
