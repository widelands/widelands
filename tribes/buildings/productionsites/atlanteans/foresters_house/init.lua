dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_foresters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Forester’s House"),
   directory = dirname,
   icon = dirname .. "menu.png",
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

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 37, 44 },
		}
	},

   aihints = {
		space_consumer = true,
		renews_map_resource = "log",
		prohibited_till = 240
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
