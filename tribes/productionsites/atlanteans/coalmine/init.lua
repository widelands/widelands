dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_coalmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Coal Mine",
   size = "mine",

   buildcost = {
		log = 7,
		planks = 4,
		spidercloth = 1
	},
	return_on_dismantle = {
		log = 3,
		planks = 2
	},

	-- TRANSLATORS: Helptext for a building: Coal Mine
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 50, 56 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 50, 56 },
		},
		empty = {
			pictures = { dirname .. "empty_\\d+.png" },
			hotspot = { 50, 56 },
		},
	},

   aihints = {
		mines = coal,
		prohibited_till = 1200
   },

	working_positions = {
		atlanteans_miner = 3
	},

   inputs = {
		bread_atlanteans = 10,
		smoked_fish = 10,
		smoked_meat = 6
	},
   outputs = {
		"coal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining because ...
			descname = _"mining",
			actions = {
				"sleep=45000",
				"return=skipped unless economy needs coal",
				"consume=smoked_fish,smoked_meat:2 bread_atlanteans:2",
				"animate=working 20000",
				"mine=coal 4 100 5 2",
				"produce=coal:2",
				"animate=working 20000",
				"mine=coal 4 100 5 2",
				"produce=coal:2",
				"animate=working 20000",
				"mine=coal 4 100 5 2",
				"produce=coal:3"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Coal Vein Exhausted",
		message =
			_"This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider dismantling or destroying it.",
		delay_attempts = 0
	},
}
