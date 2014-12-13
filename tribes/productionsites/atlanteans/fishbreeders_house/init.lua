dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_fishbreeders_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Fish Breeder’s House",
   size = "small",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1
	},
	return_on_dismantle = {
		granite = 1
	},

	-- TRANSLATORS: Helptext for a building: Fish Breeder’s House
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 34, 42 },
		},
	},

   aihints = {
		needs_water = true,
		renews_map_resource = fish,
		prohibited_till = 900
   },

	working_positions = {
		atlanteans_fishbreeder = 1
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start breeding fish because ...
			descname = _"breeding fish",
			actions = {
				"sleep=24000",
				"worker=breed"
			}
		},
	},
}
