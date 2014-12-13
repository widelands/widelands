dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_spiderfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Spider Farm",
   size = "big",

   buildcost = {
		log = 2,
		granite = 2,
		planks = 2
	},
	return_on_dismantle = {
		granite = 1,
		planks = 1
	},

	-- TRANSLATORS: Helptext for a building: Spider Farm
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 87, 75 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 87, 75 },
		}
	},

   aihints = {
		forced_after = 60,
		prohibited_till = 60
   },

	working_positions = {
		atlanteans_spiderbreeder = 1
	},

   inputs = {
		corn = 7,
		water = 7
	},
   outputs = {
		"spider_silk"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"sleep=25000",
				"return=skipped unless economy needs spider_silk",
				"consume=corn water",
				"animate=working 30000",
				"produce=spider_silk"
			}
		},
	},
}
