dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_scouts_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Scout’s Hut",
   size = "small",

   buildcost = {
		log = 2,
		granite = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 1
	},

	-- TRANSLATORS: Helptext for a building: Scout’s Hut
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 45, 43 },
		},
	},

	working_positions = {
		barbarians_scout = 1
	},

   inputs = {
		ration = 2
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start scouting because ...
			descname = _"scouting",
			actions = {
				"sleep=30000",
				"consume=ration",
				"worker=scout"
			}
		},
	},
}
