dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_lime_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Lime Kiln"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 4,
		granite = 2,
		blackwood = 1
	},
	return_on_dismantle = {
		log = 2,
		granite = 1
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 45, 53 },
		},
		working = {
			template = "idle_??", -- TODO(GunChleoc): No animation yet.
			directory = dirname,
			hotspot = { 45, 53 },
		},
	},

	aihints = {},

	working_positions = {
		barbarians_lime_burner = 1
	},

   inputs = {
		granite = 6,
		water = 6,
		coal = 3
	},
   outputs = {
		"grout"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mixing grout because ...
			descname = _"mixing grout",
			actions = {
				"sleep=50000",
				"return=skipped unless economy needs grout",
				"consume=coal granite:2 water:2",
				"animate=working 32000",
				"produce=grout:2"
			}
		},
	},
}
