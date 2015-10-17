dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_fishbreeders_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Fish Breeder’s House"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1
	},
	return_on_dismantle = {
		granite = 1
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 34, 42 },
		},
	},

   aihints = {
		needs_water = true,
		renews_map_resource = "fish",
		prohibited_till = 300
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
