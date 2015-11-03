dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Brewery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
		log = 3,
		granite = 1,
		thatch_reed = 1
	},
	return_on_dismantle_on_enhanced = {
		log = 1,
		granite = 1
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 60, 59 },
		},
		working = {
			template = "idle_??", -- TODO(GunChleoc): No animation yet.
			directory = dirname,
			hotspot = { 60, 59 },
		},
	},

   aihints = {
		prohibited_till = 600,
   },

	working_positions = {
		barbarians_brewer_master = 1,
		barbarians_brewer = 1,
	},

   inputs = {
		water = 8,
		wheat = 8
	},
   outputs = {
		"beer_strong"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start brewing strong beer because ...
			descname = _"brewing strong beer",
			actions = {
				"sleep=30000",
				"return=skipped unless economy needs beer_strong",
				"consume=water wheat",
				"animate=working 30000",
				"produce=beer_strong"
			}
		},
	},
}
