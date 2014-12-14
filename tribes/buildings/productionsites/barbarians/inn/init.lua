dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Inn",
   size = "medium",
   buildable = false,
   enhanced_building = true,
   enhancement = "barbarians_big_inn"

   enhancement_cost = {
		log = 2,
		grout = 2,
		thatch_reed = 1
	},
	return_on_dismantle_on_enhanced = {
		log = 1,
		grout = 1
	},

	-- #TRANSLATORS: Helptext for a building: Inn
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 48, 59 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 48, 59 },
		},
	},

	working_positions = {
		barbarians_innkeeper = 1
	},

   inputs = {
		fish = 4,
		bread_barbarians = 4,
		meat = 4,
		beer = 4
	},
   outputs = {
		"ration",
		"snack"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=produce_ration",
				"call=produce_snack",
				"return=skipped"
			}
		},
		produce_ration = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
			descname = _"preparing a ration",
			actions = {
				"return=skipped unless economy needs ration",
				"sleep=14000",
				"consume=fish,bread_barbarians,meat",
				"animate=working 19000",
				"produce=ration"
			}
		},
		produce_snack = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
			descname = _"preparing a snack",
			actions = {
				"return=skipped unless economy needs snack",
				"sleep=15000",
				"consume=bread_barbarians fish,meat beer",
				"animate=working 20000",
				"produce=snack"
			}
		},
	},
}
