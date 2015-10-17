dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Charcoal Kiln"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 3,
		grout = 2,
		thatch_reed = 2
	},
	return_on_dismantle = {
		log = 2,
		grout = 1
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 50, 71 },
		},
		build = {
			template = "build_??",
			directory = dirname,
			hotspot = { 50, 71 },
		},
		working = {
			template = "working_??",
			directory = dirname,
			hotspot = { 50, 71 },
		},
	},

   aihints = {
		prohibited_till = 900
   },

	working_positions = {
		barbarians_charcoal_burner = 1
	},

   inputs = {
		log = 8
	},
   outputs = {
		"coal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
			descname = _"producing coal",
			actions = {
				"sleep=30000",
				"return=skipped unless economy needs coal",
				"consume=log:6",
				"animate=working 90000", -- Charcoal fires will burn for some days in real life
				"produce=coal"
			}
		},
	},
}
