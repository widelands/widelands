dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_weaving_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Weaving Mill",
   size = "big",

   buildcost = {
		log = 5,
		granite = 2,
		thatch_reed = 2
	},
	return_on_dismantle = {
		log = 2,
		granite = 2
	},

	-- TRANSLATORS: Helptext for a building: Weaving Mill
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 36, 74 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 36, 74 },
		},
		working = {
			pictures = { dirname .. "working_\\d+.png" },
			hotspot = { 36, 74 },
		},
	},

   aihints = {
		prohibited_till = 1200
   },

	working_positions = {
		barbarians_weaver = 1
	},

   inputs = {
		thatch_reed = 8
	},
   outputs = {
		"cloth"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start weaving because ...
			descname = _"weaving",
			actions = {
				"sleep=20000",
				"return=skipped unless economy needs cloth",
				"consume=thatch_reed",
				"animate=working 25000",
				"produce=cloth"
			}
		},
	},
}
