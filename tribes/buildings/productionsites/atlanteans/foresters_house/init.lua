dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_foresters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Forester’s House",
   size = "small",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1
	},
	return_on_dismantle = {
		granite = 1,
		log = 1
	},

	-- #TRANSLATORS: Helptext for a building: Forester’s House
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 37, 44 },
		}
	},

   aihints = {
		space_consumer = true,
		renews_map_resource = "log",
		prohibited_till = 50
   },

	working_positions = {
		atlanteans_forester = 1
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
			descname = _"planting trees",
			actions = {
				"sleep=11000",
				"worker=plant"
			}
		},
	},
}
