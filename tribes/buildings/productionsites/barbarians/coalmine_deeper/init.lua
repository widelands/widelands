dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_coalmine_deeper",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Deeper Coal Mine"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement_cost = {
		log = 4,
		granite = 2
	},
	return_on_dismantle_on_enhanced = {
		log = 2,
		granite = 1
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 60, 37 },
		},
		build = {
			template = "build_??",
			directory = dirname,
			hotspot = { 60, 37 },
		},
		working = {
			template = "working_??",
			directory = dirname,
			hotspot = { 60, 37 },
		},
		empty = {
			template = "empty_??",
			directory = dirname,
			hotspot = { 60, 37 },
		},
	},

   aihints = {
		mines = "coal"
   },

	working_positions = {
		barbarians_miner = 1,
		barbarians_miner_chief = 1,
		barbarians_miner_master = 1,
	},

   inputs = {
		meal = 6
	},
   outputs = {
		"coal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
			descname = _"mining coal",
			actions = {
				"sleep=40000",
				"return=skipped unless economy needs coal",
				"consume=meal",
				"animate=working 16000",
				"mine=coal 2 100 10 2",
				"produce=coal:2",
				"animate=working 16000",
				"mine=coal 2 100 10 2",
				"produce=coal:3"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Coal Vein Exhausted",
		message =
			pgettext("barbarians_building", "This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
	},
}
