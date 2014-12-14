dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_scouts_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Scout’s House",
   size = "small",

   buildcost = {
		log = 2,
		granite = 1
	},
	return_on_dismantle = {
		log = 1
	},

	-- TRANSLATORS: Helptext for a building: Scout’s House
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 41, 44 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 41, 44 },
		}
	},

	working_positions = {
		atlanteans_scout = 1
	},

   inputs = {
		smoked_fish = 2,
		bread_atlanteans = 2
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start scouting because ...
			descname = _"scouting",
			actions = {
				"sleep=30000",
				"consume=smoked_fish",
				"worker=scout",
				"sleep=30000",
				"consume=bread_atlanteans",
				"worker=scout"
			}
		},
	},
}
