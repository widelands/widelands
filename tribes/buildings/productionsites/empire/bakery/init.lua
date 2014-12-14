dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Bakery",
   size = "medium",

   buildcost = {
		log = 2,
		planks = 2,
		granite = 3
	},
	return_on_dismantle = {
		planks = 1,
		granite = 2
	},

	-- #TRANSLATORS: Helptext for a building: Bakery
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 42, 65 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 42, 65 },
		},
		working = {
			pictures = { dirname .. "working_\\d+.png" },
			hotspot = { 43, 65 },
			fps = 2
		},
	},

   aihints = {
		prohibited_till = 600
   },

	working_positions = {
		empire_baker = 1
	},

   inputs = {
		flour = 6,
		water = 6
	},
   outputs = {
		"bread_empire"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
			descname = _"baking bread",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs bread_empire",
				"consume=flour water",
				"animate=working 15000",
				"produce=bread_empire"
			}
		},
	},
}
