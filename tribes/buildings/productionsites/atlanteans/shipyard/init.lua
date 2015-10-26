dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Shipyard"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",
   needs_seafaring = true,

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

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 53, 66 },
		},
		build = {
			template = "build_??",
			directory = dirname,
			hotspot = { 53, 66 },
		},
		working = {
			template = "idle_??", -- TODO(GunChleoc): No animation yet.
			directory = dirname,
			hotspot = { 53, 66 },
		},
		unoccupied = {
			template = "idle_??", -- TODO(GunChleoc): No animation yet.
			directory = dirname,
			hotspot = { 53, 66 },
		}
	},

   aihints = {
		needs_water = true,
		shipyard = true,
		prohibited_till = 1500
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
				"construct=atlanteans_shipconstruction buildship 6",
				"animate=working 35000",
				"return=completed"
			}
		},
	},
}
