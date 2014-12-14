dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Shipyard",
   size = "medium",

   buildcost = {
		log = 3,
		planks = 2,
		granite = 3,
		spidercloth = 2
	},
	return_on_dismantle = {
		log = 1,
		planks = 1,
		granite = 2,
		spidercloth = 1
	},

	-- #TRANSLATORS: Helptext for a building: Shipyard
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 53, 66 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 53, 66 },
		}
	},

   aihints = {
		needs_water = true
   },

	working_positions = {
		atlanteans_shipwright = 1
	},

   inputs = {
		planks = 10,
		log = 2,
		spidercloth = 4
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
				"return=completed"
			}
		},
	},
}
