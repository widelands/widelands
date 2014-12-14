dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_goldmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Deep Gold Mine",
   size = "mine",
   buildable = false,
   enhanced_building = true,
   enhancement = "barbarians_goldmine_deeper",

   enhancement_cost = {
		log = 4,
		granite = 2
	},
	return_on_dismantle_on_enhanced = {
		log = 2,
		granite = 1
	},

	-- #TRANSLATORS: Helptext for a building: Deep Gold Mine
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 21, 37 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 21, 37 },
		},
		working = {
			pictures = { dirname .. "working_\\d+.png" },
			hotspot = { 21, 37 },
		},
		empty = {
			pictures = { dirname .. "empty_\\d+.png" },
			hotspot = { 21, 37 },
		},
	},

   aihints = {
		mines = "gold",
		mines_percent = 60
   },

	working_positions = {
		barbarians_miner = 1,
		barbarians_miner_chief = 1,
	},

   inputs = {
		snack = 6
	},
   outputs = {
		"gold_ore"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
			descname = _"mining gold",
			actions = {
				"sleep=42000",
				"return=skipped unless economy needs gold_ore",
				"consume=snack",
				"animate=working 18000",
				"mine=gold 2 66 5 17",
				"produce=gold_ore:2",
				"animate=working 18000",
				"mine=gold 2 66 5 17",
				"produce=gold_ore:2"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Gold Vein Exhausted",
		message =
			_"This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider enhancing, dismantling or destroying it.",
		delay_attempts = 0
	},
}
