dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_gamekeepers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Gamekeeper’s Hut",
   size = "small",

   buildcost = {
		log = 4,
		granite = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 1
	},

	-- #TRANSLATORS: Helptext for a building: Gamekeeper’s Hut
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 44, 41 },
		},
	},

   aihints = {
		renews_map_resource = "meat",
		prohibited_till = 900
   },

	working_positions = {
		barbarians_gamekeeper = 1
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"sleep=52500",
				"worker=release"
			}
		},
	},
}
