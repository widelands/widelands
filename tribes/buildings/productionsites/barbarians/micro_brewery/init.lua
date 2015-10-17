dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_micro_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Micro Brewery"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_brewery",

   buildcost = {
		log = 3,
		blackwood = 2,
		granite = 3,
		thatch_reed = 2
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 2
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 42, 50 },
		},
		working = {
			template = "idle_??", -- TODO(GunChleoc): No animation yet.
			directory = dirname,
			hotspot = { 42, 50 },
		},
	},

   aihints = {
		forced_after = 500
   },

	working_positions = {
		barbarians_brewer = 1
	},

   inputs = {
		water = 8,
		wheat = 8
	},
   outputs = {
		"beer"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
			descname = _"brewing beer",
			actions = {
				"sleep=30000",
				"return=skipped unless economy needs beer or workers need experience",
				"consume=water wheat",
				"animate=working 30000",
				"produce=beer"
			}
		},
	},
}
