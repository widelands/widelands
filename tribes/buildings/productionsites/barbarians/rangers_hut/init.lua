dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_rangers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Ranger’s Hut"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 4
	},
	return_on_dismantle = {
		log = 3
	},

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 45, 49 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 45, 49 },
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 45, 49 },
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
