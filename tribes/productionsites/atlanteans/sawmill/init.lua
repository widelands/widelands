dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_sawmill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Sawmill",
   size = "medium",

   buildcost = {
		log = 2,
		granite = 3
	},
	return_on_dismantle = {
		log = 1,
		granite = 2
	},

	-- TRANSLATORS: Helptext for a building: Sawmill
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 53, 60 },
		},
		working = {
			pictures = { dirname .. "working_\\d+.png" },
			hotspot = { 53, 60 },
			fps = 25
		}
	},

   aihints = {
		forced_after = 300,
		prohibited_till = 60
   },

	working_positions = {
		atlanteans_sawyer = 1
	},

   inputs = {
		log = 8
	},
   outputs = {
		"planks"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start sawing logs because ...
			descname = _"sawing logs",
			actions = {
				"sleep=16500" -- Much faster than barbarians' hardener
				"return=skipped unless economy needs planks",
				"consume=log:2",
				"playFX=../../../sound/sawmill/sawmill 192",
				"animate=working 20000", -- Much faster than barbarians' hardener
				"produce=planks"
			}
		},
	},
}
