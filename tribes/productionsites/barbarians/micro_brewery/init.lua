dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_micro_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Micro Brewery",
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

	-- TRANSLATORS: Helptext for a building: Micro Brewery
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 42, 50 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
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
			-- TRANSLATORS: Completed/Skipped/Did not start brewing because ...
			descname = _"brewing",
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
