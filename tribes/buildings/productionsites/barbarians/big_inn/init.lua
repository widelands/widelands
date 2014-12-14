dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_big_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Big Inn",
   size = "medium",
   buildable = false,
   enhanced_building = true,

   enhancement_cost = {
		log = 1,
		grout = 3,
		thatch_reed = 2
	},
	return_on_dismantle_on_enhanced = {
		grout = 2
	},

	-- #TRANSLATORS: Helptext for a building: Big Inn
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 53, 62 },
		},
	},

   aihints = {
		prohibited_till = 600,
   },

	working_positions = {
		barbarians_innkeeper = 2
	},

   inputs = {
		fish = 4,
		bread_barbarians = 4,
		meat = 4,
		stout = 4,
		beer = 4
	},
   outputs = {
		"ration",
		"snack",
		"meal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=produce_ration",
				"call=produce_snack",
				"call=produce_meal",
				"return=skipped"
			}
		},
		produce_ration = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
			descname = _"preparing a ration",
			actions = {
				"return=skipped unless economy needs ration",
				"sleep=33000",
				"consume=fish,bread_barbarians,meat",
				"produce=ration"
			}
		},
		produce_snack = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
			descname = _"preparing a snack",
			actions = {
				"return=skipped unless economy needs snack",
				"sleep=35000",
				"consume=bread_barbarians fish,meat beer",
				"produce=snack"
			}
		},
		produce_meal = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
			descname = _"preparing a meal",
			actions = {
				"return=skipped unless economy needs meal",
				"sleep=37000",
				"consume=bread_barbarians fish,meat stout",
				"produce=meal"
			}
		},
	},
}
