dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Brewery",
   size = "medium",
   buildable = false,
   enhanced_building = true,

   enhancement_cost = {
		log = 3,
		granite = 1,
		thatch_reed = 1
	},
	return_on_dismantle_on_enhanced = {
		log = 1,
		granite = 1
	},

	-- TRANSLATORS: Helptext for a building: Brewery
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 60, 59 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
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
		"stout"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start brewing stout because ...
			descname = _"brewing stout",
			actions = {
				"sleep=30000",
				"return=skipped unless economy needs stout",
				"consume=water wheat",
				"animate=working 30000",
				"produce=stout"
			}
		},
	},
}
