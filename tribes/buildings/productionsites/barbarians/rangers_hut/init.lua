dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_rangers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Farm",
   size = "small",

   buildcost = {
		log = 4
	},
	return_on_dismantle = {
		log = 3
	},

	-- #TRANSLATORS: Helptext for a building: Farm
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 43, 40 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 40, 33 },
		},
	},

   aihints = {
		renews_map_resource = "log",
		space_consumer = true
   },

	working_positions = {
		barbarians_ranger = 1
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
			descname = _"planting trees",
			actions = {
				"sleep=16000",
				"worker=plant"
			}
		},
	},
}
