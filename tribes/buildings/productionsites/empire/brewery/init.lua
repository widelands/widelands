dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Brewery",
   size = "medium",

   buildcost = {
		log = 1,
		planks = 2,
		granite = 2
	},
	return_on_dismantle = {
		planks = 1,
		granite = 1
	},

	-- #TRANSLATORS: Helptext for a building: Brewery
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 39, 62 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 39, 62 },
		},
	},

   aihints = {
		forced_after = 900,
		prohibited_till = 600
   },

	working_positions = {
		empire_brewer = 1
	},

   inputs = {
		water = 7,
		wheat = 7
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
				"return=skipped unless economy needs beer",
				"consume=water wheat",
				"animate=working 30000",
				"produce=beer"
			}
		},
	},
}
