dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_goldmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Gold Mine",
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

	-- TRANSLATORS: Helptext for a building: Gold Mine
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
		mines = gold,
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
		"gold_ore"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining because ...
			descname = _"mining",
			actions = {
				"sleep=45000",
				"return=skipped unless economy needs gold_ore",
				"consume=smoked_fish,smoked_meat:2 bread_atlanteans:2",
				"animate=working 20000",
				"mine=gold 4 100 5 2",
				"produce=gold_ore",
				"animate=working 20000",
				"mine=gold 4 100 5 2",
				"produce=gold_ore",
				"animate=working 20000",
				"mine=gold 4 100 5 2",
				"produce=gold_ore"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Gold Vein Exhausted",
		message =
			_"This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider dismantling or destroying it.",
		delay_attempts = 0
	},
}
